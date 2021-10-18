#ifndef PERFT_H
#define PERFT_H

#include <iostream>
#include "movegen.h"

class Perft
{
public:
	std::string fen;
	int depth;
	int nodesSearched;
	float duration;
	float knps;
	Game game;
	
	Perft(std::string fen, int depth);
	void perft_driver(int depth);
};

// FEN strings and constants for perft test
const std::string EMPTY_BOARD = "8/8/8/8/8/8/8/8 w - - 0 0";
const std::string START_POSITION = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const std::string TRICKY_POSITION = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
const std::string CMK_POSITION = "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9";

const uint64_t START_POSITION_PERFT[7] = {20, 400, 8902, 197281, 4865609, 119060324, 3195901860};
const uint64_t TRICY_POSITION_PERFT[6] = {48, 2039, 97862, 4085603, 193690690, 8031647685};

#endif