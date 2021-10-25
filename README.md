# &#120536;
A C++ chess engine with uci support based on [Code Monkey King's youtube tutorials].

## v1.1

#### Performance:
Below are the results of over 150 games played with stockfish (over 120 were played with the 2000 Elo version since they have the closest strength). After some minor updates, the engine's estimated Elo now became around 1970.

###### Blitz (2 mins with 1 sec increment)

| Stockfish Elo | Wins | Draws | Score | Total games |
| ------------- | ---- | ----- | ----- | ----------- |
| 1800          | 7    | 0     | 7     | 10          |
| 1900          | 3    | 1     | 3.5   | 6           |
| 2000          | 41   | 29    | 55.5  | 123         |
| 2100          | 2    | 0     | 2     | 4           |
| 2200          | 2    | 6     | 5     | 11          |

#### Search performace improvement:
Results above clearly indicate a great improvement in the engine's strength. Here is another example: in the kiwi position, v1.1 was able to search not only deeper but faster than v1.0. Delta pruning and evaluation pruning made the big difference by cutting off over half of the nodes searched.

###### v1.1: 

`info depth 10 score cp -75 time 620 nodes 957680 nps 1544000 pv e2a6 e6d5 c3d5 b6d5 a6b7 e7e5 b7a8 h3g2 f3g2 f6e4`

###### v1.0: 

`info depth 8 score cp -73 time 714 nodes 1352488 nps 1894000 pv e2a6 e6d5 e5g6 f7g6 c3b5 d5e4 b5c7 e8d8 f3g3 `

#### Updates from v1.0:

###### Move generation
- [x] Check masks (slightly speeds up the generation)

###### Evaluation
- [x] Pawn structures
- [x] King's safety
- [x] Piece mobility
- [x] Incremental game phase score update (boosts evaluation by 3x speed)

###### Search
- [x] Delta & Bad capture pruning in the quiescence search
- [x] Evaluation pruning
- [x] Static null move pruning
- [x] Razoring

###### Move ordering
- [x] Hash move ordering

###### UCI time control
- [x] some minor tuning

###### Simple GUI
- [x] Only for the purpose of watching engines play

#### Unfixed Bugs:
- [ ] Illegal move 'e5e6' in this position: `8/Rb6/8/1PBpk3/8/2b1Kp2/4B1P1/7r w - - 0 47`

- [ ] python-chess UCI showing illegal promotion while move is totally legal



## v1.0

#### Performance:
Some games against stockfish with limited strength. The engine is obviously terrible in bullet games, while in blitz ( 5 secs per move ), it has an estimated elo around 1700 to 1900, according to stockfish's elo. ( These results are not proven with any mathematical evidence and sould only serve as a source of reference to its possible strength. )

###### 5 secs per move
| Stockfish Elo | Score | Total games |
| ------------- | ----- | ----------- |
| 1800          | 0     | 1           |
| 1900          | 1.5   | 2           |
| 2000          | 0     | 1           |
| 2100          | 0     | 1           |
| 2200          | 0.5   | 3           |

###### 2 secs per move
| Stockfish Elo | Score | Total games |
| ------------- | ----- | ----------- |
| 1800          | 1     | 5           |
| 1900          | 3     | 9           |
| 2000          | 0     | 2           |
| 2100          | 0     | 3           |
| 2200          | 0     | 1           |

#### Supported features:

###### Board
- [x] Bitboard representation

###### Move generation
- [x] Move generation by magic bitboards
- [x] Pseudo legal move generation

###### Perft
- [x] Perft test (depth 7 in the start position passed)

###### Evaluation
- [x] PeSTO's evaluation function

###### Search
- [x] Negamax search + Alpha-beta pruning
- [x] Quiescence search
- [x] PVS (Principle variation search)
- [x] LRM (Late move reduction)
- [x] Null move pruning

###### Move ordering
- [x] Captures
- [x] Killer heuristics
- [x] History heuristics

###### Tranposition table
- [x] Zobrist hashing
- [x] Incremental move hashing
- [x] Transposition table
- [x] Checkmate, stalemate, three fold repetitions

###### UCI
- [x] Basic UCI protocol
- [x] UCI time control

#### Unsupported in this version:
- [ ] Opening book
- [ ] More heuristics for move ordering
- [ ] Own GUI



## v0.9
A basic chess engine without stable UCI support.

#### Supported features:

###### Board
- [x] Bitboard representation

###### Move generation
- [x] Move generation by magic bitboards
- [x] Pseudo legal move generation

###### Perft
- [x] Perft test (depth 7 in the start position passed)

###### Evaluation
- [x] Material
- [x] Pawn structure
- [x] King security
- [x] Piece mobility

###### Search
- [x] Negamax search + Alpha-beta pruning
- [x] Quiescence search
- [x] PVS (Principle variation search)
- [x] LRM (Late move reduction)
- [x] Null move pruning

###### Move ordering
- [x] Captures
- [x] Killer heuristics
- [x] History heuristics

###### Tranposition table
- [x] Zobrist hashing
- [x] Incremental move hashing
- [x] Transposition table
- [x] Checkmate, stalemate, three fold repetitions

#### Unsupported in this version:
- [ ] PeSTO's evaluation function
- [ ] UCI protocol
- [ ] UCI time control
- [ ] Opening book
- [ ] More heuristics for move ordering
- [ ] Own GUI

[Code Monkey King's youtube tutorials]: https://www.youtube.com/playlist?list=PLmN0neTso3Jxh8ZIylk74JpwfiWNI76Cs