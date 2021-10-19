#include "bitboard.h"
#include "types.h"
#include "masks.h"
#include "eval.h"

Bitboard FILE_MASKS[64];
Bitboard RANK_MASKS[64];
Bitboard ISOLATED_MASKS[64];
Bitboard WHITE_PASS_MASKS[64];
Bitboard BLACK_PASS_MASKS[64];

void Evaluation::init()
{
	// Initilize masks that help determine pawn structures, king safety and piece mobility
	generateFileMasks();
	generateRankMasks();
	generateIsolatedMasks();
	generatePassMasks();

	if (DEBUG_EVAL)
	{
		printDebug();
	}
}

// Give a static evaluation of the position by assessing material, piece placement, pawn structures, king safety, and piece mobility
int evaluate(Game game)
{
	int score = 0, square = 0;
	
	for (int piece = P; piece <= k; piece++)
	{
		Bitboard bb = game.bitboards[piece];
		while (bb)
		{
			square = getLeastSignificantBitIndex(bb);

			// Add material placement score
			score += MATERIAL[piece];

			// Update positional scores
			if (piece == P)
			{
				// Add piece placement score
				score += PAWN_SCORE[MIRROR[square]];
				// Punish doubled pawns
				score -= DOUBLE_PAWN_PENALTY * countBits(game.bitboards[P] & FILE_MASKS[square]);
				// Punish isolated pawns
				score -= ISOLATED_PAWN_PENALTY * (((game.bitboards[P] & ISOLATED_MASKS[square]) == 0) ? 1 : 0);
				// Give bonus score to pass pawns
				score += PASS_PAWN_BONUS[square/8] * (((WHITE_PASS_MASKS[square] & game.bitboards[p]) == 0) ? 1 : 0);
			}
			else if (piece == N)
			{
				// Add piece placement score
				score += KNIGHT_SCORE[MIRROR[square]];
				// Give bonus score to mobilized knights
				score += getMobilityScore(N, countBits(KNIGHT_ATTACKS[square]));
			}
			else if (piece == B)
			{
				// Add piece placement score				
				score += BISHOP_SCORE[MIRROR[square]];
				// Give bonus score to mobilized bishops
				score += getMobilityScore(B, countBits(generateBishopAttacks(square, game.occupancies[ALL])));
			}
			else if (piece == R)
			{
				// Add piece placement score
				score += ROOK_SCORE[MIRROR[square]];
				// Give bonus score to rooks on (semi) open files			
				score += OPEN_FILE_SCORE * ((((game.bitboards[P] | game.bitboards[p]) & FILE_MASKS[square]) == 0) ? 1 : 0);
				score += SEMI_OPEN_FILE_SCORE * (((game.bitboards[P] & FILE_MASKS[square]) == 0) ? 1 : 0);
				// Give bonus score to mobilized rooks				
				score += getMobilityScore(R, countBits(generateRookAttacks(square, game.occupancies[ALL])));
			}
			else if (piece == K)
			{
				// Add piece placement score
				score += KING_SCORE[MIRROR[square]];
				// Punish the king on (semi) open files			
				score -= OPEN_FILE_SCORE * ((((game.bitboards[P] | game.bitboards[p]) & FILE_MASKS[square]) == 0) ? 1 : 0);
				score -= SEMI_OPEN_FILE_SCORE * (((game.bitboards[P] & FILE_MASKS[square]) == 0) ? 1 : 0);
				// Give bonus score to the king with a pawn shield
				score += PAWN_SHIELD_SCORE * countBits((KING_ATTACKS[square] & game.bitboards[P]) & ~(FILE_D_MASK | FILE_E_MASK));
			}

			// Minus scores for black since black and white have opposite perspectives
			else if (piece == p)
			{
				score -= PAWN_SCORE[square];
				// Punish doubled pawns
				score += DOUBLE_PAWN_PENALTY * countBits(game.bitboards[p] & FILE_MASKS[square]);
				// Punish isolated pawns
				score += ISOLATED_PAWN_PENALTY * (((game.bitboards[p] & ISOLATED_MASKS[square]) == 0) ? 1 : 0);
				// Give bonus score to pass pawns
				score -= PASS_PAWN_BONUS[7 - square/8] * (((BLACK_PASS_MASKS[square] & game.bitboards[P]) == 0) ? 1 : 0);
			}
			else if (piece == n)
			{
				score -= KNIGHT_SCORE[square];
				score -= getMobilityScore(n, countBits(KNIGHT_ATTACKS[square]));
			}
			else if (piece == b)
			{
				score -= BISHOP_SCORE[square];
				score -= getMobilityScore(b, countBits(generateBishopAttacks(square, game.occupancies[ALL])));
			}
			else if (piece == r)
			{
				score -= ROOK_SCORE[square];
				score -= OPEN_FILE_SCORE * ((((game.bitboards[P] | game.bitboards[p]) & FILE_MASKS[square]) == 0) ? 1 : 0);
				score -= SEMI_OPEN_FILE_SCORE * (((game.bitboards[p] & FILE_MASKS[square]) == 0) ? 1 : 0);		
				score -= getMobilityScore(r, countBits(generateRookAttacks(square, game.occupancies[ALL])));
			}
			else if (piece == k)
			{
				score -= KING_SCORE[square];
				score += OPEN_FILE_SCORE * ((((game.bitboards[P] | game.bitboards[p]) & FILE_MASKS[square]) == 0) ? 1 : 0);
				score += SEMI_OPEN_FILE_SCORE * (((game.bitboards[p] & FILE_MASKS[square]) == 0) ? 1 : 0);
				score -= PAWN_SHIELD_SCORE * countBits((KING_ATTACKS[square] & game.bitboards[p]) & ~(FILE_D_MASK | FILE_E_MASK));
			}

			// Remove bit
			bb = popBit(bb, square);
		}
	}
	// Return minus score for black to make sure that every player tries to maximize their scores
	return (game.side == WHITE) ? score : -score;
}

