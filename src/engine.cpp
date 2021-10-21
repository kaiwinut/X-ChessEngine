#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/time.h>
#include "bitboard.h"
#include "types.h"
#include "masks.h"
#include "movegen.h"
#include "eval.h"
#include "engine.h"

using std::cout;
using std::endl;
using std::string;

Engine::Engine()
{
	resetEngine();
}

void Engine::search(Game curerntGame, int depth)
{
	game = curerntGame;	
	nodes = 0, ply = 0, bestEval = 0, inPV = 0, scorePV = 0, duration = 0, knps = 0, stopped = 0;

	memset(killerMoves, 0, sizeof(killerMoves));
	memset(historyMoves, 0, sizeof(historyMoves));
	memset(pvLength, 0, sizeof(pvLength));
	memset(pvTable, 0, sizeof(pvTable));

	int alpha = -INF;
	int beta = INF;

	for (int currentDepth = 1; currentDepth <= depth; currentDepth ++)
	{
		if (stopped == 1) { break; }

		// Follow the principle variation by default
		inPV = 1;
		// Run the principle variation search and measure the time
		clock_t startTime = clock();
		bestEval = PVS(currentDepth, alpha, beta);
		clock_t endTime = clock();	
		// Calculate the time and kilo nodes per second
		duration = (endTime - startTime) * 1000 / CLOCKS_PER_SEC;
		knps = nodes / duration;

		if ((bestEval <= alpha) || (bestEval >= beta))
		{
			alpha = -INF;
			beta = INF;
			continue;
		}

		alpha = bestEval - ASPIRATION_WINDOW_SIZE;
		beta = bestEval + ASPIRATION_WINDOW_SIZE;

		// Second parameter toggles debug mode
		printResults(currentDepth, depth, DEBUG_ENGINE);
	}
}

