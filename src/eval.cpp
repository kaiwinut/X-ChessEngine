#include "utils.h"
#include "masks.h"
#include "movegen.h"
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
		// printDebug();
	}
}

// Give a static evaluation of the position by assessing material, piece placement, pawn structures, king safety, and piece mobility
int evaluate(Game game)
{
	int score;
	int gamePhase = -1;
	int openingScore = 0;
	int endgameScore = 0;

	if (game.gamePhaseScore > OPENING_PHASE_SCORE) { gamePhase = OPENING; }
	else if (game.gamePhaseScore < ENDGAME_PHASE_SCORE) { gamePhase = ENDGAME; }
	else { gamePhase = MIDDLEGAME; }

	int square;

	for (int piece = P; piece <= k; piece++)
	{
		Bitboard bb = game.bitboards[piece];
		while (bb)
		{
			square = getLeastSignificantBitIndex(bb);

			// Add material placement score
			openingScore += MATERIAL[OPENING][piece];
			endgameScore += MATERIAL[ENDGAME][piece];

			switch (piece)
			{
				case P:
					openingScore += POSITIONAL_SCORE[OPENING][PAWN][MIRROR[square]];
					endgameScore += POSITIONAL_SCORE[ENDGAME][PAWN][MIRROR[square]];

					// Punish doubled pawns
					endgameScore -= DOUBLE_PAWN_PENALTY * countBits(game.bitboards[P] & FILE_MASKS[square]);
					// Punish isolated pawns
					endgameScore -= ISOLATED_PAWN_PENALTY * (((game.bitboards[P] & ISOLATED_MASKS[square]) == 0) ? 1 : 0);
					// Give bonus score to pass pawns
					endgameScore += POSITIONAL_SCORE[ENDGAME][PAWN][MIRROR[square]] / 2 * (((WHITE_PASS_MASKS[square] & game.bitboards[p]) == 0) ? 1 : 0);

					break;

				case p:
					openingScore -= POSITIONAL_SCORE[OPENING][PAWN][square];
					endgameScore -= POSITIONAL_SCORE[ENDGAME][PAWN][square];

					// Punish doubled pawns
					endgameScore += DOUBLE_PAWN_PENALTY * countBits(game.bitboards[p] & FILE_MASKS[square]);
					// Punish isolated pawns
					endgameScore += ISOLATED_PAWN_PENALTY * (((game.bitboards[p] & ISOLATED_MASKS[square]) == 0) ? 1 : 0);
					// Give bonus score to pass pawns
					endgameScore -= POSITIONAL_SCORE[ENDGAME][PAWN][square] / 2 * (((BLACK_PASS_MASKS[square] & game.bitboards[P]) == 0) ? 1 : 0);

					break;

				case N:
					openingScore += POSITIONAL_SCORE[OPENING][KNIGHT][MIRROR[square]];
					openingScore += getKnightMobilityScore(countBits(KNIGHT_ATTACKS[square]));
					endgameScore += POSITIONAL_SCORE[ENDGAME][KNIGHT][MIRROR[square]];
					endgameScore += getKnightMobilityScore(countBits(KNIGHT_ATTACKS[square]));
					break;

				case n:
					openingScore -= POSITIONAL_SCORE[OPENING][KNIGHT][square];
					openingScore -= getKnightMobilityScore(countBits(KNIGHT_ATTACKS[square]));
					endgameScore -= POSITIONAL_SCORE[ENDGAME][KNIGHT][square];
					endgameScore -= getKnightMobilityScore(countBits(KNIGHT_ATTACKS[square]));
					break;

				case B:
					openingScore += POSITIONAL_SCORE[OPENING][BISHOP][MIRROR[square]];
					openingScore += getBishopMobilityScore(countBits(generateBishopAttacks(square, game.bitboards[ALL])));
					endgameScore += POSITIONAL_SCORE[ENDGAME][BISHOP][MIRROR[square]];
					endgameScore += getBishopMobilityScore(countBits(generateBishopAttacks(square, game.bitboards[ALL])));
					break;

				case b:
					openingScore -= POSITIONAL_SCORE[OPENING][BISHOP][square];
					openingScore -= getBishopMobilityScore(countBits(generateBishopAttacks(square, game.bitboards[ALL])));
					endgameScore -= POSITIONAL_SCORE[ENDGAME][BISHOP][square];
					endgameScore -= getBishopMobilityScore(countBits(generateBishopAttacks(square, game.bitboards[ALL])));
					break;

				case R:
					openingScore += POSITIONAL_SCORE[OPENING][ROOK][MIRROR[square]];
					openingScore += getRookMobilityScore(OPENING, countBits(generateRookAttacks(square, game.bitboards[ALL])));
					endgameScore += POSITIONAL_SCORE[ENDGAME][ROOK][MIRROR[square]];
					endgameScore += getRookMobilityScore(ENDGAME, countBits(generateRookAttacks(square, game.bitboards[ALL])));

					// Give bonus score to rooks on (semi) open files			
					openingScore += OPEN_FILE_SCORE * ((((game.bitboards[P] | game.bitboards[p]) & FILE_MASKS[square]) == 0) ? 1 : 0);
					openingScore += SEMI_OPEN_FILE_SCORE * (((game.bitboards[P] & FILE_MASKS[square]) == 0) ? 1 : 0);

					break;

				case r:
					openingScore -= POSITIONAL_SCORE[OPENING][ROOK][square];
					openingScore -= getRookMobilityScore(OPENING, countBits(generateRookAttacks(square, game.bitboards[ALL])));
					endgameScore -= POSITIONAL_SCORE[ENDGAME][ROOK][square];
					endgameScore -= getRookMobilityScore(ENDGAME, countBits(generateRookAttacks(square, game.bitboards[ALL])));

					// Give bonus score to rooks on (semi) open files			
					openingScore -= OPEN_FILE_SCORE * ((((game.bitboards[P] | game.bitboards[p]) & FILE_MASKS[square]) == 0) ? 1 : 0);
					openingScore -= SEMI_OPEN_FILE_SCORE * (((game.bitboards[P] & FILE_MASKS[square]) == 0) ? 1 : 0);

					break;

				case Q:
					openingScore += POSITIONAL_SCORE[OPENING][QUEEN][MIRROR[square]];
					endgameScore += POSITIONAL_SCORE[ENDGAME][QUEEN][MIRROR[square]];
					break;

				case q:
					openingScore -= POSITIONAL_SCORE[OPENING][QUEEN][square];
					endgameScore -= POSITIONAL_SCORE[ENDGAME][QUEEN][square];
					break;	

				case K:
					openingScore += POSITIONAL_SCORE[OPENING][KING][MIRROR[square]];
					endgameScore += POSITIONAL_SCORE[ENDGAME][KING][MIRROR[square]];

					// Punish the king on (semi) open files			
					openingScore -= OPEN_FILE_SCORE * ((((game.bitboards[P] | game.bitboards[p]) & FILE_MASKS[square]) == 0) ? 1 : 0);
					openingScore -= SEMI_OPEN_FILE_SCORE * (((game.bitboards[P] & FILE_MASKS[square]) == 0) ? 1 : 0);
					// Give bonus score to the king with a pawn shield
					openingScore += PAWN_SHIELD_SCORE * countBits((KING_ATTACKS[square] & game.bitboards[P]) & ~(FILE_D_MASK | FILE_E_MASK));

					break;

				case k:
					openingScore -= POSITIONAL_SCORE[OPENING][KING][square];
					endgameScore -= POSITIONAL_SCORE[ENDGAME][KING][square];

					// Punish the king on (semi) open files			
					openingScore += OPEN_FILE_SCORE * ((((game.bitboards[P] | game.bitboards[p]) & FILE_MASKS[square]) == 0) ? 1 : 0);
					openingScore += SEMI_OPEN_FILE_SCORE * (((game.bitboards[p] & FILE_MASKS[square]) == 0) ? 1 : 0);
					// Give bonus score to the king with a pawn shield
					openingScore -= PAWN_SHIELD_SCORE * countBits((KING_ATTACKS[square] & game.bitboards[p]) & ~(FILE_D_MASK | FILE_E_MASK));

					break;	

				default:
					if (DEBUG_EVAL) { std::cout << "Eval Err! Piece: " << piece << std::endl; }
					return 0;							
			}
			// Remove bit
			bb = popBit(bb, square);
		}
	}

	if (gamePhase == OPENING)
	{
		score = openingScore;
	}
	else if (gamePhase == ENDGAME)
	{
		score = endgameScore;
	}
	// Interpolated score
	else
	{
		score = (openingScore * game.gamePhaseScore + endgameScore * (OPENING_PHASE_SCORE - game.gamePhaseScore)) / OPENING_PHASE_SCORE;
	}

	// // Return minus score for black to make sure that every player tries to maximize their scores
	return (game.side == WHITE) ? score : -score;
}

int getGamePhaseScore(Game game)
{
	int pieceScore = 0;
	pieceScore += countBits(game.bitboards[N]) * MATERIAL_ABS[OPENING][N];
	pieceScore += countBits(game.bitboards[B]) * MATERIAL_ABS[OPENING][B];
	pieceScore += countBits(game.bitboards[R]) * MATERIAL_ABS[OPENING][R];
	pieceScore += countBits(game.bitboards[Q]) * MATERIAL_ABS[OPENING][Q];
	pieceScore += countBits(game.bitboards[n]) * MATERIAL_ABS[OPENING][n];
	pieceScore += countBits(game.bitboards[b]) * MATERIAL_ABS[OPENING][b];
	pieceScore += countBits(game.bitboards[r]) * MATERIAL_ABS[OPENING][r];
	pieceScore += countBits(game.bitboards[q]) * MATERIAL_ABS[OPENING][q];
	return pieceScore;
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