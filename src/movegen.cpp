#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include "bitboard.h"
#include "types.h"
#include "masks.h"
#include "eval.h"
#include "movegen.h"

using std::cout;
using std::endl;
using std::string;
using std::map;

// Constructor of the class Game
Game::Game()
{
	// Initialize new game state
	side = WHITE;
	enPassantSquare = SQ_NONE;
	castlingRights = 0b1111;
	moveNum = 0;
	fiftyMoveRuleCount = 0;	
	hashKey = 0x0;

	for (int piece = P; piece <= k; piece ++)
	{
		bitboards[piece] = 0ULL;
	}

	for (int color = BLACK; color <= ALL; color ++)
	{
		occupancies[color] = 0ULL;
	}

	// Initialize piece bitboards and occupancy bitboard
	initPieceBitboards();

	// Initialize hash keys
	initHashKey();
	hashKey = generateHashKey();
}

// Alternative constructor when a fen string is passed
Game::Game(string fen)
{
	parseFen(fen);
}

// Set game state with fen
void Game::parseFen(string fen)
{
	// Refer to defined enumeraters from characters in the fen string
	map <char, int> PIECES = {{'P', P}, {'N', N}, {'B', B}, {'R', R}, {'Q', Q}, {'K', K}, 
							  {'p', p}, {'n', n}, {'b', b}, {'r', r}, {'q', q}, {'k', k}};

	// Used to parse the en passant square
	map <char, int> FILE_TO_INT = {{'a', FILE_A}, {'b', FILE_B}, {'c', FILE_C}, {'d', FILE_D}, 
								   {'e', FILE_E}, {'f', FILE_F}, {'g', FILE_G}, {'h', FILE_H}};

	// Reset every game state first
	side = WHITE;
	enPassantSquare = SQ_NONE;
	castlingRights = 0x0;
	moveNum = 0;
	fiftyMoveRuleCount = 0;	

	for (int piece = P; piece <= k; piece ++)
	{
		bitboards[piece] = 0ULL;
	}

	for (int color = BLACK; color <= ALL; color ++)
	{
		occupancies[color] = 0ULL;
	}

	// Initialize bitboards (count from index 56 since fen strings start from the 8th rank)
	int count = 56;
	int i;
	for (i = 0; i < fen.length(); i++)
	{
		// The first space signifies the end of information on piece positions
		if (fen[i] == ' ') { break; }
		// Subtarct 16 from the count to skip from the end of the current rank to the beginning of the next rank
		else if (fen[i] == '/')
		{
			count -= 16;
		}
		// If the character is an alphabet, update the bitboard of the corresponding piece and the occupancy bitboard
		else if (isalpha(fen[i]))
		{
			bitboards[PIECES[fen[i]]] = setBit(bitboards[PIECES[fen[i]]], count);
			if (isupper(fen[i])) { occupancies[WHITE] = setBit(occupancies[WHITE], count); }
			else { occupancies[BLACK] = setBit(occupancies[BLACK], count); }
			count ++;
		}
		// If the character is a number, the square is empty. Simply add the number to the count and go to the next character
		else if (isdigit(fen[i]))
		{
			count += atoi(&fen[i]);
		}
		else
		{
			cout << "Error occured when parsing fen!" << endl;
		}
	}
	// Update the occupancy board
	occupancies[ALL] = occupancies[WHITE] | occupancies[BLACK];

	// Initialize side
	i ++;
	side = (fen[i] == 'w' ? WHITE : BLACK);
	// Initialize castling rights
	i += 2;
	while (fen[i] != ' ')
	{
		if (fen[i] == '-')
		{
			castlingRights = 0;
			i ++;
		}
		else if (isalpha(fen[i]))
		{
			if (fen[i] == 'K') { castlingRights += 1; }
			if (fen[i] == 'Q') { castlingRights += 2; }
			if (fen[i] == 'k') { castlingRights += 4; }
			if (fen[i] == 'q') { castlingRights += 8; }
			i++;
		}
	}
	// Initialize en passant square
	i ++;
	if (fen[i] == '-') { i++; }
	else if (isalpha(fen[i]))
	{
		enPassantSquare = FILE_TO_INT[fen[i]] + 8 * (atoi(&fen[i + 1]) - 1);
		i += 2;
	}
	// Initialize fifty move rule count
	if (i + 1 < fen.length())
	{
		i ++;
		fiftyMoveRuleCount = atoi(&fen[i]);
		i ++;
	}
	// Initialize move number
	if (i + 1 < fen.length())
	{
		i ++;
		moveNum = atoi(&fen[i]);
	}

	// Initialize hash keys and generate a hash key of the current position
	initHashKey();
	hashKey = generateHashKey();
}