int Engine::PVS(int depth, int alpha, int beta)
{
	if ((nodes & 2047) == 0)
	{
		communicate();
	}

	int score;
	pvLength[ply] = ply;

	score = readHashEntry(depth, alpha, beta);
	int hashFlag = HASH_ALPHA;

	if (ply > 0 && isRepetition()) { return 0; }
	if (ply > 0 && score != NO_HASH_ENTRY && (beta - alpha <= 1)) { return score; }

	// Check for all captures possible at the end of the search 
	if (depth <= 0) { return quiescenceSearch(alpha, beta); }

	if (ply > MAX_PLY - 1) { return evaluate(game); }

	nodes ++;

	int legalMoves = 0;
	int isInCheck = game.isSquareAttacked(getLeastSignificantBitIndex((game.side == WHITE ? game.bitboards[K] : game.bitboards[k])), game.side ^ 1);

	if (isInCheck) { depth ++; }

	// Null move pruning
	if (depth >= 3 && isInCheck == 0 && ply > 0)
	{
		ply ++;

		repetitionIndex ++;
		repetitionTable[repetitionIndex] = game.hashKey;

		GameState prevState = game.makeNullMove();
		score = -PVS(depth - 1 - NULL_MOVE_REDUCTION, -beta, -beta + 1);
		game.takeBack(prevState);
		ply --;
		repetitionIndex --;

		if (stopped == 1) { return 0; }

		if (score >= beta)
		{
			return beta;
		}
	}

	game.generateAllMoves();

	// Make sure that we are actually inside PV before we switch on score PV move flag
	if (inPV) { enablePVScoring(); }

	sortMoves(game.moveList);
	int *move = game.moveList;
	int movesSearched = 0;

	while (*move)
	{
		ply ++;

		repetitionIndex ++;
		repetitionTable[repetitionIndex] = game.hashKey;

		GameState prevState = game.makeMove(*move, ALL_MOVES);
		if (!prevState.valid)
		{
			ply --;
			repetitionIndex --;
			move ++;
			continue;
		}
		legalMoves ++;

		int moveIsCheck = game.isSquareAttacked(getLeastSignificantBitIndex((game.side == WHITE ? game.bitboards[K] : game.bitboards[k])), game.side ^ 1);

		// Only do the full window search for the first move (supposedly the best move because we are following the principle variation)
		if (movesSearched == 0)	
		{
			score = -PVS(depth - 1, -beta, -alpha);	
		}
		// For the rest of the moves, do a narrow window search to prove that other moves are worse than the first one. 
		// The narrow window focuses around alpha since we no longer need to consider any other move that gives a score lower than alpha.
		else
		{
			// Do not reduce leaf nodes, moves that leave king in check, moves that gives checks, 
			// moves that are captures/promotions, and moves that are in the principle variation
			if (movesSearched >= FULL_DEPTH_MOVES && depth >= REDUCTION_LIMIT && isInCheck == 0 && moveIsCheck == 0 && 
				getCaptureFlag(*move) == 0 && getPromotion(*move) == NULL_PIECE && *move != pvTable[0][ply])
			{
				score = -PVS(depth - 2, -alpha - 1, -alpha);
			}
			// Later on this will fit the score > alpha && score < beta criteria in order to search this move in the full window
			else { score = alpha + 1; }

			if (score > alpha) {

				score = -PVS(depth - 1, -alpha - 1, -alpha);

				if (score > alpha && score < beta)
				{
					score = -PVS(depth - 1, -beta, -alpha);
				}
			}
		}

		game.takeBack(prevState);
		ply --;
		repetitionIndex --;
		movesSearched ++;

		if (stopped == 1) { return 0; }

		if (score > alpha)
		{
			hashFlag = HASH_EXACT;

			alpha = score;

			// Write move to triangular principle variation table
			pvTable[ply][ply] = *move;
			// Loop over the next ply and copy the principle variation to the current ply
			for (int nextPly = ply + 1; nextPly < pvLength[ply + 1]; nextPly ++)
			{
				pvTable[ply][nextPly] = pvTable[ply + 1][nextPly];
			}
			// Update the length of the variation (should be equal to the maximum ply?)
			pvLength[ply] = pvLength[ply + 1];

			if (score >= beta)
			{
				writeHashEntry(depth, beta, HASH_BETA);

				if (getCaptureFlag(*move) == 0)
				{
					killerMoves[ply][1] = killerMoves[ply][0];
					killerMoves[ply][0] = *move;
					historyMoves[getPiece(*move)][getEndSquare(*move)] += depth * depth;
				}
				return beta;
			}
		}
		move++;
	}

	// If player has no legal moves, it is either checkmate or stalemate
	if (legalMoves == 0)
	{
		// If the king is in check, return mate value. Plus ply is needed in order to find fastest mate.
		if (isInCheck) { return -MATE_VALUE + ply; }
		// Stalemate score equals draw value
		else { return 0; }
	}

	writeHashEntry(depth, alpha, hashFlag);

	return alpha;
}

int Engine::quiescenceSearch(int alpha, int beta)
{
	if ((nodes & 2047) == 0)
	{
		communicate();
	}

	nodes ++;
	int evaluation = evaluate(game);

	if (ply > MAX_PLY - 1) { return evaluation; }

	if (evaluation >= beta)
	{
		return beta;
	}

	if (evaluation > alpha)
	{
		alpha = evaluation;
	}

	game.generateAllMoves();
	sortMoves(game.moveList);

	int *move = game.moveList;
	
	while (*move)
	{
		ply ++;

		repetitionIndex ++;
		repetitionTable[repetitionIndex] = game.hashKey;

		GameState prevState = game.makeMove(*move, ONLY_CAPTURES);
		if (!prevState.valid)
		{
			ply --;
			repetitionIndex --;
			move ++;
			continue;
		}
		int score = -quiescenceSearch(-beta, -alpha);
		game.takeBack(prevState);
		ply --;
		repetitionIndex --;

		if (stopped == 1) { return 0; }

		if (score > alpha)
		{
			alpha = score;

			if (score >= beta)
			{
				return beta;
			}
		}

		move++;
	}
	return alpha;
}

