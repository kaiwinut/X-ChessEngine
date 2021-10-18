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
	// Initilize all libraries needed
	AttackMasks::init();
	Evaluation::init();

	// Perft(START_POSITION, 6);
	// Game game("4k3/Q7/4K3/8/8/8/8/8 w - - 0 0");
	// Game game(TRICKY_POSITION);
	// game.generateAllMoves();
	// parsePosition(game, command);
	// Search(game, 7);
	uciLoop();

	return 0;
}