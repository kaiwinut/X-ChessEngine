all: main

main: main.cpp
	g++ -std=c++17 -Ofast main.cpp engine.cpp eval.cpp perft.cpp movegen.cpp masks.cpp utils.cpp -o main && ./main

debug: main-debug

main-debug: main.cpp
	g++ -std=c++17 -Og main.cpp engine.cpp eval.cpp perft.cpp movegen.cpp masks.cpp utils.cpp -o main && ./main			