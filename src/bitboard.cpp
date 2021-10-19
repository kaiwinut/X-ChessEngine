#include <iostream>
#include "types.h"
#include "bitboard.h"

using std::cout;
using std::endl;
using std::hex;
using std::dec;

// Check if a bitboard contains a certain bit (square)
Bitboard getBit(Bitboard board, int square)
{
	return board & (1ULL << square);
}

// Set a bit at a given index in the bitboard
Bitboard setBit(Bitboard board, int square)
{
	return board | (1ULL << square);	
}

// Remove a bit at a given index in the bitboard
Bitboard popBit(Bitboard board, int square)
{
	return board & ~(1ULL << square);
}

// Count bits that are 1 in a bitboard
int countBits(Bitboard board)
{
	int count = 0;
	
	while (board)
	{
		// This operation removes the last (right-most) bit every time
		board &= board - 1;
		count ++;
	}

	return count;
}

/*
	Retrieve the index of the least significant bit in a bitboard.
	Example: (   4  &  -4) - 1 =    3
			 (0100 & 1100) - 1 = 0011
	0011 contains two bits that are 1, which tells us that the index 
	of the least significant bit of bitboard 4 (0100) is 2.
*/
int getLeastSignificantBitIndex(Bitboard board)
{
	if (board)
	{
		return countBits((board & -board) - 1);
	}
	else
	{
		// Return illegal bit if board equals 0
		return -1;
	}
}

// Display bitboard in 8 x 8 style
void displayBitboard(Bitboard board)
{
	int square;
	cout << "Board: 0x" << hex << board << endl;
	cout << dec;
	for (int rank = RANK_8; rank >= RANK_1; rank--)
	{
		// Uncomment this to show ranks
		// cout << rank << "  ";
		for (int file = FILE_A; file <= FILE_H; file++)
		{
			square = rank * 8 + file;
			cout << (getBit(board, square) ? "1 " : ". ");
		}
		cout << endl;
	}
	cout << endl;
	// Uncomment this to show files
	// cout << "   a b c d e f g h\n" << endl;
}