int Engine::scoreMove(int move)
{
	if (move == 0)
	{
		return -1;
	}

	if (scorePV && move == pvTable[0][ply])
	{
		scorePV = 0;
		return PV_MOVE_SCORE;
	}

	if (getCaptureFlag(move))
	{
		return MVV_LVA[getPiece(move)][getCapturedPiece(move)] + CAPTURE_SCORE;
	}
	else
	{
		if (killerMoves[ply][0] == move)
		{
			return FIRST_KILLER_SCORE;
		}
		else if (killerMoves[ply][1] == move)
		{
			return SECOND_KILLER_SCORE;
		}
		else
		{
			return historyMoves[getPiece(move)][getEndSquare(move)];
		}
	}
}

void Engine::sortMoves(int * moveList)
{
	int* move = moveList;
	int* moveStart = move;
	while(*move)
	{
		int* p = move;
		int tmp = 0; 
		while(p != moveStart)
		{
			if (scoreMove(*p) > scoreMove(*(p-1)))
			{
				tmp = *p;
				*p = *(p-1);
				*(p-1) = tmp;
				p--;
			}
			else { break; }
		}
		move++;
	}
}

void Engine::enablePVScoring()
{
	inPV = 0;
	int *move = game.moveList;
	while (*move)
	{
		if (*move == pvTable[0][ply])
		{
			inPV = 1;
			scorePV = 1;
		}
		move++;
	}
}

void Engine::resetEngine()
{
	nodes = 0, ply = 0, bestEval = 0, inPV = 0, scorePV = 0, repetitionIndex = 0, duration = 0, knps = 0, stopped = 0;

	memset(killerMoves, 0, sizeof(killerMoves));
	memset(historyMoves, 0, sizeof(historyMoves));
	memset(pvLength, 0, sizeof(pvLength));
	memset(pvTable, 0, sizeof(pvTable));
	memset(repetitionTable, 0, sizeof(repetitionTable));

	hashEntries = 0;
	initTranspositionTable(64);
}

void Engine::clearTranspositionTable()
{
	for (HashEntry* entry = tt; entry < tt + hashEntries; entry ++)
	{
		entry -> hashKey = 0;
		entry -> depth = 0;
		entry -> flag = 0;
		entry -> score = 0;
	}
}

void Engine::initTranspositionTable(int mb)
{
	int hashSize = 0x100000 * mb;
	hashEntries = hashSize / sizeof(HashEntry);

	// Strange bug! Trying to free unallocated memory ? 
	if (tt != NULL)
	{
		// if (DEBUG_ENGINE) { cout << "Clearing hash memory ..." << endl; }
		delete[] tt;
	}

	tt = new HashEntry[hashEntries];

	if (tt == NULL)
	{
		// if (DEBUG_ENGINE) { cout << "Couldn't allocate memory to transposition table, try " << mb / 2 << " Mb ..." << endl; }
		initTranspositionTable(mb / 2);
	}

	else
	{
		clearTranspositionTable();
		// if (DEBUG_ENGINE) { cout << "Transposition table initialized with " << hashEntries << " entries. (" << mb << ")" << endl; }		
	}
}

int Engine::readHashEntry(int depth, int alpha, int beta)
{
	HashEntry *hashEntry = &tt[game.hashKey % hashEntries];
	if ( hashEntry -> hashKey == game.hashKey )
	{
		if (hashEntry -> depth >= depth)
		{
			int score = hashEntry -> score;
			if (score < -MATE_SCORE) { score += ply; }
			if (score > MATE_SCORE) { score -= ply; }

			if (hashEntry -> flag == HASH_EXACT)
			{
				return score;
			}
			if (hashEntry -> flag == HASH_ALPHA && score <= alpha)
			{
				return alpha;				
			}
			if (hashEntry -> flag == HASH_ALPHA && score >= beta)
			{
				return beta;
			}
		}
	}
	return NO_HASH_ENTRY;
}

void Engine::writeHashEntry(int depth, int score, int flag)
{
	HashEntry *hashEntry = &tt[game.hashKey % hashEntries];

	if (score < -MATE_SCORE) { score -= ply; }
	if (score > MATE_SCORE) { score += ply; }

	hashEntry -> score = score;
	hashEntry -> depth = depth;
	hashEntry -> flag = flag;
	hashEntry -> hashKey = game.hashKey;
}

