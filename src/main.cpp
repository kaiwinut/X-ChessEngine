#include <iostream>
#include "bitboard.h"
#include "types.h"
#include "masks.h"
#include "movegen.h"
#include "perft.h"
#include "eval.h"
#include "search.h"
#include "uci.h"

using std::cout;
using std::endl;
using std::string;

int main()
{
	// Initilize all pre-grenerated look up lists needed
	AttackMasks::init();
	Evaluation::init();
	// Perft(START_POSITION, 2);
	// Game game(START_POSITION);
	// Game game("1R6/5bkp/P7/2pp1p2/5P2/2P2r2/3K2p1/8 b - - 0 55");
	// game.displayGame();
	// game.generateAllMoves();
	// Engine engine;
	// engine.search(game, 7);
	UCILoop();

	return 0;
}