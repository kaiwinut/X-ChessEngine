#ifndef EVAL_H
#define EVAL_H

#include "types.h"
#include "movegen.h"

// Material count
const int MATERIAL[12] = {100, 300, 350, 500, 1000, 10000, -100, -300, -350, -500, -1000, -10000};

// Positional Scores
const int PAWN_SCORE[64] = {
    90,  90,  90,  90,  90,  90,  90,  90,
    30,  30,  30,  40,  40,  30,  30,  30,
    20,  20,  20,  30,  30,  30,  20,  20,
    10,  10,  10,  20,  20,  10,  10,  10,
     5,   5,  10,  20,  20,   5,   5,   5,
     0,   0,   0,   5,   5,   0,   0,   0,
     0,   0,   0, -10, -10,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0
};

const int KNIGHT_SCORE[64] = {
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,  10,  10,   0,   0,  -5,
    -5,   5,  20,  20,  20,  20,   5,  -5,
    -5,  10,  20,  30,  30,  20,  10,  -5,
    -5,  10,  20,  30,  30,  20,  10,  -5,
    -5,   5,  20,  10,  10,  20,   5,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5, -10,   0,   0,   0,   0, -10,  -5
};

const int BISHOP_SCORE[64] = {
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,  10,  10,   0,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,  10,   0,   0,   0,   0,  10,   0,
     0,  30,   0,   0,   0,   0,  30,   0,
     0,   0, -10,   0,   0, -10,   0,   0
};

const int ROOK_SCORE[64] = {
    50,  50,  50,  50,  50,  50,  50,  50,
    50,  50,  50,  50,  50,  50,  50,  50,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,   0,  20,  20,   0,   0,   0
};

const int KING_SCORE[64] = {
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   5,   5,   5,   5,   0,   0,
     0,   5,   5,  10,  10,   5,   5,   0,
     0,   5,  10,  20,  20,  10,   5,   0,
     0,   5,  10,  20,  20,  10,   5,   0,
     0,   0,   5,  10,  10,   5,   0,   0,
     0,   5,   5,  -5,  -5,   0,   5,   0,
     0,   0,   5,   0, -15,   0,  10,   0
};

const int MIRROR[64] = {
     A8,  B8,  C8,  D8,  E8,  F8,  G8,  H8,
     A7,  B7,  C7,  D7,  E7,  F7,  G7,  H7,
     A6,  B6,  C6,  D6,  E6,  F6,  G6,  H6,
     A5,  B5,  C5,  D5,  E5,  F5,  G5,  H5,
     A4,  B4,  C4,  D4,  E4,  F4,  G4,  H4,
     A3,  B3,  C3,  D3,  E3,  F3,  G3,  H3,
     A2,  B2,  C2,  D2,  E2,  F2,  G2,  H2,
     A1,  B1,  C1,  D1,  E1,  F1,  G1,  H1
};

extern Bitboard FILE_MASKS[64];
extern Bitboard RANK_MASKS[64];
extern Bitboard ISOLATED_MASKS[64];
extern Bitboard WHITE_PASS_MASKS[64];
extern Bitboard BLACK_PASS_MASKS[64];
void generateFileMasks();
void generateRankMasks();
void generateIsolatedMasks();
void generateWhitePassMasks();
void generateBlackPassMasks();

// Index is rank - 1 (from the perspective of white pawns)
const int PASS_PAWN_BONUS[8] = {0, 5, 10, 20, 35, 60, 100, 200};
const int DOUBLE_PAWN_PENALTY = -10;
const int ISOLATED_PAWN_PENALTY = -10;
const int OPEN_FILE_SCORE = 15;
const int SEMI_OPEN_FILE_SCORE = 10;
const int PAWN_SHIELD_SCORE = 10;

namespace Evaluation
{
	void init();
}

int evaluate(Game game);

#endif