int Engine::isRepetition()
{
	for (int i = 0; i < repetitionIndex; i ++)
	{
		if (repetitionTable[repetitionIndex] == game.hashKey)
		{
			return 1;
		}
	}
	return 0;
}

void Engine::printResults(int currentDepth, int depth_limit, int debug)
{
	// Show full result
	if (debug)
	{
		cout << "info depth " << currentDepth << " score cp " << bestEval << " time " << (int)duration << " nodes " << nodes << " nps " << knps * 1000 << " pv ";
		for (int i = 0; i < pvLength[0]; i ++)
		{
			int promotion = getPromotion(pvTable[0][i]);
			if (promotion != NULL_PIECE) 
			{
				if (promotion == Q || promotion == q) 
				{
					cout << SQUARES[getStartSquare(pvTable[0][i])] << SQUARES[getEndSquare(pvTable[0][i])] << 'q' << " ";
				}
				if (promotion == R || promotion == r) 
				{
					cout << SQUARES[getStartSquare(pvTable[0][i])] << SQUARES[getEndSquare(pvTable[0][i])] << 'r' << " ";
				}
				if (promotion == N || promotion == n) 
				{
					cout << SQUARES[getStartSquare(pvTable[0][i])] << SQUARES[getEndSquare(pvTable[0][i])] << 'n' << " ";
				}
				if (promotion == B || promotion == b) 
				{
					cout << SQUARES[getStartSquare(pvTable[0][i])] << SQUARES[getEndSquare(pvTable[0][i])] << 'b' << " ";
				}
			}
			else
			{
				cout << SQUARES[getStartSquare(pvTable[0][i])] << SQUARES[getEndSquare(pvTable[0][i])] << " ";
			}
		}
		cout << endl;	
	}
	// If not in debug mode, print results in uci protocol style
	else
	{
		if (currentDepth == depth_limit)
		{
			cout << "info depth " << currentDepth << " score cp " << bestEval << " time " << (int)duration << " nodes " << nodes << " nps " << knps * 1000 << " pv";
			for (int i = 0; i < pvLength[0]; i ++)
			{
				int promotion = getPromotion(pvTable[0][i]);
				if (promotion != NULL_PIECE) 
				{
					if (promotion == Q || promotion == q) 
					{
						cout << ' ' << SQUARES[getStartSquare(pvTable[0][i])] << SQUARES[getEndSquare(pvTable[0][i])] << 'q';
					}
					if (promotion == R || promotion == r) 
					{
						cout << ' ' << SQUARES[getStartSquare(pvTable[0][i])] << SQUARES[getEndSquare(pvTable[0][i])] << 'r';
					}
					if (promotion == N || promotion == n) 
					{
						cout << ' ' << SQUARES[getStartSquare(pvTable[0][i])] << SQUARES[getEndSquare(pvTable[0][i])] << 'n';
					}
					if (promotion == B || promotion == b) 
					{
						cout << ' ' << SQUARES[getStartSquare(pvTable[0][i])] << SQUARES[getEndSquare(pvTable[0][i])] << 'b';
					}
				}
				else
				{
					cout << ' ' << SQUARES[getStartSquare(pvTable[0][i])] << SQUARES[getEndSquare(pvTable[0][i])];
				}
			}
			cout << endl;
		}
	}
}

/*** UCI Section ***/

