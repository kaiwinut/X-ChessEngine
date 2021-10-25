#include <iostream>
#include <string>
#include <cstring>
#include <map>
#include "utils.h"
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
	castlingRights = 0;
	moveNum = 0;
	fiftyMoveRuleCount = 0;
	hashKey = 0x0;
	gamePhaseScore = 6766;

	memset(bitboards, 0ull, sizeof(bitboards));
	memset(occupancies, 0ull, sizeof(occupancies));

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
	gamePhaseScore = getGamePhaseScore(*this);
	checkMask = getCheckMask(side ^ 1);
}

// Initilize bitboards for 12 pieces and 3 occupancy maps
void Game::initPieceBitboards()
{
	// Initilize bitboards for 12 pieces
	bitboards[P] = RANK_2_MASK;
	bitboards[N] = (FILE_B_MASK | FILE_G_MASK) & RANK_1_MASK;
	bitboards[B] = (FILE_C_MASK | FILE_F_MASK) & RANK_1_MASK;
	bitboards[R] = (FILE_A_MASK | FILE_H_MASK) & RANK_1_MASK;
	bitboards[Q] = FILE_D_MASK & RANK_1_MASK;
	bitboards[K] = FILE_E_MASK & RANK_1_MASK;
	bitboards[p] = RANK_7_MASK;
	bitboards[n] = (FILE_B_MASK | FILE_G_MASK) & RANK_8_MASK;
	bitboards[b] = (FILE_C_MASK | FILE_F_MASK) & RANK_8_MASK;
	bitboards[r] = (FILE_A_MASK | FILE_H_MASK) & RANK_8_MASK;
	bitboards[q] = FILE_D_MASK & RANK_8_MASK;
	bitboards[k] = FILE_E_MASK & RANK_8_MASK;

	// Initilize bitboards for 3 occupancy maps
	occupancies[WHITE] |= (bitboards[P] | bitboards[N] | bitboards[B] | bitboards[R] | bitboards[Q] | bitboards[K]);
	occupancies[BLACK] |= (bitboards[p] | bitboards[n] | bitboards[b] | bitboards[r] | bitboards[q] | bitboards[k]);
	occupancies[ALL] |= occupancies[WHITE] | occupancies[BLACK];
}

void Game::initHashKey()
{
	// Initialize piece hash keys
	for (int piece = P; piece <= k; piece ++)
	{
		for (int square = 0; square < 64; square ++)
		{
			PIECE_KEY[piece][square] = generateRandomUint64();
		}
	}

	// Initialize en passant hash keys
	for (int square = 0; square < 64; square ++)
	{
		ENPASSANT_KEY[square] = generateRandomUint64();
	}

	// Initialize side hash key
	SIDE_KEY = generateRandomUint64();

	// Initilize castle right hash key
	for (int i = 0; i < 16; i ++)
	{
		CASTLE_KEY[i] = generateRandomUint64();
	}
}

