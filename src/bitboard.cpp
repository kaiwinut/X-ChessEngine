#include <iostream>
#include "types.h"
#include "bitboard.h"

using std::cout;
using std::endl;
using std::hex;
using std::dec;

Bitboard getBit(Bitboard board, int square)
{
	return board & (1ULL << square);
}

Bitboard setBit(Bitboard board, int square)
{
	return board | (1ULL << square);	
}

Bitboard popBit(Bitboard board, int square)
{
	return board & ~(1ULL << square);
}

int countBits(Bitboard board)
{
	int count = 0;
	
	while (board)
	{
		board &= board - 1;
		count ++;
	}

	return count;
}

/*
	Algorithm: (0100 & 1100) - 1 = 0011. First bit has an index of 0.
*/
int getLeastSignificantBitIndex(Bitboard board)
{
	if (board)
	{
		return countBits((board & -board) - 1);
	}
	else
	{
		// Returns an illegal bit if board equals 0
		return -1;
	}
}

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