#ifndef UTILS_H
#define UTILS_H

#include <iostream>

/*********************************************************
		
						Debug flags 

**********************************************************/

const bool DEBUG_EVAL = 0;
const bool DEBUG_MASK = 0;
const bool DEBUG_PERFT = 0;
const bool DEBUG_GAME = 0;
const bool DEBUG_ENGINE = 0;
const bool DEBUG_UCI = 0;


/*********************************************************
		
				   Custom type / struct 

**********************************************************/

typedef uint64_t Bitboard;

// Frequently used enumerators
enum Square {
	A1, B1, C1, D1, E1, F1, G1, H1,
	A2, B2, C2, D2, E2, F2, G2, H2,
	A3, B3, C3, D3, E3, F3, G3, H3,
	A4, B4, C4, D4, E4, F4, G4, H4,
	A5, B5, C5, D5, E5, F5, G5, H5,
	A6, B6, C6, D6, E6, F6, G6, H6,
	A7, B7, C7, D7, E7, F7, G7, H7,
	A8, B8, C8, D8, E8, F8, G8, H8,
	SQ_NONE,
};

enum Direction {
	NORTH = 8,
	EAST = 1,
	SOUTH = - NORTH,
	WEST = - EAST,

	NORTH_EAST = NORTH + EAST,
	NORTH_WEST = NORTH + WEST,
	SOUTH_EAST = SOUTH + EAST,
	SOUTH_WEST = SOUTH + WEST
};

enum File {
	FILE_A, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H
};

enum Rank {
	RANK_1, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8
};

enum Color {
	BLACK, WHITE, ALL
};

enum Piece {
	P, N, B, R, Q, K, p, n, b, r, q, k, NULL_PIECE
};

// piece types
enum PieceType { 
	PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING 
};

enum CastlingRights {
	WK = 1,
	WQ = 2,
	BK = 4,
	BQ = 8
};

enum MoveType {
	ALL_MOVES, ONLY_CAPTURES
};

// game phases
enum GamePhase { 
	OPENING, ENDGAME, MIDDLEGAME 
};

// GameState structure used to store previous game state
struct GameState
{
	int valid;
	uint64_t bitboards[12];
	uint64_t occupancies[3];
	int moveList[150];
	int side;
	int enPassantSquare;
	int castlingRights;
	int moveNum;
	int fiftyMoveRuleCount;	
	uint64_t hashKey;
	int gamePhaseScore;
	uint64_t checkMask;
};

class Game
{
public:
	// Current game state
	int side = WHITE;
	int enPassantSquare = SQ_NONE;
	int castlingRights = 0b1111;
	uint64_t bitboards[12] = {0ull};
	uint64_t occupancies[3] = {0ull};
	int moveList[150];
	int moveNum = 0;
	int fiftyMoveRuleCount = 0;
	uint64_t checkMask = 0ull;

	// for generating trasposition tables
	uint64_t hashKey = 0ull;
	uint64_t PIECE_KEY[12][64];
	uint64_t ENPASSANT_KEY[64];
	uint64_t CASTLE_KEY[16];
	uint64_t SIDE_KEY;

	// Get value right after make move
	int gamePhaseScore = 6766;

	// Construct instance with fen or start position
	Game();
	Game(std::string fen);
	void parseFen(std::string fen);
	// std::string generateFen();

	// Initialize bitboards
	void initPieceBitboards();
	void initHashKey();
	uint64_t generateHashKey();

	// Move generation
	void generateAllMoves();
	int getCaptures(int target, int attacker);
	uint64_t isSquareAttacked(int square, int attacker);
	uint64_t getCheckMask(int attacker);

	GameState makeMove(int move, int moveType = ALL_MOVES);
	GameState makeNullMove();
	void takeBack(GameState prevState);
};


/*********************************************************
		
					Bitboard operations 

**********************************************************/

// Check if a bitboard contains a certain bit (square)
static inline uint64_t getBit(uint64_t board, int square) { return board & (1ull << square); }
// Set a bit at a given index in the bitboard
static inline uint64_t setBit(uint64_t board, int square) { return board | (1ull << square); }
// Remove a bit at a given index in the bitboard
static inline uint64_t popBit(uint64_t board, int square) { return board & ~(1ull << square); }
// Count bits that are 1 in a bitboard
static inline int countBits(uint64_t board)
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
// Retrieve the index of the least significant bit in a bitboard
static inline int getLeastSignificantBitIndex(uint64_t board) { return (board ? countBits((board & -board) - 1) : -1); }


/*********************************************************
		
				        Evaluation 

**********************************************************/

// Give bonus to mobilized pieces and punish undeveloped ones
static inline int getKnightMobilityScore(int moveCount) { return -4 * (4 - moveCount); }
static inline int getBishopMobilityScore(int moveCount) { return -5 * (5 - moveCount); }
static inline int getRookMobilityScore(int moveCount, int gamePhase)
{
    return ((gamePhase == OPENING) ? (-2 * (6 - moveCount)) : (-6 * (6 - moveCount))); 
}


/*********************************************************
		
				      Move generation 

**********************************************************/

// Move encoding
static inline int encodeMove(int start, int end, int piece, int promotion, int capturedPiece, int capture, int doublePush, int enPassant, int castling)
{
	return start | (end << 6) | (piece << 12) | (promotion << 16) | (capturedPiece << 20) | (capture << 24) | (doublePush << 25) | (enPassant << 26) | (castling << 27);
}

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


/*********************************************************
		
				    	   Perft 

**********************************************************/

// FEN strings and constants for the perft
const std::string EMPTY_BOARD = "8/8/8/8/8/8/8/8 w - - 0 0";
const std::string START_POSITION = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const std::string TRICKY_POSITION = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
const std::string CMK_POSITION = "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9";

// Perft result look up
const uint64_t START_POSITION_PERFT[7] = {20, 400, 8902, 197281, 4865609, 119060324, 3195901860};
const uint64_t TRICY_POSITION_PERFT[6] = {48, 2039, 97862, 4085603, 193690690, 8031647685};


/*********************************************************
		
				    	   Print 

**********************************************************/

// Display bitboard in 8 x 8 style
void displayBitboard(uint64_t board);
// Display bitboard in 8 x 8 style
void displayGame(Game game);
// Display move list
void printMoveList(Game game);

const std::string ASCII_PIECES[13] = {"P","N","B","R","Q","K","p","n","b","r","q","k","-"};

const std::string UNICODE_PIECES[13] = {"♟","♞","♝","♜","♛","♚","♙","♘","♗","♖","♕","♔","-"};

const std::string SQUARES[64] = {"a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1", 
							   	  "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2", 
							   	  "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3", 
							   	  "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4", 
							   	  "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5", 
							   	  "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6", 
							   	  "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7", 
							   	  "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8"};

#endif