// Give bonus to mobilized pieces and punish undeveloped ones
int getMobilityScore(int piece, int moveCount)
{
	if (piece == N || piece == n)
	{
		return -4 * (4 - moveCount);
	}
	if (piece == B || piece == b)
	{
		return -5 * (5 - moveCount);
	}
	if (piece == R || piece == r)
	{
		return -2 * (6 - moveCount);
	}
	return 0;
}

void generateFileMasks()
{
	for (int square = 0; square < 64; square ++)
	{
		FILE_MASKS[square] = (FILE_A_MASK << (square % 8));
	}
}

void generateRankMasks()
{
	for (int square = 0; square < 64; square ++)
	{
		RANK_MASKS[square] = (RANK_1_MASK << (8 * (square / 8)));
	}
}

void generateIsolatedMasks()
{
	for (int square = 0; square < 64; square ++)
	{
		ISOLATED_MASKS[square] = 0x0;
		if (FILE_MASKS[square] & ~FILE_A_MASK)
		{
			ISOLATED_MASKS[square] |= (FILE_MASKS[square - 1]);
		}
		if (FILE_MASKS[square] & ~FILE_H_MASK)
		{
			ISOLATED_MASKS[square] |= (FILE_MASKS[square + 1]);
		}
	}
}

void generatePassMasks()
{
	for (int square = 0; square < 64; square ++)
	{
		WHITE_PASS_MASKS[square] = FILE_MASKS[square] | ISOLATED_MASKS[square];
		BLACK_PASS_MASKS[square] = FILE_MASKS[square] | ISOLATED_MASKS[square];
		for (int rank = 0; rank < 8; rank ++)
		{
			if (square - 8 * rank >= 0)
			{
				WHITE_PASS_MASKS[square] &= ~RANK_MASKS[square - 8 * rank];
			}
			if (square + 8 * rank < 64)
			{
				BLACK_PASS_MASKS[square] &= ~RANK_MASKS[square + 8 * rank];
			}
		}
	}
}

void Evaluation::printDebug()
{
	for (int i = 0; i < 64; i ++)
	{
		std::cout << "File mask: " << i << std::endl;
		displayBitboard(FILE_MASKS[i]);
	} 
	for (int i = 0; i < 64; i ++)
	{
		std::cout << "Rank mask: " << i << std::endl;
		displayBitboard(RANK_MASKS[i]);
	} 
	for (int i = 0; i < 64; i ++)
	{
		std::cout << "Isolated mask: " << i << std::endl;
		displayBitboard(ISOLATED_MASKS[i]);
	} 
	for (int i = 0; i < 64; i ++)
	{
		std::cout << "White pass pawn mask: " << i << std::endl;
		displayBitboard(WHITE_PASS_MASKS[i]);
	}
	for (int i = 0; i < 64; i ++)
	{
		std::cout << "Black pass pawn mask: " << i << std::endl;
		displayBitboard(BLACK_PASS_MASKS[i]);
	}	
}