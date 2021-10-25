#include <iostream>
#include <cstring>
#include "utils.h"
#include "masks.h"

using std::cout;
using std::endl;
using std::hex;
using std::dec;

// Pre-generated look up lists
Bitboard PAWN_ATTACKS[2][64];
Bitboard KNIGHT_ATTACKS[64];
Bitboard KING_ATTACKS[64];
Bitboard BISHOP_ATTACKS[64][512];
Bitboard BISHOP_RELEVANT_OCCUPANCY[64];
Bitboard ROOK_ATTACKS[64][4096];
Bitboard ROOK_RELEVANT_OCCUPANCY[64];

void AttackMasks::init()
{
	// Initialize magics (Not needed since they are already initialized as constants)
	// initMagics();
	// Initialize attacks for leapers (pawns, knights, and kings)
	initLeaperAttacks();
	// Initialize attacks for rooks
	initSlideAttacks(1);
	// Initialize attacks for bishops
	initSlideAttacks(0);

	if (DEBUG_MASK)
	{
		printDebug();
	}
}

/***   Pre-generation of pieces' attacking moves   ***/

// Generate pawn attacks for pre-generated pawn attack list
Bitboard generatePawnAttacks(int square, int side)
{
	Bitboard mask = 0x0;
	Bitboard piece = setBit(0x0, square);

	// generate moves for white pawns
	if (side == WHITE)
	{
		// make sure that all moves are inside the bounds
		if (piece & ~FILE_A_MASK)
		{
			mask |= piece << NORTH_WEST;
		}
		if (piece & ~FILE_H_MASK)
		{
			mask |= piece << NORTH_EAST;
		}
	}
	// generate moves for black pawns
	else
	{
		if ((piece & ~FILE_A_MASK))
		{
			mask |= piece >> -SOUTH_WEST;
		}
		if ((piece & ~FILE_H_MASK))
		{
			mask |= piece >> -SOUTH_EAST;
		}
	}

	return mask;
}

// Generate knight attacks for pre-generated knight attack list
Bitboard generateKnightAttacks(int square)
{
	Bitboard mask = 0x0;
	Bitboard piece = setBit(0x0, square);

	// make sure that all moves are inside the bounds
	if (piece & ~(FILE_A_MASK | FILE_B_MASK))
	{
		mask |= piece << (NORTH_WEST + WEST);
		mask |= piece >> -(SOUTH_WEST + WEST);
	}
	if (piece & ~(FILE_G_MASK | FILE_H_MASK))
	{
		mask |= piece << (NORTH_EAST + EAST);
		mask |= piece >> -(SOUTH_EAST + EAST);
	}
	if (piece & ~FILE_A_MASK)
	{
		mask |= piece << (NORTH_WEST + NORTH);
		mask |= piece >> -(SOUTH_WEST + SOUTH);
	}
	if (piece & ~FILE_H_MASK)
	{
		mask |= piece << (NORTH_EAST + NORTH);
		mask |= piece >> -(SOUTH_EAST + SOUTH);
	}

	return mask;
}

// Generate knight attacks for pre-generated knight attack list
Bitboard generateKingAttacks(int square)
{
	Bitboard mask = 0x0;
	Bitboard piece = setBit(0x0, square);

	mask |= piece << NORTH;
	mask |= piece >> -SOUTH;

	// make sure that all moves are inside the bounds
	if (piece & ~FILE_A_MASK)
	{
		mask |= piece << NORTH_WEST;
		mask |= piece >> -WEST;		
		mask |= piece >> -SOUTH_WEST;
	}
	if (piece & ~FILE_H_MASK)
	{
		mask |= piece << NORTH_EAST;
		mask |= piece << EAST;
		mask |= piece >> -SOUTH_EAST;
	}

	return mask;
}

