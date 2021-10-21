#include <iostream>
#include "bitboard.h"
#include "types.h"
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

	if (DEBUG_EVAL == 1 || DEBUG_MASK == 1 || DEBUG_GAME == 1 || DEBUG_ENGINE == 1|| DEBUG_UCI == 1)
	{
		Engine engine;
		engine.uciLoop();
	}
	else
	{
		Engine engine;
		engine.uciLoop();
	}
	return 0;
}