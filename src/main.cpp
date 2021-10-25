#include <iostream>
#include "utils.h"
#include "masks.h"
#include "movegen.h"
#include "perft.h"
#include "eval.h"
#include "engine.h"

using std::cout;
using std::endl;

int main()
{
	// Initilize all pre-grenerated look up lists needed
	AttackMasks::init();
	Evaluation::init();

	if (DEBUG_EVAL == 1 || DEBUG_MASK == 1 || DEBUG_PERFT == 1 || DEBUG_GAME == 1 || DEBUG_ENGINE == 1|| DEBUG_UCI == 1)
	{
		// Perft(START_POSITION, 6);//
		// r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1
		Game game(TRICKY_POSITION);
		// Game game(START_POSITION);
		// Game game("8/1b2q3/8/5pn1/4K3/8/8/4r3 w - - 0 0");
		displayGame(game);
		// displayBitboard(game.checkMask);
		Engine engine;
		engine.search(game, 10);
		// engine.uciLoop();
	}
	else
	{
		Engine engine;
		engine.uciLoop();
	}
	return 0;
}