// Generate bishop attacks without considering blocks. Note that edges are not included.
Bitboard generateBishopRelevantOccupancyMask(int square)
{
	Bitboard mask = 0x0;
	Bitboard piece = setBit(0x0, square);

	int rank = square / 8;
	int file = square % 8;

	for (int i = 1; i < 7; i ++)
	{
		mask |= piece << ((rank + i < 7 && file + i < 7) ? (NORTH_EAST * i) : 0);
		mask |= piece << ((rank + i < 7 && file - i > 0) ? (NORTH_WEST * i) : 0);
		mask |= piece >> ((rank - i > 0 && file + i < 7) ? -(SOUTH_EAST * i) : 0);
		mask |= piece >> ((rank - i > 0 && file - i > 0) ? -(SOUTH_WEST * i) : 0);
	}

	mask = popBit(mask, square);

	return mask;
}

// Generate rook attacks without considering blocks. Note that edges are not included.
Bitboard generateRookRelevantOccupancyMask(int square)
{
	Bitboard mask = 0x0;
	Bitboard piece = setBit(0x0, square);

	int rank = square / 8;
	int file = square % 8;

	for (int i = 1; i < 7; i ++)
	{
		mask |= piece << ((rank + i < 7) ? (NORTH * i) : 0);
		mask |= piece << ((file + i < 7) ? (EAST * i) : 0);
		mask |= piece >> ((rank - i > 0) ? -(SOUTH * i) : 0);
		mask |= piece >> ((file - i > 0) ? -(WEST * i) : 0);
	}

	mask = popBit(mask, square);

	return mask;
}

// Generate one of all the possible combinations of occupancies given the square and the relevant occupancy mask
Bitboard generateOccupancyMasks(int index, int maskBitCount, Bitboard relevantOccupancyMask)
{
	Bitboard occupancy = 0x0;
	int square;

	for (int count = 0; count < maskBitCount; count ++)
	{
		square = getLeastSignificantBitIndex(relevantOccupancyMask);
		relevantOccupancyMask = popBit(relevantOccupancyMask, square);

		// If index has exceeded 2 ^ (relevant occupancy mask's bit count), loop over the results again
		if (index & (1ULL << count))
		{
			occupancy |= (1ULL << square);
		}

	}
	return occupancy;
}

// Generate bishop attacks while taking blocks in account. Note that edges are included in this case.
Bitboard generateBishopAttacksOnTheFly(int square, Bitboard occupancy)
{
	Bitboard mask = 0x0;
	Bitboard piece = setBit(0x0, square);

	int rank = square / 8;
	int file = square % 8;

	for (int i = 1; i < 8; i ++)
	{
		if (rank + i < 8 && file + i < 8)
		{
			mask |= piece << (NORTH_EAST * i);
			if ((piece << (NORTH_EAST * i)) & occupancy) { break; }
		}
		else break;
	}

	for (int i = 1; i < 8; i ++)
	{
		if (rank + i < 8 && file - i >= 0)
		{
			mask |= piece << (NORTH_WEST * i);
			if ((piece << (NORTH_WEST * i)) & occupancy) { break; }
		}
		else break;
	}
	for (int i = 1; i < 8; i ++)
	{
		if (rank - i >= 0 && file + i < 8)
		{
			mask |= piece >> -(SOUTH_EAST * i);
			if ((piece >> -(SOUTH_EAST * i)) & occupancy) { break; }
		}
		else break;
	}
	for (int i = 1; i < 8; i ++)
	{
		if (rank - i >= 0 && file - i >= 0)
		{
			mask |= piece >> -(SOUTH_WEST * i);
			if ((piece >> -(SOUTH_WEST * i)) & occupancy) { break; }
		}
		else break;
	}

	return mask;
}

