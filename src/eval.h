#ifndef EVAL_H
#define EVAL_H

#include "types.h"
#include "movegen.h"

namespace Evaluation
{
    void init();
    void printDebug();
}

// Give a static evaluation of the position by assessing material, piece placement, pawn structures, king safety, and piece mobility
int evaluate(Game game);

// Material count
const int MATERIAL[12] = {100, 300, 350, 500, 1000, 10000, -100, -300, -350, -500, -1000, -10000};
// const int MATERIAL_END[12] = {100, 300, 350, 500, 1000, 10000, -100, -300, -350, -500, -1000, -10000};

// Positional score of pawns
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

// const int PAWN_SCORE_END[64] = {
//     90,  90,  90,  90,  90,  90,  90,  90,
//     30,  30,  30,  40,  40,  30,  30,  30,
//     20,  20,  20,  30,  30,  30,  20,  20,
//     10,  10,  10,  20,  20,  10,  10,  10,
//      5,   5,  10,  20,  20,   5,   5,   5,
//      0,   0,   0,   5,   5,   0,   0,   0,
//      0,   0,   0, -10, -10,   0,   0,   0,
//      0,   0,   0,   0,   0,   0,   0,   0
// };

// Positional score of knights
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

// const int KNIGHT_SCORE_END[64] = {
//     -5,   0,   0,   0,   0,   0,   0,  -5,
//     -5,   0,   0,  10,  10,   0,   0,  -5,
//     -5,   5,  20,  20,  20,  20,   5,  -5,
//     -5,  10,  20,  30,  30,  20,  10,  -5,
//     -5,  10,  20,  30,  30,  20,  10,  -5,
//     -5,   5,  20,  10,  10,  20,   5,  -5,
//     -5,   0,   0,   0,   0,   0,   0,  -5,
//     -5, -10,   0,   0,   0,   0, -10,  -5
// };

// Positional score of bishops
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

// const int BISHOP_SCORE_END[64] = {
//      0,   0,   0,   0,   0,   0,   0,   0,
//      0,   0,   0,   0,   0,   0,   0,   0,
//      0,   0,   0,  10,  10,   0,   0,   0,
//      0,   0,  10,  20,  20,  10,   0,   0,
//      0,   0,  10,  20,  20,  10,   0,   0,
//      0,  10,   0,   0,   0,   0,  10,   0,
//      0,  30,   0,   0,   0,   0,  30,   0,
//      0,   0, -10,   0,   0, -10,   0,   0
// };

// Positional score of rooks
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

// const int ROOK_SCORE_END[64] = {
//     50,  50,  50,  50,  50,  50,  50,  50,
//     50,  50,  50,  50,  50,  50,  50,  50,
//      0,   0,  10,  20,  20,  10,   0,   0,
//      0,   0,  10,  20,  20,  10,   0,   0,
//      0,   0,  10,  20,  20,  10,   0,   0,
//      0,   0,  10,  20,  20,  10,   0,   0,
//      0,   0,  10,  20,  20,  10,   0,   0,
//      0,   0,   0,  20,  20,   0,   0,   0
// };

// Positional score of the king
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

// const int KING_SCORE_END[64] = {
//      0,   0,   0,   0,   0,   0,   0,   0,
//      0,   0,   5,   5,   5,   5,   0,   0,
//      0,   5,   5,  10,  10,   5,   5,   0,
//      0,   5,  10,  20,  20,  10,   5,   0,
//      0,   5,  10,  20,  20,  10,   5,   0,
//      0,   0,   5,  10,  10,   5,   0,   0,
//      0,   5,   5,  -5,  -5,   0,   5,   0,
//      0,   0,   5,   0, -15,   0,  10,   0
// };

// Mirror the squares virtically 
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

// Masks that help determine pawn structures, king safety and piece mobility
extern Bitboard FILE_MASKS[64];
extern Bitboard RANK_MASKS[64];
extern Bitboard ISOLATED_MASKS[64];
extern Bitboard WHITE_PASS_MASKS[64];
extern Bitboard BLACK_PASS_MASKS[64];

// Functions generating masks that help determine pawn structures, king safety and piece mobility
void generateFileMasks();
void generateRankMasks();
void generateIsolatedMasks();
void generatePassMasks();

// Give bonus to mobilized pieces and punish undeveloped ones
int getMobilityScore(int piece, int moveCount);

// Bonus score for pass pawns on each rank (from the perspective of white pawns)
const int PASS_PAWN_BONUS[8] = {0, 5, 10, 20, 35, 60, 100, 200};

// Penalty score for doubled pawns
const int DOUBLE_PAWN_PENALTY = 10;

// Penalty score for isolated pawns
const int ISOLATED_PAWN_PENALTY = 10;

// Bonus score for rooks on (semi) open files, and penalty score for kings on them
const int OPEN_FILE_SCORE = 15;
const int SEMI_OPEN_FILE_SCORE = 10;

// Bonus score for kings that have a pawn shield before them
const int PAWN_SHIELD_SCORE = 10;

#endif