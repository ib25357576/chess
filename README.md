# Chess engine
This chess engine was primarily designed to test effectiveness of Transposition Table policies. It is simple but fully functioning one that you can use or play against.
Its overall strength is equivalent to approximately 2000 rapid on lichess, although there are quite a few lacks and fields for improvement. The engine is a command line program, there is no GUI. Below there are some commands you may find usefull. If you want to check all supported commands or edit them, check out Engine::start() function in Engine.cpp file. 
## Standard UCI commands supported
### position fen [arg]
Sets up the position to the specified one provided in FEN notation as the argument. Note that FEN string should include all additional fields, like catsle rights, en passant target and movecounts. Use "position startpos" command to reset position to the inititial one.
### position fen [arg] moves [args]
Sets the specified position and plays the moves specified in the seconds argument. All moves should be separated by space and follow UCI notation like e2e4, e1g1 (for castling), d7d8q (for promotion). If one of the moves is wrong the command plays all moves up to it.
### go
Searches for the best move and position evaluation. The search goes either up to the depth of 12 plies, or 10 seconds, whichever is reached first. All the evaluation is calculated in centi-pawns (devide by 100 to get a commonly used format).
### go time [arg]
Searches exactly up to the specified time in seconds. Argument should be a positive double.
### go depth [arg]
Searches up to the specified depth. Argument should be a positive integer.
### quit
Finishes the program.
## Non standard commands supported
### print
Prints current board state, together with turn right and checkers.
### fen
Computes FEN notation for current position.
### show moves
Prints all legal moves in current position.
### show best line
Prints best line in current position according to the last search if any.
### move [arg]
Makes a specified move on the board if legal. The argument is a move in UCI notation, like e2e4, e1g1 (for castling), d7d8q (for promotion).
### undo move
Undoes last played move if any. We do not recommend to use this feature too often, because it may affect Transposition Table workflow.
### play time/depth [arg]
Enters play mode, which you may wish to use to play against engine conviniently. The argument is either time to think per move (must be double), or depth to search for (must be integer). The engine will think for specified time each move, or search up to specified depth. The mode does not limit your thinking time. Input your move in UCI notation after each response. All moves will be made automatically. Command "exit" to exit this mode.
### tt policy [arg]
Switches Transposition Table to the new one with specified policy type. Checkout workflow info for TT policies supported. Note that this command deletes the whole previous TT, so all the data saved will be lost.
### drop tt
Clears all data saved to transposition table, however it remains enabled with the same policy.
### reset
Resets the whole engine. It includes the board state, all the history, and the transposition table.
## General engine workflow
The engine is a single-thread program. It uses iterative deepening, minimax recursion with alpha-beta pruning and quiescence search only to search the position in depth. Static evaluation function accounts only on matereal and piece-square tables. There is no opening book, so the engine will probably play the same moves with the same settings. Consider setting up a few first moves manually if you want to get different game scenarios. You may wish to use some opening positions from silver_op_suite.txt file. Both bitboards and piece arrays are used to hold a board state, magic bitboard technique is used for move generation. 
Transposition table is implemented and is allowed to be switched off or change the policy. 
### Transposition Table policies
Supported policy types are : "age" (prefer newest entry), "depth" (prefer deepest entry), "size" (prefer entry with larger subtree). Both depth and size policies have "last" and "num" extensions (write a policy in form "size_num" to turn them on). <br>
"Last" extension introduces extra condition to override an entry if its distance in plies from the start of the game is less than the one of the last irreversible move. It essentially assumes that position saved before the last irreversible is almost always unreachable (which is not exactly true, but usually happens only a few times during the game). <br>
"Num" extension introduces extra condition to override an entry if the number of irreversible moves played to reach it since the start of the game is lower than that number for the current board state. It relies on the fact that the number of irreversible move can only increase and thus this position cannot be reached. <br>
TT also allows "compare" policy which is essentially a two-tier policy (holds two entries per each TT index for two different policies). <br>
By default the policy is size_num.