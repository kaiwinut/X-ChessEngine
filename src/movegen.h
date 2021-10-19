#ifndef MOVEGEN_H
#define MOVEGEN_H

#include <iostream>
#include <string>

class Game
{
public:
	int side;
	int enPassantSquare;
	int castlingRights;
	Bitboard bitboards[12];
	Bitboard occupancies[3];
	int moveList[150];
	int moveNum;
	int fiftyMoveRuleCount;
	// for generating trasposition tables
	uint64_t hashKey;
	uint64_t PIECE_KEY[12][64];
	uint64_t ENPASSANT_KEY[64];
	uint64_t CASTLE_KEY[16];
	uint64_t SIDE_KEY;

	Game();
	Game(std::string fen);
	void parseFen(std::string fen);
	std::string generateFen();
	void initPieceBitboards();
	void initHashKey();
	uint64_t generateHashKey();
	void generateAllMoves();
	void printMoveList();
	int getCaptures(int target, int attacker);
	int isSquareAttacked(int square, int attacker);
	GameState makeMove(int move, int moveType = ALL_MOVES);
	GameState makeNullMove();
	void takeBack(GameState prevState);
	void displayGame();
};

// Move decoding
static inline int getStartSquare (int move) { return move & 0x3f; }
static inline int getEndSquare (int move) { return (move & 0xfc0) >> 6; }
static inline int getPiece (int move) { return (move & 0xf000) >> 12; }
static inline int getPromotion (int move) { return (move & 0xf0000) >> 16; }
static inline int getCapturedPiece (int move) { return (move & 0xf00000) >> 20; }
static inline int getCaptureFlag (int move) { return (move & 0x1000000) >> 24; }
static inline int getDoublePushFlag (int move) { return (move & 0x2000000) >> 25; }
static inline int getEnpassantFlag (int move) { return (move & 0x4000000) >> 26; }
static inline int getCastlingFlag (int move) { return (move & 0x8000000) >> 27; }

#endif