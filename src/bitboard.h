#ifndef BITBOARD_H
#define BITBOARD_H

#include <iostream>
#include "types.h"

Bitboard getBit(Bitboard board, int square);
Bitboard setBit(Bitboard board, int square);
Bitboard popBit(Bitboard board, int square);
int getLeastSignificantBitIndex(Bitboard board);
int countBits(Bitboard board);

void displayBitboard(Bitboard board);

#endif