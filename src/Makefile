all: main

main: main.cpp
	g++ -std=c++17 -Ofast main.cpp uci.cpp search.cpp eval.cpp perft.cpp movegen.cpp masks.cpp bitboard.cpp -o main && ./main

debug: main-debug

main-debug: main.cpp
	g++ -std=c++17 -Og main.cpp uci.cpp search.cpp eval.cpp perft.cpp movegen.cpp masks.cpp bitboard.cpp -o main && ./main			