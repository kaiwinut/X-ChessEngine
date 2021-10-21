#ifndef ENGINE_H
#define ENGINE_H

const std::string VERSION = "1.0";

// Used for negamx and alpha beta pruning
const int INF = 50000;
const int MAX_PLY = 64;

// Used for move ordering
const int PV_MOVE_SCORE = 20000;
const int CAPTURE_SCORE = 10000;
const int FIRST_KILLER_SCORE = 9000;
const int SECOND_KILLER_SCORE = 8000;

// MMV_LVA stand for: Most valuable victim / Least valuable attacker
// indicies are [attacker] [victim]
const int MVV_LVA[12][12] = {
 	{105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605},
	{104, 204, 304, 404, 504, 604, 104, 204, 304, 404, 504, 604},
	{103, 203, 303, 403, 503, 603, 103, 203, 303, 403, 503, 603},
	{102, 202, 302, 402, 502, 602, 102, 202, 302, 402, 502, 602},
	{101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601},
	{100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600},
	{105, 205, 305, 405, 505, 605, 105, 205, 305, 405, 505, 605},
	{104, 204, 304, 404, 504, 604, 104, 204, 304, 404, 504, 604},
	{103, 203, 303, 403, 503, 603, 103, 203, 303, 403, 503, 603},
	{102, 202, 302, 402, 502, 602, 102, 202, 302, 402, 502, 602},
	{101, 201, 301, 401, 501, 601, 101, 201, 301, 401, 501, 601},
	{100, 200, 300, 400, 500, 600, 100, 200, 300, 400, 500, 600}
};

// Used for Tranposition Tables
const int MATE_SCORE = 48000;
const int MATE_VALUE = 49000;

// Used for Aspiration Windows
const int WINDOW_SIZE = 50;

// Used for LMR (Late Move Reduction) 
const int FULL_DEPTH_MOVES = 4;
const int REDUCTION_LIMIT = 3;

// Used for Null move pruning
const int NULL_MOVE_REDUCTION = 2;

// 
const int ASPIRATION_WINDOW_SIZE = 50;

// Transposition table
const int HASH_EXACT = 0;
const int HASH_ALPHA = 1;
const int HASH_BETA = 2;
const int HASH_SIZE = 0x100000;
const int NO_HASH_ENTRY = 100000;

struct HashEntry
{
	uint64_t hashKey;
	int depth;
	int flag;
	int score;
};

class Engine
{
public:
	Game game;
	int nodes;
	int ply;
	int duration;
	int knps;
	int bestEval;
	int inPV;
	int scorePV;
	int killerMoves[MAX_PLY][2];
	int historyMoves[12][64];
	int pvLength[MAX_PLY];
	int pvTable[MAX_PLY][MAX_PLY];
	int hashEntries;
	HashEntry * tt = NULL;
	int repetitionTable[1000];
	int repetitionIndex;

	Engine();
	void search(Game curerntGame, int depth);
	int PVS(int depth, int alpha, int beta);
	int quiescenceSearch(int alpha, int beta);	
	int scoreMove(int move);
	void sortMoves(int * moveList);
	void enablePVScoring();
	void resetEngine();
	void clearTranspositionTable();
	void initTranspositionTable(int mb);
	int readHashEntry(int depth, int alpha, int beta);
	void writeHashEntry(int depth, int alpha, int beta);
	int isRepetition();
	void printResults(int currentDepth, int depth_limit, int fullResults);	

	/*** UCI Section ***/

	int quit = 0;
	int movestogo = 70;
	int movetime = -1;
	int uciTime = -1;
	int inc = 0;
	int starttime = 0;
	int stoptime = 0;
	int timeset = 0;
	int stopped = 0;

	void uciLoop();
	int parseMove(Game& game, std::string moveString);
	void parsePosition(Game& game, std::string command);
	void parseGo(Game& game, std::string command);

	void resetParams();
	int getTimems();
	int waitForInput();
	void readInput();
	void communicate();
};

#endif