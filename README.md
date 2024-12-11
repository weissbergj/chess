## Wizard's Chess

Unfortunately, there will never be enough atoms in the universe to "solve" a game of chess.

## Team members
* Jared Weissberg
* Zara Rutherford

## Project description
This is a naive implementation of a chess algorithm, using a minimax scoring function with alpha/beta pruning. Please see headers.txt for all functions. Use this to understand overview.txt (in place of detailed file headers), which describes how our program functions. We implement minimax with alpha-beta pruning and an initial heuristic. Relevant lines of code include approximately 3,740 (4,994 total) lines of C, 1,378 text, 411 JS, 131 CSS, 131 C Header, 198 Powershel, 77 Markdown, and 58 make among others. Lines of Python are unkown.

Final style submissions are titled jared-final-style.txt.

## Member contribution
A short description of the work performed by each member of the team.

## References
These two ideas inspired our project, although we adapted no code from either:
* https://github.com/ellenjxu/mango-chess
* https://www.chess.com/forum/view/chess-equipment/making-my-automatic-chessboard
The main algorithm was designed from scratch. There are no reference libraries or repositories, but there are countless examples on sites like Wikipedia of algorithms implemented (e.g., minimax, alpha/beta pruning). These require a simple Google search.

## Self-evaluation
We did everything we proposed except for Mango-Pi interaction (because it would be so easy to implement, we excluded it in favor of simplicity on one device). One thing worth mentioning is instead of networking through the Mango-Pi, we wrote a simple Python script to host the UART output, although this was just extra. It is almost identical to the terminal interface, so we demoed the terminal.

The chess algorithm works well enough to face an intermediate player. It incorporates edge cases like castling, en passant, etc. Areas of improvement include more efficiently generating and cycling through potential moves, dynamic scoring that incorporates---through board state rather than move count---more types of game play beyond beginning, middle, and end game. Furthermore, alpha-beta pruning and the initial heuristic could be more robust.

Ultimately, the goal is to make this algorithm as efficient as possible in the future to allow for more recursive calls of minimax (i.e., more depth). Dynamic recursive calling based on game state would be cool (e.g., 2 levels of depth in beginning versus 4-8 middle game)! We could have used Monte Carlo search trees instead of alpha-beta pruning or neural nets like AlphaZero.

I am proud of the breadth of edge cases I was able to include and the heuristic before the recursive minimax calls. Furthermore, this was a great effort in efficient algorithm design (putting 161 into practice). It took well over 60 hours to complete the chess engine without piece movement (~15,000 lines of code initially written and refactored/refined into ~4-5,000, not to mention Python + text). I hope you appreciate the effort. My guess is this might take an average SWE 400-600 hours. It took Stockfish ~40,000-100,000 LOC and 2 years to develop + 15 to refine.


How well was your team able to execute on the plan in your proposal?  
Any trying or heroic moments you would like to share? Of what are you particularly proud:
the effort you put into it? the end product? the process you followed?
what you learned along the way? Tell us about it!

## Photos
You are encouraged to submit photos/videos of your project in action. 
Add the files and commit to your project repository to include along with your submission.