// Generate fen representation of the current game (Not yet done!)
string Game::generateFen()
{
	return "Yet to be done!";
}

// Initilize bitboards for 12 pieces and 3 occupancy maps
void Game::initPieceBitboards()
{
	// Initilize bitboards for 12 pieces
	bitboards[P] |= RANK_2_MASK;
	bitboards[N] = setBit(bitboards[N], B1);
	bitboards[N] = setBit(bitboards[N], G1); 
	bitboards[B] = setBit(bitboards[B], C1);
	bitboards[B] = setBit(bitboards[B], F1);
	bitboards[R] = setBit(bitboards[R], A1);
	bitboards[R] = setBit(bitboards[R], H1);
	bitboards[Q] = setBit(bitboards[Q], D1);
	bitboards[K] = setBit(bitboards[K], E1);
	bitboards[p] |= RANK_7_MASK;
	bitboards[n] = setBit(bitboards[n], B8);
	bitboards[n] = setBit(bitboards[n], G8); 
	bitboards[b] = setBit(bitboards[b], C8);
	bitboards[b] = setBit(bitboards[b], F8);
	bitboards[r] = setBit(bitboards[r], A8);
	bitboards[r] = setBit(bitboards[r], H8);
	bitboards[q] = setBit(bitboards[q], D8);
	bitboards[k] = setBit(bitboards[k], E8);

	// Initilize bitboards for 3 occupancy maps
	occupancies[WHITE] |= (bitboards[P] | bitboards[N] | bitboards[B] | bitboards[R] | bitboards[Q] | bitboards[K]);
	occupancies[BLACK] |= (bitboards[p] | bitboards[n] | bitboards[b] | bitboards[r] | bitboards[q] | bitboards[k]);
	occupancies[ALL] |= occupancies[WHITE] | occupancies[BLACK];

	if (DEBUG_GAME)
	{
		// Uncomment this to show piece bitboards for debugging purposes
		// for (int piece = P; piece <= k; piece++)
		// {
		// 	displayBitboard(bitboards[piece]);
		// }

		// // Uncomment this to show occupancy bitboards for debugging purposes
		// for (int color = BLACK; color <= ALL; color ++)
		// {
		// 	displayBitboard(occupancies[color]);
		// }
	}
}

void Game::initHashKey()
{
	for (int piece = P; piece <= k; piece ++)
	{
		for (int square = 0; square < 64; square ++)
		{
			PIECE_KEY[piece][square] = generateRandomUint64();
		}
	}
	for (int square = 0; square < 64; square ++)
	{
		ENPASSANT_KEY[square] = generateRandomUint64();
	}
	SIDE_KEY = generateRandomUint64();
	for (int i = 0; i < 16; i ++)
	{
		CASTLE_KEY[i] = generateRandomUint64();
	}
}

uint64_t Game::generateHashKey()
{
	uint64_t key = 0x0;
	int square;
	Bitboard bb;

	for (int piece = P; piece <= k; piece ++)
	{
		bb = bitboards[piece];
		while (bb)
		{
			square = getLeastSignificantBitIndex(bb);
			key ^= PIECE_KEY[piece][square];
			bb = popBit(bb, square);
		}
	}
	if (enPassantSquare != SQ_NONE)
	{
		key ^= ENPASSANT_KEY[enPassantSquare];
	}

	key ^= CASTLE_KEY[castlingRights];

	if (side == BLACK)
	{
		key ^= SIDE_KEY;
	}

	return key;
}

