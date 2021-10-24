#include <iostream>
#include <map>
#include "utils.h"

using std::cout;
using std::endl;
using std::hex;
using std::dec;
using std::string;

// Display bitboard in 8 x 8 style
void displayBitboard(uint64_t board)
{
	int square;
	cout << "Board: 0x" << hex << board << endl;
	cout << dec;
	for (int rank = RANK_8; rank >= RANK_1; rank--)
	{
		for (int file = FILE_A; file <= FILE_H; file++)
		{
			square = rank * 8 + file;
			cout << (getBit(board, square) ? "1 " : ". ");
		}
		cout << endl;
	}
	cout << endl;
}

// Display game
void displayGame(Game game)
{
	int index;
	string osName;

	#ifdef __APPLE__
		osName = "Mac OSX";
	#else
		osName = "Others";
	#endif

	cout << "Hash: " << game.hashKey << endl;

	for (int rank = RANK_8; rank >= RANK_1; rank--)
	{
		cout << rank + 1 << "  ";
		for (int file = FILE_A; file <= FILE_H; file++)
		{
			index = rank * 8 + file;
			if (!getBit(game.occupancies[ALL], index))
			{
				cout <<  ". ";					
			}
			else
			{
				for (int piece = P; piece <= k; piece++)
				{
					if (getBit(game.bitboards[piece], index))
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

	cout << "   Side:     " << (game.side == WHITE ? "White" : "Black") << endl;
	cout << "   Enp.:        " << (game.enPassantSquare != SQ_NONE ? SQUARES[game.enPassantSquare] : "--") << endl;

	std::map <int, string> CASTLERIGHTS = {{0, "----"}, {1, "K---"}, {2, "-Q--"}, {3, "KQ--"}, {4, "--k-"}, {5, "K-k-"}, {6, "-Qk-"}, {7, "KQk-"},
									 {8, "---q"}, {9, "K--q"}, {10, "-Q-q"}, {11, "KQ-q"}, {12, "--kq"}, {13, "K-kq"}, {14, "-Qkq"}, {15, "KQkq"}};

	cout << "   Cast.:     " << CASTLERIGHTS[game.castlingRights] << endl;

	cout << endl;
}

void printMoveList(Game game)
{
	cout << endl;
	cout << "     Move   P.  Pr.  Cp.  Cf.  DP.  Ep.  Cs.\n" << endl;
	int i = 1;
	int *p = game.moveList;
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