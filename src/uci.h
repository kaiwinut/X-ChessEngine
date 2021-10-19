#ifndef UCI_H
#define UCI_H

#include <iostream>
#include "movegen.h"

class UCILoop
{
public:
	Game game;
	Engine engine;

	UCILoop();	
	int parseMove(std::string moveString);
	void parsePosition(std::string command);
	void parseGo(std::string command);
};


#endif