void Game::displayGame()
{
	int index;
	string osName;

	#ifdef __APPLE__
		osName = "Mac OSX";
	#else
		osName = "Others";
	#endif

	cout << "Hash: " << hashKey << std::endl;

	for (int rank = RANK_8; rank >= RANK_1; rank--)
	{
		cout << rank + 1 << "  ";
		for (int file = FILE_A; file <= FILE_H; file++)
		{
			index = rank * 8 + file;
			if (!getBit(occupancies[ALL], index))
			{
				cout <<  ". ";					
			}
			else
			{
				for (int piece = P; piece <= k; piece++)
				{
					if (getBit(bitboards[piece], index))
					{
						cout <<  (osName.compare("Mac OSX") == 0 ? UNICODE_PIECES[piece] : ASCII_PIECES[piece]);
						cout << ' ';
						break;
					}
				}
			}	
		}
		cout << endl;
	}
	cout << endl;
	cout << "   a b c d e f g h\n" << endl;

	cout << "   Side:     " << (side == WHITE ? "White" : "Black") << endl;
	cout << "   Enp.:        " << (enPassantSquare != SQ_NONE ? SQUARES[enPassantSquare] : "--") << endl;

	map <int, string> CASTLERIGHTS = {{0, "----"}, {1, "K---"}, {2, "-Q--"}, {3, "KQ--"}, {4, "--k-"}, {5, "K-k-"}, {6, "-Qk-"}, {7, "KQk-"},
									 {8, "---q"}, {9, "K--q"}, {10, "-Q-q"}, {11, "KQ-q"}, {12, "--kq"}, {13, "K-kq"}, {14, "-Qkq"}, {15, "KQkq"}};

	cout << "   Cast.:     " << CASTLERIGHTS[castlingRights] << endl;
	cout << "   Eval.:      " << evaluate(*this) << std::endl;

	cout << endl;
}

/* 
	Move encoding / decoding
*/
static inline int encodeMove(int start, int end, int piece, int promotion, int capturedPiece, int capture, int doublePush, int enPassant, int castling)
{
	return start | (end << 6) | (piece << 12) | (promotion << 16) | (capturedPiece << 20) | (capture << 24) | (doublePush << 25) | (enPassant << 26) | (castling << 27);
}

void Game::takeBack(GameState prevState)
{
	memcpy(bitboards, prevState.bitboards, sizeof(bitboards));
	memcpy(occupancies, prevState.occupancies, sizeof(occupancies));
	memcpy(moveList, prevState.moveList, sizeof(moveList));
	side = prevState.side;
	enPassantSquare = prevState.enPassantSquare;
	castlingRights = prevState.castlingRights;
	moveNum = prevState.moveNum;
	fiftyMoveRuleCount = prevState.fiftyMoveRuleCount;
	hashKey = prevState.hashKey;
}

GameState Game::makeNullMove()
{
	GameState prevState;
	memcpy(prevState.bitboards, bitboards, sizeof(bitboards));
	memcpy(prevState.occupancies, occupancies, sizeof(occupancies));
	memcpy(prevState.moveList, moveList, sizeof(moveList));
	prevState.side = side;
	prevState.enPassantSquare = enPassantSquare;
	prevState.castlingRights = castlingRights;
	prevState.moveNum = moveNum;
	prevState.fiftyMoveRuleCount = fiftyMoveRuleCount;
	prevState.hashKey = hashKey;

	// Reset the en passant square
	if (enPassantSquare != SQ_NONE)
	{
		hashKey ^= ENPASSANT_KEY[enPassantSquare];
	}
	enPassantSquare = SQ_NONE;	

	// Switch sides
	side = !side;
	hashKey ^= SIDE_KEY;

	return prevState;
}

