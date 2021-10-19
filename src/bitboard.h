#ifndef BITBOARD_H
#define BITBOARD_H

#include <iostream>
#include "types.h"

// Check if a bitboard contains a certain bit (square)
Bitboard getBit(Bitboard board, int square);

// Set a bit at a given index in the bitboard
Bitboard setBit(Bitboard board, int square);

// Remove a bit at a given index in the bitboard
Bitboard popBit(Bitboard board, int square);

// Count bits that are 1 in a bitboard
int countBits(Bitboard board);

// Retrieve the index of the least significant bit in a bitboard
int getLeastSignificantBitIndex(Bitboard board);

// Display bitboard in 8 x 8 style
void displayBitboard(Bitboard board);

#endif