void Engine::uciLoop()
{
	std::cin.clear();

	int MAX_HASH = 128;
	int mb = 64;

	string input;
	Game game;

	while (1)
	{
		input.clear();
		cout << std::flush;
		if (!std::getline(std::cin, input)) { continue; }
		if (input[0] == '\n') { continue; }

		if (input.compare(0, 7, "isready", 7) == 0)
		{
			cout << "readyok" << endl;
			continue;
		}
		// parse uci position command
		else if (input.compare(0, 8, "position", 8) == 0)
		{
			parsePosition(game, input);
			if (DEBUG_UCI) { game.displayGame(); }
		}		

		// parse uci new game command
		else if (input.compare(0, 10, "ucinewgame", 10) == 0)
		{
			parsePosition(game, "position startpos");
			this -> resetEngine();
			if (DEBUG_UCI) { game.displayGame(); }
		}	

		else if (input.compare(0, 3, "uci", 3) == 0)
		{
			cout << "id name X" << endl;
			cout << "id author Kai" << endl;
			cout << "option name Hash type spin default 64 min 4 max " << MAX_HASH << endl;
			cout << "uciok" << endl;
		}

		// parse uci position command
		else if (input.compare(0, 2, "go", 2) == 0)
		{
			parseGo(game, input);
			if (DEBUG_UCI) { game.displayGame(); }
		}	

		else if (input.compare(0, 4, "quit", 4) == 0)
		{
			break;
		}

		// parse uci setoption command
		else if (input.compare(0, 24, "setoption name Hash value", 24) == 0)
		{
			mb = stoi(input.substr(26, input.length() - 1));
            // adjust MB if going beyond the aloowed bounds
            if(mb < 4) mb = 4;
            if(mb > MAX_HASH) mb = MAX_HASH;
            this -> initTranspositionTable(mb);
            cout << "Set hash size to " << mb << "Mb" << endl;
		}		
	}
}

// Parse user/GUI move command (e.g. e7e8q)
int Engine::parseMove(Game& game, string moveString)
{
	game.generateAllMoves();
	int startSquare = (moveString[0] - 'a')	+ 8 * (atoi(&moveString[1]) - 1);
	int endSquare = (moveString[2] - 'a') + 8 * (atoi(&moveString[3]) - 1);
	int* move = game.moveList;

	while (*move)
	{
		if (getStartSquare(*move) == startSquare && getEndSquare(*move) == endSquare)
		{
			int promotion = getPromotion(*move);
			if (promotion != NULL_PIECE)
			{
				if ((promotion == Q || promotion == q) && moveString[4] == 'q') { return *move; }
				else if ((promotion == R || promotion == r) && moveString[4] == 'r') { return *move; }
				else if ((promotion == B || promotion == b) && moveString[4] == 'b') { return *move; }
				else if ((promotion == N || promotion == n) && moveString[4] == 'n') { return *move; }

				cout << "Illegal promotion!" << endl;
				move ++;
				continue;		
			}
			return *move;
		}
		move++;
	}
	// Invalid move
	cout << "Illegal move!" << endl;
	return 0;
}

