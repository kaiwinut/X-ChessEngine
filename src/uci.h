#ifndef UCI_H
#define UCI_H

#include <iostream>
#include "movegen.h"

int parseMove(Game& game, std::string moveString);
void parsePosition(Game& game, std::string command);
void parseGo(Game& game, std::string command);
void uciLoop();

#endif