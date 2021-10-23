# &#120536;
A C++ chess engine with uci support based on [Code Monkey King's youtube tutorials].

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