/*
    Example UCI commands to init position on chess board
    
    // init start position
    position startpos
    
    // init start position and make the moves on chess board
    position startpos moves e2e4 e7e5
    
    // init position from FEN string
    position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 
    
    // init position from fen string and make moves on chess board
    position fen r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 moves e2a6 e8g8
*/
void Engine::Engine::parsePosition(Game& game, string command) 
{
	if (command.compare(9, 8, "startpos", 8) == 0)
	{ 
		// cout << "Parse start position..." << endl;
		game.parseFen(START_POSITION);
		string moveCommand = command;

		moveCommand.erase(0, 17);
		if (moveCommand.length() > 0 && moveCommand.compare(1, 5, "moves", 5) == 0)
		{
			moveCommand.erase(0, 7);
			while (moveCommand.length())
			{
				for (int i = 0; i < 6; i++)
				{
					if (i < moveCommand.length() && moveCommand[i] == ' ')
					{
						string moveString = moveCommand.substr(0, i);
						// cout << moveString << endl;
						int move = parseMove(game, moveString);
						if (move != 0)
						{
							game.makeMove(move);
						}
						moveCommand.erase(0, i + 1);
					}
					else if (i == moveCommand.length())
					{
						string moveString = moveCommand.substr(0, i);
						// cout << moveString << endl;
						int move = parseMove(game, moveString);
						if (move != 0)
						{
							game.makeMove(move);
						}
						moveCommand.erase(0, i);	
					}
				}
			}
		}
		else
		{
			this -> resetEngine();
		}
	}
	else
	{
		if (command.compare(9, 3, "fen", 3) != 0) 
		{
			cout << "Couldn't find fen command..." << endl; game.parseFen(START_POSITION); 
		}
		else
		{
			string moveCommand = command;
			moveCommand.erase(0, 13);
			string fen = command;
			fen.erase(0, 13);
			int spaceCount = 0;
			int i;
			for (i = 0; i < 100; i++)
			{
				if (fen[i] == ' ') spaceCount ++;
				if (spaceCount >= 5) {i += 2; break;}
			}
			fen = fen.substr(0, i);
			game.parseFen(fen);

			moveCommand.erase(0, fen.length());
			if (moveCommand.length() > 0 && moveCommand.compare(1, 5, "moves", 5) == 0)
			{
				moveCommand.erase(0, 7);
				while (moveCommand.length())
				{
					for (int i = 0; i < 6; i++)
					{
						if (i < moveCommand.length() && moveCommand[i] == ' ')
						{
							string moveString = moveCommand.substr(0, i);
							int move = parseMove(game, moveString);
							if (move != 0)
							{
								this -> repetitionIndex ++;
								this -> repetitionTable[this -> repetitionIndex] = game.hashKey;
								game.makeMove(move);
							}
							moveCommand.erase(0, i + 1);
						}
						else if (i == moveCommand.length())
						{
							string moveString = moveCommand.substr(0, i);
							int move = parseMove(game, moveString);
							if (move != 0)
							{
								this -> repetitionIndex ++;
								this -> repetitionTable[this -> repetitionIndex] = game.hashKey;
								game.makeMove(move);
							}
							moveCommand.erase(0, i);	
						}
					}
				}
			}
			else
			{
				this -> resetEngine();
			}			
		}
	}
}

/* Referencing from: http://wbec-ridderkerk.nl/html/UCIProtocol.html */

void Engine::parseGo(Game& game, string command)
{
	int depth = -1;
	int pos;

    // infinite search: search until the "stop" command.
    if ((pos = command.find("infinite")) != string::npos) {}

    // match UCI "binc" command: black increment per move in mseconds if x > 0
    if ((pos = command.find("binc")) != string::npos && game.side == BLACK)
        // parse black time increment
        inc = atoi(&command[pos + 5]);

    // match UCI "winc" command: white increment per move in mseconds if x > 0
    if ((pos = command.find("winc")) != string::npos && game.side == WHITE)
        // parse white time increment
        inc = atoi(&command[pos + 5]);

    // match UCI "wtime" command: white has x msec left on the clock
    if ((pos = command.find("wtime")) != string::npos && game.side == WHITE)
        // parse white time limit
        uciTime = atoi(&command[pos + 6]);

    // match UCI "btime" command: black has x msec left on the clock
    if ((pos = command.find("btime")) != string::npos && game.side == BLACK)
        // parse black time limit
        uciTime = atoi(&command[pos + 6]);

    // match UCI "movestogo" command: there are x moves to the next time control, this will only be sent if x > 0,
    if ((pos = command.find("movestogo")) != string::npos)
        // parse number of moves to go
        movestogo = atoi(&command[pos + 10]);

    // match UCI "movetime" command: search exactly x mseconds
    if ((pos = command.find("movetime")) != string::npos)
        // parse amount of time allowed to spend to make a move
        movetime = atoi(&command[pos + 9]);

    // match UCI "depth" command: search x plies only
    if ((pos = command.find("depth")) != string::npos)
        // parse search depth
        depth = atoi(&command[pos + 6]);

    // if move time is available
    if(movetime != -1)
    {
    	if (movetime > 5000)
    	{
    		movetime = 5000;
    	}

        // set time equal to move time
        uciTime = movetime;

        // set moves to go to 1
        movestogo = 1;
    }

    // init start time
    starttime = getTimems();

    // init search depth
    depth = depth;

    // if time control is available
    if(uciTime != -1)
    {
        // flag we're playing with time control
        timeset = 1;

        // set up timing
        uciTime /= movestogo;
        uciTime -= 100;
        stoptime = starttime + uciTime + inc / 3;
    }

    // if depth is not available
    if(depth == -1)
        // set depth to 64 plies (takes ages to complete...)
        depth = 24;

    // print debug info
    if (DEBUG_ENGINE)
    {
	    cout << "time " << uciTime << " start " << (uint64_t)starttime << " stop " << (uint64_t)stoptime << " depth " << depth << " timeset " << timeset << endl;
		cout << "movetime " << movetime << " movestogo " << movestogo << endl;
    }

	this -> search(game, depth);

	int bestMove = this -> pvTable[0][0];
	int promotion = getPromotion(bestMove);
	if (promotion != NULL_PIECE)
	{
		if (promotion == Q || promotion == q) 
		{
			cout << "bestmove " << SQUARES[getStartSquare(bestMove)] << SQUARES[getEndSquare(bestMove)] << 'q' << endl;
		}
		if (promotion == R || promotion == r) 
		{
			cout << "bestmove " << SQUARES[getStartSquare(bestMove)] << SQUARES[getEndSquare(bestMove)] << 'r' << endl;
		}
		if (promotion == N || promotion == n) 
		{
			cout << "bestmove " << SQUARES[getStartSquare(bestMove)] << SQUARES[getEndSquare(bestMove)] << 'n' << endl;
		}
		if (promotion == B || promotion == b) 
		{
			cout << "bestmove " << SQUARES[getStartSquare(bestMove)] << SQUARES[getEndSquare(bestMove)] << 'b' << endl;
		}			
	}
	else
	{
		cout << "bestmove " << SQUARES[getStartSquare(bestMove)] << SQUARES[getEndSquare(bestMove)] << endl;
	}
	if (bestMove != 0)
	{
		this -> repetitionIndex ++;
		this -> repetitionTable[this -> repetitionIndex] = game.hashKey;
		game.makeMove(bestMove);
	}
}

