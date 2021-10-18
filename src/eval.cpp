#include "types.h"
#include "bitboard.h"
#include "movegen.h"
#include "eval.h"

Bitboard FILE_MASKS[64];
Bitboard RANK_MASKS[64];
Bitboard ISOLATED_MASKS[64];
Bitboard WHITE_PASS_MASKS[64];
Bitboard BLACK_PASS_MASKS[64];

void Evaluation::init()
{
	generateFileMasks();
	generateRankMasks();
	generateIsolatedMasks();
	generateWhitePassMasks();
	generateBlackPassMasks();
}

void generateFileMasks()
{

}

void generateRankMasks()
{

}

void generateIsolatedMasks()
{

}

void generateWhitePassMasks()
{

}

void generateBlackPassMasks()
{

}

int evaluate(Game game)
{
	int score = 0, square = 0;
	for (int piece = P; piece <= k; piece++)
	{
		Bitboard bb = game.bitboards[piece];
		while (bb)
		{
			square = getLeastSignificantBitIndex(bb);

			// Update material scores
			score += MATERIAL[piece];

			// Update positional scores
			if (piece == P)
			{
				score += PAWN_SCORE[MIRROR[square]];
			}
			else if (piece == N)
			{
				score += KNIGHT_SCORE[MIRROR[square]];
			}
			else if (piece == B)
			{
				score += BISHOP_SCORE[MIRROR[square]];
			}
			else if (piece == R)
			{
				score += ROOK_SCORE[MIRROR[square]];
			}
			else if (piece == K)
			{
				score += KING_SCORE[MIRROR[square]];
			}

			// Minus scores for black since black and white have opposite perspectives
			else if (piece == p)
			{
				score -= PAWN_SCORE[square];
			}
			else if (piece == n)
			{
				score -= KNIGHT_SCORE[square];
			}
			else if (piece == b)
			{
				score -= BISHOP_SCORE[square];
			}
			else if (piece == r)
			{
				score -= ROOK_SCORE[square];
			}
			else if (piece == k)
			{
				score -= KING_SCORE[square];
			}

			// Remove bit
			bb = popBit(bb, square);
		}
	}
	return (game.side == WHITE) ? score : -score;
}