GameState Game::makeMove(int move, int moveType)
{
	// Decode move
	int start = getStartSquare(move);
	int end = getEndSquare(move);
	int piece = getPiece(move);
	int promotion = getPromotion(move);
	int capturedPiece = getCapturedPiece(move);
	int capture = getCaptureFlag(move);
	int doublePush = getDoublePushFlag(move);
	int enPassant = getEnpassantFlag(move);
	int castling = getCastlingFlag(move);
	// int prevHashKey = hashKey;

	// Store current game state in GameState structure
	GameState prevState;
	memcpy(prevState.bitboards, bitboards, sizeof(bitboards));
	memcpy(prevState.occupancies, occupancies, sizeof(occupancies));
	memcpy(prevState.moveList, moveList, sizeof(moveList));
	prevState.side = side;
	prevState.enPassantSquare = enPassantSquare;
	prevState.castlingRights = castlingRights;
	prevState.moveNum = moveNum;
	prevState.fiftyMoveRuleCount = fiftyMoveRuleCount;
	prevState.hashKey = hashKey;

	// Check move flag
	if (moveType == ONLY_CAPTURES && capture == 0)
	{
		prevState.valid = 0; 
		return prevState;
	}

	// Make move
	bitboards[piece] = popBit(bitboards[piece], start);
	hashKey ^= PIECE_KEY[piece][start];
	bitboards[piece] = setBit(bitboards[piece], end);
	hashKey ^= PIECE_KEY[piece][end];

	// Handle captures
	if (capture == 1 && enPassant == 0)
	{
		bitboards[capturedPiece] = popBit(bitboards[capturedPiece], end);
		hashKey ^= PIECE_KEY[capturedPiece][end];
	}

	// Handle pawn promotions
	if (promotion != NULL_PIECE)
	{
		bitboards[piece] = popBit(bitboards[piece], end);
		hashKey ^= PIECE_KEY[piece][end];
		bitboards[promotion] = setBit(bitboards[promotion], end);
		hashKey ^= PIECE_KEY[promotion][end];
	}

	// Handle en passant
	if (enPassant == 1)
	{
		if (piece == P)
		{
			bitboards[p] = popBit(bitboards[p], end - 8);
			hashKey ^= PIECE_KEY[p][end - 8];
		}
		if (piece == p)
		{
			bitboards[P] = popBit(bitboards[P], end + 8);
			hashKey ^= PIECE_KEY[P][end + 8];
		}
	}

	// Reset the en passant square
	if (enPassantSquare != SQ_NONE)
	{
		hashKey ^= ENPASSANT_KEY[enPassantSquare];
	}
	enPassantSquare = SQ_NONE;

	// Handle double push
	if (doublePush == 1)
	{
		enPassantSquare = (piece == P ? end - 8 : end + 8);
		hashKey ^= ENPASSANT_KEY[enPassantSquare];
	}

	// Handle castling
	if (castling == 1)
	{
		if (end == G1)
		{
			bitboards[R] = popBit(bitboards[R], H1);
			hashKey ^= PIECE_KEY[R][H1];
			bitboards[R] = setBit(bitboards[R], F1);
			hashKey ^= PIECE_KEY[R][F1];
		}
		else if (end == C1)
		{
			bitboards[R] = popBit(bitboards[R], A1);
			hashKey ^= PIECE_KEY[R][A1];
			bitboards[R] = setBit(bitboards[R], D1);
			hashKey ^= PIECE_KEY[R][D1];
		}
		else if (end == G8)
		{
			bitboards[r] = popBit(bitboards[r], H8);
			hashKey ^= PIECE_KEY[r][H8];
			bitboards[r] = setBit(bitboards[r], F8);
			hashKey ^= PIECE_KEY[r][F8];
		}
		else if (end == C8)
		{
			bitboards[r] = popBit(bitboards[r], A8);
			hashKey ^= PIECE_KEY[r][A8];
			bitboards[r] = setBit(bitboards[r], D8);
			hashKey ^= PIECE_KEY[r][D8];
		}
	}

	// Update castling rights
	hashKey ^= CASTLE_KEY[castlingRights];

	// If white king moved, white king can't castle king / queen side
	if (piece == K) { castlingRights &= 0b1100; }
	// If black king moved, black king can't castle king / queen side
	if (piece == k) { castlingRights &= 0b0011; }
	// If white rook on h1 moved, white king can't castle king side. If white rook on a1 moved, white king can't castle queen side. 				
	if (piece == R && start == H1) { castlingRights &= 0b1110; }
	if (piece == R && start == A1) { castlingRights &= 0b1101; }
	// If black rook on h8 moved, black king can't castle king side. If black rook on a8 moved, black king can't castle queen side. 				
	if (piece == r && start == H8) { castlingRights &= 0b1011; }
	if (piece == r && start == A8) { castlingRights &= 0b0111; }
	// If white rook on h1 is captured, white king can't castle king side
	if (capturedPiece == R && end == H1) { castlingRights &= 0b1110; }
	// If white rook on a1 is captured, white king can't castle queen side
	if (capturedPiece == R && end == A1) { castlingRights &= 0b1101; }
	// If black rook on h8 is captured, black king can't castle king side
	if (capturedPiece == r && end == H8) { castlingRights &= 0b1011; }
	// If black rook on a8 is captured, black king can't castle queen side
	if (capturedPiece == r && end == A8) { castlingRights &= 0b0111; }

	hashKey ^= CASTLE_KEY[castlingRights];

	// Reset occupancy masks
	occupancies[WHITE] = 0ULL;
	occupancies[BLACK] = 0ULL;
	occupancies[ALL] = 0ULL;
	// Loop over pieces to update occupancy mask
	for (int whitePiece = P; whitePiece <= K; whitePiece ++)
	{
		occupancies[WHITE] |= bitboards[whitePiece];
	}
	for (int blackPiece = p; blackPiece <= k; blackPiece ++)
	{
		occupancies[BLACK] |= bitboards[blackPiece];
	}
	occupancies[ALL] = occupancies[BLACK] | occupancies[WHITE];

	// Update move number
	if (side == BLACK)
	{
		moveNum ++;
	}
	if (piece == P || piece == p || capture == 1)
	{
		fiftyMoveRuleCount = 0;
	}
	else
	{
		fiftyMoveRuleCount ++;
	}

	// Switch sides
	side = !side;
	hashKey ^= SIDE_KEY;

	// Hash from scratch to check for errors
	if (DEBUG_GAME)
	{
		uint64_t hashFromScratch = generateHashKey();
		assert(hashKey == hashFromScratch);
	}

	Bitboard playerKing = (side == BLACK ? bitboards[K] : bitboards[k]);
	if (isSquareAttacked(getLeastSignificantBitIndex(playerKing), side))
	{
		prevState.valid = 0;
		takeBack(prevState);
		return prevState;
	}

	prevState.valid = 1;
	return prevState;
}

