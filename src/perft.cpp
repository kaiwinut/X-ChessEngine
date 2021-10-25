#include <iostream>
#include <time.h>
#include "utils.h"
#include "movegen.h"
#include "perft.h"

using std::cout;
using std::endl;
using std::string;

Perft::Perft(string fen, int depth)
{	
	depth = depth;
	nodesSearched = 0, duration = 0, knps = 0;
	game.parseFen(fen);

	cout << "\nStarting perft at depth " << depth << '!' << endl; 
	cout << "..." << endl;
	cout << "...\n" << endl;
	displayGame(game);

	clock_t startTime = clock();
	perft_driver(depth);
	clock_t endTime = clock();	

	duration = (endTime - startTime) * 1000.0 / CLOCKS_PER_SEC;
	knps = nodesSearched / duration;
	
	cout << "========================" << endl;
	cout << "Performance Test Results" << endl;
	cout << endl;
	cout << "Depth: " << depth << endl;
	cout << "Nodes searched: " << nodesSearched << endl;
	// cout << "Nodes searched: " << nodesSearched << " / " << START_POSITION_PERFT[depth - 1] << endl;
	cout << "Duration: " << duration << " ms" << endl;
	cout << "Nodes per second: " << knps << " KNps" << endl;
	cout << "========================" << endl;	
}

void Perft::perft_driver(int depth)
{
	if (depth == 0) { nodesSearched ++ ; return; }

	game.generateAllMoves();
	int *move = game.moveList;
	while (*move != 0)
	{
		GameState prevState = game.makeMove(*move, ALL_MOVES);
		if (!prevState.valid)
		{
			move++;
			continue;
		}
		perft_driver(depth - 1);
		game.takeBack(prevState);
		move++;
	}
}