// Generate rook attacks while taking blocks in account. Note that edges are included in this case.
Bitboard generateRookAttacksOnTheFly(int square, Bitboard occupancy)
{
	Bitboard mask = 0x0;
	Bitboard piece = setBit(0x0, square);

	int rank = square / 8;
	int file = square % 8;

	for (int i = 1; i < 8; i ++)
	{
		if (rank + i < 8)
		{
			mask |= piece << (NORTH * i);
			if ((piece << (NORTH * i)) & occupancy) { break; }
		}
		else break;
	}

	for (int i = 1; i < 8; i ++)
	{
		if (file + i < 8)
		{
			mask |= piece << (EAST * i);
			if ((piece << (EAST * i)) & occupancy) { break; }
		}
		else break;
	}
	for (int i = 1; i < 8; i ++)
	{
		if (rank - i >= 0)
		{
			mask |= piece >> -(SOUTH * i);
			if ((piece >> -(SOUTH * i)) & occupancy) { break; }
		}
		else break;
	}
	for (int i = 1; i < 8; i ++)
	{
		if (file - i >= 0)
		{
			mask |= piece >> -(WEST * i);
			if ((piece >> -(WEST * i)) & occupancy) { break; }
		}
		else break;
	}

	return mask;
}

// Initialize pawns', knights', and kings' pseudo legal move look up
void initLeaperAttacks()
{
	for (int square = A1; square <= H8; square++)
	{
		PAWN_ATTACKS[WHITE][square]	= generatePawnAttacks(square, WHITE);
		PAWN_ATTACKS[BLACK][square]	= generatePawnAttacks(square, BLACK);
		KNIGHT_ATTACKS[square] = generateKnightAttacks(square);
		KING_ATTACKS[square] = generateKingAttacks(square);
	}
}

// Initialize rooks' and bishops' pseudo legal move look up
void initSlideAttacks(int isRook)
{
	for (int square = A1; square <= H8; square++)
	{
		BISHOP_RELEVANT_OCCUPANCY[square] = generateBishopRelevantOccupancyMask(square);
		ROOK_RELEVANT_OCCUPANCY[square] = generateRookRelevantOccupancyMask(square);
	}
	int magicIndex = 0;
	for (int square = A1; square <= H8; square++)
	{
		Bitboard relevantOccupancyMask = (isRook ? ROOK_RELEVANT_OCCUPANCY[square] : BISHOP_RELEVANT_OCCUPANCY[square]);
		int maskBitCount = countBits(relevantOccupancyMask);

		for (int index = 0; index < (1 << maskBitCount); index ++)
		{
			if (isRook)
			{
				Bitboard occupancy = generateOccupancyMasks(index, maskBitCount, relevantOccupancyMask);
				magicIndex = ((occupancy * ROOK_MAGICS[square]) >> (64 - ROOK_OCCUPANCY_COUNT[square]));
				ROOK_ATTACKS[square][magicIndex] = generateRookAttacksOnTheFly(square, occupancy);
			}
			else
			{
				Bitboard occupancy = generateOccupancyMasks(index, maskBitCount, relevantOccupancyMask);
				magicIndex = ((occupancy * BISHOP_MAGICS[square]) >> (64 - BISHOP_OCCUPANCY_COUNT[square]));
				BISHOP_ATTACKS[square][magicIndex] = generateBishopAttacksOnTheFly(square, occupancy);
			}
		}
	}
}

uint32_t randomSeed = 1804289383;

uint32_t generateRandomUint32()
{
	uint32_t number = randomSeed;
	number ^= (number << 13);
	number ^= (number >> 17);
	number ^= (number << 5);
	randomSeed = number;
	return number;
}