void Game::generateAllMoves()
{
	// Reset move list every time we generate moves
	memset(moveList, 0, sizeof(moveList));
	// Initialize a pointer that points to moveList's first element;
	int *pointer = moveList;
	int pieceStart = (side == WHITE ? 0 : 6);
	
	for (int piece = pieceStart; piece <= pieceStart + 5; piece++)
	{
		Bitboard bb = bitboards[piece];
		if (piece == P)
		{
			while (bb)
			{
				// Quiet pawn moves
				int start = getLeastSignificantBitIndex(bb);
				int end = start + 8;
				// If end square is inside the board and the end square is not occupied
				if (!(end > H8) && !getBit(occupancies[ALL], end))
				{
					// Pawn promotions (white pawn pushes from the 7th rank)
					if (start >= A7 && start <= H7)
					{
						*pointer = encodeMove(start, end, piece, Q, NULL_PIECE, 0, 0, 0, 0); pointer++;
						*pointer = encodeMove(start, end, piece, R, NULL_PIECE, 0, 0, 0, 0); pointer++;
						*pointer = encodeMove(start, end, piece, B, NULL_PIECE, 0, 0, 0, 0); pointer++;
						*pointer = encodeMove(start, end, piece, N, NULL_PIECE, 0, 0, 0, 0); pointer++;
					}
					else
					{
						// One square pawn push
						*pointer = encodeMove(start, end, piece, NULL_PIECE, NULL_PIECE, 0, 0, 0, 0); pointer++;
						// Two square pawn push
						if ((start >= A2 && start <= H2) && !getBit(occupancies[ALL], end + 8))
						{
							*pointer = encodeMove(start, end + 8, piece, NULL_PIECE, NULL_PIECE, 0, 1, 0, 0); pointer++;
						}
					}
				}
				// Captures
				Bitboard attacks = PAWN_ATTACKS[WHITE][start] & occupancies[BLACK];
				int target, capturedPiece;
				while (attacks)
				{
					target = getLeastSignificantBitIndex(attacks);
					capturedPiece = getCaptures(target, side);
					// Handle promotion captures
					if (start >= A7 && start <= H7)
					{
						*pointer = encodeMove(start, target, piece, Q, capturedPiece, 1, 0, 0, 0); pointer++;
						*pointer = encodeMove(start, target, piece, R, capturedPiece, 1, 0, 0, 0); pointer++;
						*pointer = encodeMove(start, target, piece, B, capturedPiece, 1, 0, 0, 0); pointer++;
						*pointer = encodeMove(start, target, piece, N, capturedPiece, 1, 0, 0, 0); pointer++;						
					}
					// Handle normal captures
					else
					{
						*pointer = encodeMove(start, target, piece, NULL_PIECE, capturedPiece, 1, 0, 0, 0); pointer++;						
					}
					attacks = popBit(attacks, target);
				}
				// En passant
				if (enPassantSquare != SQ_NONE)
				{
					if (PAWN_ATTACKS[WHITE][start] & (1ULL << enPassantSquare))
					{
						*pointer = encodeMove(start, enPassantSquare, P, NULL_PIECE, p, 1, 0, 1, 0); pointer++;					
					}
				}
				bb = popBit(bb, start);
			}
		}
		else if (piece == p)
		{
			while (bb)
			{
				// Quiet pawn moves
				int start = getLeastSignificantBitIndex(bb);
				int end = start - 8;
				// If end square is inside the board and the end square is not occupied
				if (!(end < A1) && !getBit(occupancies[ALL], end))
				{
					// Pawn promotions (white pawn pushes from the 7th rank)
					if (start >= A2 && start <= H2)
					{
						*pointer = encodeMove(start, end, piece, q, NULL_PIECE, 0, 0, 0, 0); pointer++;
						*pointer = encodeMove(start, end, piece, r, NULL_PIECE, 0, 0, 0, 0); pointer++;
						*pointer = encodeMove(start, end, piece, b, NULL_PIECE, 0, 0, 0, 0); pointer++;
						*pointer = encodeMove(start, end, piece, n, NULL_PIECE, 0, 0, 0, 0); pointer++;
					}
					else
					{
						// One square pawn push
						*pointer = encodeMove(start, end, piece, NULL_PIECE, NULL_PIECE, 0, 0, 0, 0); pointer++;
						// Two square pawn push
						if ((start >= A7 && start <= H7) && !getBit(occupancies[ALL], end - 8))
						{
							*pointer = encodeMove(start, end - 8, piece, NULL_PIECE, NULL_PIECE, 0, 1, 0, 0); pointer++;
						}
					}
				}
				// Captures
				Bitboard attacks = PAWN_ATTACKS[BLACK][start] & occupancies[WHITE];
				int target, capturedPiece;
				while (attacks)
				{
					target = getLeastSignificantBitIndex(attacks);
					capturedPiece = getCaptures(target, side);
					// Handle promotion captures
					if (start >= A2 && start <= H2)
					{
						*pointer = encodeMove(start, target, piece, q, capturedPiece, 1, 0, 0, 0); pointer++;
						*pointer = encodeMove(start, target, piece, r, capturedPiece, 1, 0, 0, 0); pointer++;
						*pointer = encodeMove(start, target, piece, b, capturedPiece, 1, 0, 0, 0); pointer++;
						*pointer = encodeMove(start, target, piece, n, capturedPiece, 1, 0, 0, 0); pointer++;						
					}
					// Handle normal captures
					else
					{
						*pointer = encodeMove(start, target, piece, NULL_PIECE, capturedPiece, 1, 0, 0, 0); pointer++;						
					}
					attacks = popBit(attacks, target);
				}
				// En passant
				if (enPassantSquare != SQ_NONE)
				{
					if (PAWN_ATTACKS[BLACK][start] & (1ULL << enPassantSquare))
					{
						*pointer = encodeMove(start, enPassantSquare, p, NULL_PIECE, P, 1, 0, 1, 0); pointer++;					
					}
				}
				bb = popBit(bb, start);
			}
		}
		// Handle white kings castling moves
		if (piece == K)
		{
			if (castlingRights & WK)
			{
				if (!getBit(occupancies[ALL], F1) && !getBit(occupancies[ALL], G1))
				{
					if (!isSquareAttacked(E1, BLACK) && !isSquareAttacked(F1, BLACK) && !isSquareAttacked(G1, BLACK))
					{
						*pointer = encodeMove(E1, G1, K, NULL_PIECE, NULL_PIECE, 0, 0, 0, 1); pointer++;						
					}
				}
			}
			if (castlingRights & WQ)
			{
				if (!getBit(occupancies[ALL], D1) && !getBit(occupancies[ALL], C1) && !getBit(occupancies[ALL], B1))
				{
					if (!isSquareAttacked(E1, BLACK) && !isSquareAttacked(D1, BLACK) && !isSquareAttacked(C1, BLACK))
					{
						*pointer = encodeMove(E1, C1, K, NULL_PIECE, NULL_PIECE, 0, 0, 0, 1); pointer++;						
					}
				}
			}
		}
		// Handle black kings castling moves
		else if (piece == k)
		{
			if (castlingRights & BK)
			{
				if (!getBit(occupancies[ALL], F8) && !getBit(occupancies[ALL], G8))
				{
					if (!isSquareAttacked(E8, WHITE) && !isSquareAttacked(F8, WHITE) && !isSquareAttacked(G8, WHITE))
					{
						*pointer = encodeMove(E8, G8, k, NULL_PIECE, NULL_PIECE, 0, 0, 0, 1); pointer++;						
					}
				}
			}
			if (castlingRights & BQ)
			{
				if (!getBit(occupancies[ALL], D8) && !getBit(occupancies[ALL], C8) && !getBit(occupancies[ALL], B8))
				{
					if (!isSquareAttacked(E8, WHITE) && !isSquareAttacked(D8, WHITE) && !isSquareAttacked(C8, WHITE))
					{
						*pointer = encodeMove(E8, C8, k, NULL_PIECE, NULL_PIECE, 0, 0, 0, 1); pointer++;						
					}
				}
			}
		}
		// Handle king moves
		if (piece == K || piece == k)
		{
			int start;
			Bitboard attacks;
			while (bb)
			{
				start = getLeastSignificantBitIndex(bb);
				attacks = KING_ATTACKS[start] & (~occupancies[side]);
				int target, capturedPiece;
				while (attacks)
				{
					target = getLeastSignificantBitIndex(attacks);
					capturedPiece = getCaptures(target, side);
					if (capturedPiece != NULL_PIECE)
					{
						*pointer = encodeMove(start, target, piece, NULL_PIECE, capturedPiece, 1, 0, 0, 0); pointer++;		
					}
					else
					{
						*pointer = encodeMove(start, target, piece, NULL_PIECE, NULL_PIECE, 0, 0, 0, 0); pointer++;						
					}
					attacks = popBit(attacks, target);
				}
				bb = popBit(bb, start);
			}
		}
		// Handle knight moves
		if (piece == N || piece == n)
		{
			int start;
			Bitboard attacks;
			while (bb)
			{
				start = getLeastSignificantBitIndex(bb);
				attacks = KNIGHT_ATTACKS[start] & (~occupancies[side]);
				int target, capturedPiece;
				while (attacks)
				{
					target = getLeastSignificantBitIndex(attacks);
					capturedPiece = getCaptures(target, side);
					if (capturedPiece != NULL_PIECE)
					{
						*pointer = encodeMove(start, target, piece, NULL_PIECE, capturedPiece, 1, 0, 0, 0); pointer++;		
					}
					else
					{
						*pointer = encodeMove(start, target, piece, NULL_PIECE, NULL_PIECE, 0, 0, 0, 0); pointer++;						
					}
					attacks = popBit(attacks, target);
				}
				bb = popBit(bb, start);
			}
		}
		// Handle bishop moves
		if (piece == B || piece == b)
		{
			int start;
			Bitboard attacks;
			while (bb)
			{
				start = getLeastSignificantBitIndex(bb);
				attacks = generateBishopAttacks(start, occupancies[ALL]) & (~occupancies[side]);
				int target, capturedPiece;
				while (attacks)
				{
					target = getLeastSignificantBitIndex(attacks);
					capturedPiece = getCaptures(target, side);
					if (capturedPiece != NULL_PIECE)
					{
						*pointer = encodeMove(start, target, piece, NULL_PIECE, capturedPiece, 1, 0, 0, 0); pointer++;		
					}
					else
					{
						*pointer = encodeMove(start, target, piece, NULL_PIECE, NULL_PIECE, 0, 0, 0, 0); pointer++;						
					}
					attacks = popBit(attacks, target);
				}
				bb = popBit(bb, start);
			}
		}
		// Handle rook moves
		if (piece == R || piece == r)
		{
			int start;
			Bitboard attacks;
			while (bb)
			{
				start = getLeastSignificantBitIndex(bb);
				attacks = generateRookAttacks(start, occupancies[ALL]) & (~occupancies[side]);
				int target, capturedPiece;
				while (attacks)
				{
					target = getLeastSignificantBitIndex(attacks);
					capturedPiece = getCaptures(target, side);
					if (capturedPiece != NULL_PIECE)
					{
						*pointer = encodeMove(start, target, piece, NULL_PIECE, capturedPiece, 1, 0, 0, 0); pointer++;		
					}
					else
					{
						*pointer = encodeMove(start, target, piece, NULL_PIECE, NULL_PIECE, 0, 0, 0, 0); pointer++;						
					}
					attacks = popBit(attacks, target);
				}
				bb = popBit(bb, start);
			}
		}
		// Handle queen moves
		if (piece == Q || piece == q)
		{
			int start;
			Bitboard attacks;
			while (bb)
			{
				start = getLeastSignificantBitIndex(bb);
				attacks = generateQueenAttacks(start, occupancies[ALL]) & (~occupancies[side]);
				int target, capturedPiece;
				while (attacks)
				{
					target = getLeastSignificantBitIndex(attacks);
					capturedPiece = getCaptures(target, side);
					if (capturedPiece != NULL_PIECE)
					{
						*pointer = encodeMove(start, target, piece, NULL_PIECE, capturedPiece, 1, 0, 0, 0); pointer++;		
					}
					else
					{
						*pointer = encodeMove(start, target, piece, NULL_PIECE, NULL_PIECE, 0, 0, 0, 0); pointer++;						
					}
					attacks = popBit(attacks, target);
				}
				bb = popBit(bb, start);
			}
		}
	}
	if (DEBUG_GAME)
	{
		printMoveList();
	}
}

