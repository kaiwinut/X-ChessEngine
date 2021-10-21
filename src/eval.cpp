#include "bitboard.h"
#include "types.h"
#include "movegen.h"
#include "eval.h"

// Give a static evaluation of the position by assessing material, piece placement, pawn structures, king safety, and piece mobility
int evaluate(Game game)
{
	int score;
	int gamePhaseScore = getGamePhaseScore(game);
	int gamePhase = -1;
	int openingScore = 0;
	int endgameScore = 0;

	if (gamePhaseScore > OPENING_PHASE_SCORE) { gamePhase = OPENING; }
	else if (gamePhaseScore < ENDGAME_PHASE_SCORE) { gamePhase = ENDGAME; }
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
					break;
				case p:
					openingScore -= POSITIONAL_SCORE[OPENING][PAWN][square];
					endgameScore -= POSITIONAL_SCORE[ENDGAME][PAWN][square];
					break;

				case N:
					openingScore += POSITIONAL_SCORE[OPENING][KNIGHT][MIRROR[square]];
					endgameScore += POSITIONAL_SCORE[ENDGAME][KNIGHT][MIRROR[square]];
					break;
				case n:
					openingScore -= POSITIONAL_SCORE[OPENING][KNIGHT][square];
					endgameScore -= POSITIONAL_SCORE[ENDGAME][KNIGHT][square];
					break;

				case B:
					openingScore += POSITIONAL_SCORE[OPENING][BISHOP][MIRROR[square]];
					endgameScore += POSITIONAL_SCORE[ENDGAME][BISHOP][MIRROR[square]];
					break;
				case b:
					openingScore -= POSITIONAL_SCORE[OPENING][BISHOP][square];
					endgameScore -= POSITIONAL_SCORE[ENDGAME][BISHOP][square];
					break;

				case R:
					openingScore += POSITIONAL_SCORE[OPENING][ROOK][MIRROR[square]];
					endgameScore += POSITIONAL_SCORE[ENDGAME][ROOK][MIRROR[square]];
					break;
				case r:
					openingScore -= POSITIONAL_SCORE[OPENING][ROOK][square];
					endgameScore -= POSITIONAL_SCORE[ENDGAME][ROOK][square];
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
					break;
				case k:
					openingScore -= POSITIONAL_SCORE[OPENING][KING][square];
					endgameScore -= POSITIONAL_SCORE[ENDGAME][KING][square];
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
		score = (openingScore * gamePhaseScore + endgameScore * (OPENING_PHASE_SCORE - gamePhaseScore)) / OPENING_PHASE_SCORE;
	}

	// // Return minus score for black to make sure that every player tries to maximize their scores
	return (game.side == WHITE) ? score : -score;
}

int getGamePhaseScore(Game game)
{
	int whitePieceScore = 0, blackPieceScore = 0;
	for (int whitePiece = N; whitePiece <= Q; whitePiece ++)
	{
		whitePieceScore += countBits(game.bitboards[whitePiece]) * MATERIAL[OPENING][whitePiece];
	}
	for (int blackPiece = n; blackPiece <= q; blackPiece ++)
	{
		blackPieceScore += countBits(game.bitboards[blackPiece]) * (-MATERIAL[OPENING][blackPiece]);
	}
	return whitePieceScore + blackPieceScore;
}