uint64_t generateRandomUint64()
{
	uint64_t n1 = (uint64_t)(generateRandomUint32() & 0xFFFF);
	uint64_t n2 = (uint64_t)(generateRandomUint32() & 0xFFFF);
	uint64_t n3 = (uint64_t)(generateRandomUint32() & 0xFFFF);
	uint64_t n4 = (uint64_t)(generateRandomUint32() & 0xFFFF);
	return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

/*** Generation of magic numbers (Uncomment functions to run in debug mode) ***/

uint64_t generateMagicCandidate()
{
	return generateRandomUint64() & generateRandomUint64() & generateRandomUint64();
}

Bitboard findMagicNumber(int square, int maskBitCount, int isRook)
{
	// 4096 is the number of all possible occupancy combinations since rook's maximum occupancy count is 12
	Bitboard occupancies[4096];
	// An occupancy pattern matches an attack pattern
	Bitboard attackMasks[4096];
	// An array that stores the used attacks
	Bitboard usedAttacks[4096];
	Bitboard relevantOccupancyMask = (isRook ? generateRookRelevantOccupancyMask(square) : generateBishopRelevantOccupancyMask(square));
	// Loop over every possible occupancy pattern
	for (int index = 0; index < (1 << maskBitCount); index ++)
	{
		occupancies[index] = generateOccupancyMasks(index, maskBitCount, relevantOccupancyMask);
		attackMasks[index] = (isRook ? generateRookAttacksOnTheFly(square, occupancies[index]) : generateBishopAttacksOnTheFly(square, occupancies[index]));
	}
	// Brute forcing to find the magic number
	for (int r = 0; r < 100000000; r ++)
	{
		// A flag that indicates if a magic number failed
		int flag = 0;
		// Randomly generate a magic number candidate
		Bitboard magicNumber = generateMagicCandidate();
		// Skip inappropriate magic numbers (all bits should line up from the 8th rank after the multiplication)
		if (countBits((relevantOccupancyMask * magicNumber) & RANK_8_MASK) < 6) { continue; }

		memset(usedAttacks, 0ULL, sizeof(usedAttacks));

		for (int index = 0; index < (1 << maskBitCount); index ++)
		{
			int magicIndex = ((occupancies[index] * magicNumber) >> (64 - maskBitCount));
			if (usedAttacks[magicIndex] == 0)
			{
				usedAttacks[magicIndex] = attackMasks[index];
			}
			else
			{
				flag = 1;
				break;
			}
		}
		if (flag) { continue; }
		else { return magicNumber; }
	}
	cout << "Failed to find magic number..." << endl;
	return 0ULL;
}

void initMagics()
{
	cout << "Rook's magics: " << endl;
	for (int square = A1; square <= H8; square++)
	{
		cout << "0x" << hex << findMagicNumber(square, ROOK_OCCUPANCY_COUNT[square], 1) << "ULL, " << endl;
	}
	cout << "\n\n" << endl;
	cout << "Bishop's magics: " << endl;
	for (int square = A1; square <= H8; square++)
	{
		cout << "0x" << hex << findMagicNumber(square, BISHOP_OCCUPANCY_COUNT[square], 0) << "ULL, " << endl;
	}
	cout << dec;
}

void AttackMasks::printDebug()
{
	/*** Uncomment this to check the attacking moves library ***/
	cout << "White pawn's pseudo legal moves:" << endl;
	for (int square = A1; square <= H8; square++)
	{
		displayBitboard(PAWN_ATTACKS[WHITE][square]);
	}
	cout << "Black pawn's pseudo legal moves:" << endl;
	for (int square = A1; square <= H8; square++)
	{
		displayBitboard(PAWN_ATTACKS[BLACK][square]);
	}
	cout << "Knight's pseudo legal moves:" << endl;
	for (int square = A1; square <= H8; square++)
	{
		displayBitboard(KNIGHT_ATTACKS[square]);
	}
	cout << "King's pseudo legal moves:" << endl;
	for (int square = A1; square <= H8; square++)
	{
		displayBitboard(KING_ATTACKS[square]);
	}

	Bitboard occupancy = 0x0;
	occupancy = setBit(occupancy, C4);
	occupancy = setBit(occupancy, E6);
	occupancy = setBit(occupancy, G8);
	occupancy = setBit(occupancy, G7);
	occupancy = setBit(occupancy, G2);
	cout << "Occupancy example:" << endl;	
	displayBitboard(occupancy);

	cout << "Bishop's pseudo legal masks:" << endl;
	for (int square = A1; square <= H8; square++)
	{
		displayBitboard(generateBishopAttacks(square, occupancy));
	}
	cout << "Rook's pseudo legal masks:" << endl;
	for (int square = A1; square <= H8; square++)
	{
		displayBitboard(generateRookAttacks(square, occupancy));
	}
	cout << "Queen's pseudo legal masks:" << endl;
	for (int square = A1; square <= H8; square++)
	{
		displayBitboard(generateQueenAttacks(square, occupancy));
	}
}