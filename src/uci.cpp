#include <iostream>
#include <string>
#include "types.h"
#include "perft.h"
#include "movegen.h"
#include "search.h"
#include "uci.h"

using std::cout;
using std::endl;
using std::string;

void uciLoop()
{
	std::cin.clear();

	string input;
	// cout << "id name Kaaaiiiwww" << endl;
	// cout << "id name Kai" << endl;
	// cout << "uciok" << endl;

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
			// game.displayGame();
		}		

		// parse uci new game command
		else if (input.compare(0, 10, "ucinewgame", 10) == 0)
		{
			parsePosition(game, "position startpos");
			// game.displayGame();
		}	

		// parse uci position command
		else if (input.compare(0, 2, "go", 2) == 0)
		{
			parseGo(game, input);
		}	

		else if (input.compare(0, 4, "quit", 4) == 0)
		{
			break;
		}

		else if (input.compare(0, 3, "uci", 3) == 0)
		{
			cout << "id name Kaaaiiiwww" << endl;
			cout << "id name Kai" << endl;
			cout << "uciok" << endl;
		}
	}
}

// Parse user/GUI move command (e.g. e7e8q)
int parseMove(Game& game, string moveString)
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
void parsePosition(Game& game, string command) 
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
		}
	}
}

void parseGo(Game& game, string command)
{
	int depth = -1;
	string depthCommand = command;

	if (command.compare(3, 5, "depth", 5) == 0)
	{
		depthCommand.erase(0, 9);
		depth = stoi(depthCommand);
		// cout << depthCommand << endl;
		Search search(game, depth);
		cout<< "bestmove " << SQUARES[getStartSquare(search.pvTable[0][0])] << SQUARES[getEndSquare(search.pvTable[0][0])] << endl;
	}
}

