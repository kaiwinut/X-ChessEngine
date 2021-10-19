#include <iostream>
#include <cstring>
#include "bitboard.h"
#include "types.h"
#include "masks.h"
#include "movegen.h"
#include "eval.h"
#include "search.h"

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
	nodes = 0, ply = 0, bestEval = 0, inPV = 0, scorePV = 0;
	duration = 0.0, knps = 0.0;

	for (int currentDepth = 1; currentDepth <= depth; currentDepth ++)
	{
		// Follow the principle variation by default
		inPV = 1;
		// Run the principle variation search and measure the time
		clock_t startTime = clock();
		bestEval = PVS(currentDepth, -INF, INF);
		clock_t endTime = clock();	
		// Calculate the time and kilo nodes per second
		duration = (float) (endTime - startTime) * 1000.0 / CLOCKS_PER_SEC;
		knps = ((float)nodes) / duration;

		// Second parameter toggles debug mode
		printResults(currentDepth, depth, DEBUG_ENGINE);
	}
}

int Engine::PVS(int depth, int alpha, int beta)
{
	pvLength[ply] = ply;

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
		GameState prevState = game.makeNullMove();
		int score = -PVS(depth - 1 - NULL_MOVE_REDUCTION, -beta, -beta + 1);
		game.takeBack(prevState);

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

	while (*move != 0)
	{
		ply ++;
		GameState prevState = game.makeMove(*move, ALL_MOVES);
		if (!prevState.valid)
		{
			ply --;
			move ++;
			continue;
		}
		legalMoves ++;

		int moveIsCheck = game.isSquareAttacked(getLeastSignificantBitIndex((game.side == WHITE ? game.bitboards[K] : game.bitboards[k])), game.side ^ 1);

		int score;

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
			// if (movesSearched >= FULL_DEPTH_MOVES && depth >= REDUCTION_LIMIT && isInCheck == 0 && getCaptureFlag(*move) == 0 && getPromotion(*move) == NULL_PIECE)
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
		movesSearched ++;

		if (score >= beta)
		{
			if (getCaptureFlag(*move) == 0)
			{
				killerMoves[ply][1] = killerMoves[ply][0];
				killerMoves[ply][0] = *move;
				historyMoves[getPiece(*move)][getEndSquare(*move)] += depth * depth;
			}
			return beta;
		}

		if (score > alpha)
		{
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

	return alpha;
}

int Engine::quiescenceSearch(int alpha, int beta)
{
	nodes ++;
	int evaluation = evaluate(game);

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
	
	while (*move != 0)
	{
		ply ++;
		GameState prevState = game.makeMove(*move, ONLY_CAPTURES);
		if (!prevState.valid)
		{
			ply --;
			move ++;
			continue;
		}
		int score = -quiescenceSearch(-beta, -alpha);
		game.takeBack(prevState);
		ply --;

		if (score >= beta)
		{
			return beta;
		}

		if (score > alpha)
		{
			alpha = score;
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

	if (getCaptureFlag(move) == 1)
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
			break;
		}
		move++;
	}
}

void Engine::resetEngine()
{
	nodes = 0, ply = 0, bestEval = 0, inPV = 0, scorePV = 0;
	duration = 0.0, knps = 0.0;

	memset(killerMoves, 0, sizeof(killerMoves));
	memset(historyMoves, 0, sizeof(historyMoves));
	memset(pvLength, 0, sizeof(pvLength));
	memset(pvTable, 0, sizeof(pvTable));
}

void Engine::printResults(int currentDepth, int depth_limit, int debug)
{
	// Show full result
	if (debug)
	{
		cout << "info depth " << currentDepth << " score cp " << bestEval << " time " << (int)duration << " nodes " << nodes << " nps " << (int)(knps * 1000) << " pv ";
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
		cout << '\n' << endl;	
	}
	// If not in debug mode, print results in uci protocol style
	else
	{
		if (currentDepth == depth_limit)
		{
			cout << "info depth " << currentDepth << " score cp " << bestEval << " time " << (int)duration << " nodes " << nodes << " nps " << (int)(knps * 1000) << " pv";
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