int Game::getCaptures(int target, int attacker)
{
	int pieceStart = (attacker == WHITE ? 6 : 0);
	for (int piece = pieceStart; piece <= pieceStart + 5; piece++)
	{
		if (getBit(bitboards[piece], target) != 0)
		{
			return piece;
		}
	}
	return NULL_PIECE;	
}

int Game::isSquareAttacked(int square, int attacker)
{
	if (attacker == WHITE) 
	{
		if (PAWN_ATTACKS[BLACK][square] & bitboards[P]) {return 1;}
	}
	else if (attacker == BLACK) 
	{
		if (PAWN_ATTACKS[WHITE][square] & bitboards[p]) {return 1;}
	}
	if (KNIGHT_ATTACKS[square] & (attacker == WHITE ? bitboards[N] : bitboards[n])) {return 1;}
	if (KING_ATTACKS[square] & (attacker == WHITE ? bitboards[K] : bitboards[k])) {return 1;}
	if (generateBishopAttacks(square, occupancies[ALL]) & (attacker == WHITE ? bitboards[B] : bitboards[b])) {return 1;}
	if (generateRookAttacks(square, occupancies[ALL]) & (attacker == WHITE ? bitboards[R] : bitboards[r])) {return 1;}
	if (generateQueenAttacks(square, occupancies[ALL]) & (attacker == WHITE ? bitboards[Q] : bitboards[q])) {return 1;}
	return 0;
}

void Game::printMoveList()
{
	cout << endl;
	cout << "     Move   P.  Pr.  Cp.  Cf.  DP.  Ep.  Cs.\n" << endl;
	int i = 1;
	int *p = moveList;
	while (*p != 0)
	{
		if (i < 10) { cout << " " << i << "   "; }
		else { cout << i << "   "; }
		cout << SQUARES[getStartSquare(*p)];
		cout << SQUARES[getEndSquare(*p)] << "   ";
		cout << UNICODE_PIECES[getPiece(*p)] << "    ";		
		cout << UNICODE_PIECES[getPromotion(*p)] << "    ";
		cout << UNICODE_PIECES[getCapturedPiece(*p)] << "    ";
		cout << getCaptureFlag(*p) << "    ";
		cout << getDoublePushFlag(*p) << "    ";
		cout << getEnpassantFlag(*p) << "    ";
		cout << getCastlingFlag(*p);
		cout << endl;
		i++;
		p++;
	}
}

