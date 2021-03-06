#ifndef PERFT_H
#define PERFT_H

#include <iostream>
#include "movegen.h"

class Perft
{
public:
	std::string fen;
	int depth;
	long nodesSearched;
	int duration;
	int knps;
	Game game;
	
	Perft(std::string fen, int depth);
	void perft_driver(int depth);
};

#endif