// Generate hash key of position from scratch
uint64_t Game::generateHashKey()
{
	uint64_t key = 0x0;
	uint64_t bb;
	int square;

	// Hash pieces
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
	
	// Hash en passant square
	if (enPassantSquare != SQ_NONE)
	{
		key ^= ENPASSANT_KEY[enPassantSquare];
	}
	
	// Hash castle right
	key ^= CASTLE_KEY[castlingRights];

	// Hash side (only when it is blacks side)
	if (side == BLACK)
	{
		key ^= SIDE_KEY;
	}

	return key;
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
	gamePhaseScore = prevState.gamePhaseScore;
	checkMask = prevState.checkMask;
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
	prevState.gamePhaseScore = gamePhaseScore;
	prevState.checkMask = checkMask;

	// Reset the en passant square
	if (enPassantSquare != SQ_NONE)
	{
		hashKey ^= ENPASSANT_KEY[enPassantSquare];
	}
	enPassantSquare = SQ_NONE;	

	// Switch sides
	side ^= 1;
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
	prevState.gamePhaseScore = gamePhaseScore;
	prevState.checkMask = checkMask;

	// Check move flag
	if (moveType == ONLY_CAPTURES && capture == 0)
	{
		prevState.valid = 0; 
		return prevState;
	}

	if (checkMask != 0 && (piece != K && piece != k) && (((1ull << end) & checkMask) == 0))
	{
		prevState.valid = 0; 
		return prevState;
	}

	if (checkMask != 0 && (piece == K && piece == k) && ((1ull << end) & checkMask))
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
	occupancies[WHITE] = 0ull;
	occupancies[BLACK] = 0ull;
	occupancies[ALL] = 0ull;
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
	side ^= 1;
	hashKey ^= SIDE_KEY;

	// Update game phase score
	if (capturedPiece != NULL_PIECE && capturedPiece != P && capturedPiece != K && capturedPiece != p && capturedPiece != k)
	{
		gamePhaseScore -= MATERIAL_ABS[OPENING][capturedPiece];
	}
	if (promotion != NULL_PIECE)
	{
		gamePhaseScore += MATERIAL_ABS[OPENING][promotion];
	}

	// Hash from scratch to check for errors
	if (DEBUG_GAME)
	{
		// cout << piece << SQUARES[start] << SQUARES[end] << castlingRights << endl;
		uint64_t hashFromScratch = generateHashKey();
		assert(hashKey == hashFromScratch);

		// int gamePhaseScoreFromScratch = getGamePhaseScore(*this);
		// assert(gamePhaseScore == gamePhaseScoreFromScratch);		
	}

	checkMask = getCheckMask(side ^ 1);

	Bitboard playerKingAfter = (side == BLACK ? bitboards[K] : bitboards[k]);
	if (isSquareAttacked(getLeastSignificantBitIndex(playerKingAfter), side))
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
		// printMoveList(*this);
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

uint64_t Game::isSquareAttacked(int square, int attacker)
{
	return (PAWN_ATTACKS[attacker ^ 1][square] & bitboards[p - 6 * attacker]) | (KNIGHT_ATTACKS[square] & bitboards[n - 6 * attacker]) | (KING_ATTACKS[square] & bitboards[k - 6 * attacker]) | (generateBishopAttacks(square, occupancies[ALL]) & bitboards[b - 6 * attacker]) | 
						(generateRookAttacks(square, occupancies[ALL]) & bitboards[r - 6 * attacker]) | (generateQueenAttacks(square, occupancies[ALL]) & bitboards[q - 6 * attacker]);
}

uint64_t Game::getCheckMask(int attacker)
{
	Bitboard playerKing = (attacker == BLACK ? bitboards[K] : bitboards[k]);
	int square = getLeastSignificantBitIndex(playerKing);

	uint64_t leaperCheck = (PAWN_ATTACKS[attacker ^ 1][square] & bitboards[p - 6 * attacker]) | (KNIGHT_ATTACKS[square] & bitboards[n - 6 * attacker]);
	uint64_t batk = generateBishopAttacks(square, occupancies[ALL]);
	uint64_t ratk = generateRookAttacks(square, occupancies[ALL]);
	uint64_t bishopCheck = batk & bitboards[b - 6 * attacker];
	uint64_t rookCheck = ratk & bitboards[r - 6 * attacker];
	uint64_t queenCheck = 0ull;

	if (bishopCheck)
	{
		bishopCheck = batk & ((bishopCheck) | generateBishopAttacks(getLeastSignificantBitIndex(bishopCheck), occupancies[ALL]));  
	}
	if (rookCheck)
	{
		rookCheck = ratk & (rookCheck | generateRookAttacks(getLeastSignificantBitIndex(rookCheck), occupancies[ALL]));
	}
	if (batk & bitboards[q - 6 * attacker])
	{
		queenCheck = batk & (batk & bitboards[q - 6 * attacker] | generateBishopAttacks(getLeastSignificantBitIndex(batk & bitboards[q - 6 * attacker]), occupancies[ALL]));
	}
	if (ratk & bitboards[q - 6 * attacker])
	{
		queenCheck = ratk & (ratk & bitboards[q - 6 * attacker] | generateRookAttacks(getLeastSignificantBitIndex(ratk & bitboards[q - 6 * attacker]), occupancies[ALL]));
	}

	return leaperCheck | bishopCheck | rookCheck | queenCheck;
}