// get time in milliseconds
int Engine::getTimems()
{
    struct timeval time_value;
    gettimeofday(&time_value, NULL);
    return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
}

/* https://github.com/maksimKorzh/chess_programming/blob/master/src/bbc/handling_timings/bbc.c */
// Need to add <unistd.h> header file
int Engine::waitForInput()
{
	fd_set readfds;
	struct timeval tv;
	FD_ZERO (&readfds);
	FD_SET (fileno(stdin), &readfds);
	tv.tv_sec=0; tv.tv_usec=0;
	select(16, &readfds, 0, 0, &tv);

	return (FD_ISSET(fileno(stdin), &readfds));
}

// Read GUI / user input
void Engine::readInput()
{
	// bytes to read holder
	int bytes;
	// GUI/user input
	char input[256] = "", *endc;
	// "listen" to STDIN
	if (waitForInput())
	{
	    // tell engine to stop calculating
	    stopped = 1;
	    // loop to read bytes from STDIN
	    do
	    {
	        // read bytes from STDIN
	        bytes=read(fileno(stdin), input, 256);
	    }
	    // until bytes available
	    while (bytes < 0);
	    // searches for the first occurrence of '\n'
	    endc = strchr(input,'\n');
	    // if found new line set value at pointer to 0
	    if (endc) *endc=0;
	    // if input is available
	    if (strlen(input) > 0)
	    {
	        // match UCI "quit" command
	        if (!strncmp(input, "quit", 4))
	        {
	            // tell engine to terminate execution    
	            quit = 1;
	        }
	        // // match UCI "stop" command
	        else if (!strncmp(input, "stop", 4))    {
	            // tell engine to terminate execution
	            quit = 1;
	        }
	    }   
	}
}

// a bridge function to interact between search and GUI input
void Engine::communicate() {
	// if time is up break here
    if(timeset == 1 && getTimems() > stoptime) {
    	if (DEBUG_ENGINE && stopped != 1) { cout << "times up!" << endl; }
		// tell engine to stop calculating
		stopped = 1;
	}
    // read GUI input
	readInput();
};
