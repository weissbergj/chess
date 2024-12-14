## Wizard's Chess
Unfortunately, there will never be enough atoms in the universe to solve a game of chess.

## Team members
* Jared Weissberg
* Zara Rutherford

## Project description
This is a naive implementation of a chess algorithm, using a minimax scoring function with alpha/beta pruning. A 2-joint robotic arm moves these pieces using an electromagnet, servo motors, and rotary encoders (position). Please see headers.txt for all functions. Use this to understand overview.txt (in place of detailed file headers), which describes how our program functions. We implement minimax with alpha-beta pruning and an initial heuristic.

Final style submissions are titled [jared-final-style.txt](jared-final-style.txt) and [zara-final-style.txt](zara-final-style.txt).

Key files:
* [main.c](main.c)
* [hardware/myprogram.c](hardware/myprogram.c) 
* [overview.txt](overview.txt)
* [example_game.txt](example_game.txt)
* [headers.txt](headers.txt)
* [demo.jpeg](demo.jpeg)
* [demo.MOV](demo.MOV)

## Member contribution
Jared focused on the chess algorithm. Zara constructed the 2-joint robotic arm and wrote the hardware main program and all header c and h files. Jared and Zara integrated the hardware main program into the chess engine (i.e., rewriting myprogram.c to create a mapping from board positional values to move from initial to source, move to destination, and return to initial position).

## References
These two ideas inspired our project, although we adapted no code from either:
* https://github.com/ellenjxu/mango-chess
* https://www.chess.com/forum/view/chess-equipment/making-my-automatic-chessboard
The main algorithm was designed from scratch. There are examples on sites like Wikipedia of algorithms implemented (e.g., minimax, alpha/beta pruning).

These sites were referenced for the 2-joint arm:
* https://robotacademy.net.au/lesson/inverse-kinematics-for-a-2-joint-robot-arm-using-geometry/
* (Previous experience with the math behind robotics)

## Self-evaluation
We did everything we proposed except for Mango-Pi interaction (we excluded it in favor of simplicity on one device). One thing worth mentioning is instead of networking through the Mango-Pi, we wrote a simple Python script to host the UART output, although this was just extra. It is almost identical to the terminal interface, so we demoed the terminal.

The chess algorithm works well enough to face an intermediate player. It incorporates edge cases like castling, en passant, draw by threefold repetition, etc. Areas of improvement include more efficiently generating and cycling through potential moves, dynamic scoring that incorporates---through board state rather than move count---more types of game play beyond beginning, middle, and end game. Furthermore, alpha-beta pruning and the initial heuristic could be more robust.

Ultimately, the goal is to make this algorithm as efficient as possible in the future to allow for more recursive calls of minimax (i.e., more depth). Dynamic recursive calling based on game state would be cool (e.g., 2 levels of depth in beginning versus 4-8 middle game)! We could have used Monte Carlo search trees instead of alpha-beta pruning or neural nets like AlphaZero.

We are proud of the breadth of edge cases we was able to include and the heuristic before the recursive minimax calls. Furthermore, this was a great effort in efficient algorithm design (putting 161 into practice). It took well over 100 hours to complete the chess engine without piece movement (~15,000 lines of code initially written and refactored/refined into ~4-5,000, not to mention Python + text). We hope you appreciate the effort. My guess is this might take an average SWE 400-600 hours. It took Stockfish ~40,000-100,000 LOC and 2 years to develop + 15 to refine.

In terms of hardware, we struggled initially with the mechanics of our project. We quickly learnt that the lack of positional control with the servos posed a challenge when needing precise movements to move a piece from one square to another. Initially, we attempted a simple time based approach using the velocity of the servoes to determine their position and use that to start/stop rotation at the right time. However, the servos would continuously change velocities so that that wouldn't be effective. The next solution was to look at hall sensors to potentially fix our problem and provide accurate rotation. However, since we are working with an electromagnet and have magnetic chess pieces, we feared these could interfere with the calculations. Ultimately, we decided on using interrupt based rotary encoders and keeping one end fixed whilst the other rotated along with the servo. The number of pulses of rotation by the rotary encoder could then be translated to the angle degree the servo had turned. Here as well though, the difference in angles between each pulse was relatively high (10-15 degrees) and proved to be tricky to provide precise movement. The maths behind the positioning of the electromagnet was based on inverse kinematics using the size of the board and each leg to calculate the necessary angles of each servo to reach the correct position which worked well. 

Another challenge was related to the magnets. If the chess pieces were too magnetic, they would interfere with one another and push others out of the way. However, if they were too weak, it wasn't possible to use the electromagnet under the board to move it. After testing 4 different sets, we found one that had the right amount of strength to move through the board and not interfere with the other pieces. (Although 18 of the pieces had different polarities! So we spent a couple of hours just sawing off their ends and reattaching them upside down to fix the problem). Further, the electromagnet has a permanent magnet that would also cause interference even when not switched on. In the final demo, you can see that when the electromagnet is turned on, the piece is dragged, it then 'lets go' and returns to its original position. This was thanks to a balance of the width of the board and the chess piece itself.

In terms of code, to move a piece from the source position to the destination, it doesn't suffice to just move the motors from one set of angles to another as that would lead to the piece moving in an unnatural way. To implement a clean straight line movement, we would increment the distance in small steps along rows/columns to mimick a real chess movement. 

The project proved to be quite challenging hardware wise, and we had to learn to CAD and 3d print our own models to allow full rotation where necessary and reach all points in an xy plane. Discovering and using Lab64 so much was a pleasure and something we will definitely be using in future classes. 

One of the things that doesn't make it events like demoes is the Wednesday trip to the LEGO store to buy small car kits for wheels to move the 2-joint arm :) (and 3 trips to the amazon locker in Whole Foods!)

How well was your team able to execute on the plan in your proposal?  
Any trying or heroic moments you would like to share? Of what are you particularly proud:
the effort you put into it? the end product? the process you followed?
what you learned along the way? Tell us about it!

## Photos

[![Demo Video](demo.jpeg)](https://youtu.be/SC8W8NMmxuk)

See the [demo video](https://youtu.be/SC8W8NMmxuk) on YouTube, [demo.MOV](demo.MOV), and [demo.jpeg](demo.jpeg) for an example of the arm. See [example_game.txt](example_game.txt) for a sample game and commentary.

## Program Logic
This is the core logic for the chess engine. It is taken verbatim from overview.txt.

- main in ./main.c
  - find_best_move (AI's turn)
    - For AI's pieces:
      - generate_moves (calls piece-specific generators)
        - generate_pawn_moves
        - generate_knight_moves
        - generate_bishop_moves
        - generate_rook_moves
        - generate_queen_moves
        - generate_king_moves
    - For each generated move:
      - move_sort (assigns a preliminary score)
        - evaluate_castling (if applicable)
        - evaluate_captures (if applicable)
      - make_move (simulate move on a copy)
      - minimax (evaluate move recursively)
        - If depth == 0 or game over:
          - evaluate_board
            - Initialize total_score
            - Loop through board squares:
              - For each piece:
                - evaluate_position
                  - Switch based on piece type:
                    - evaluate_pawn
                    - evaluate_knight
                    - evaluate_bishop
                    - evaluate_rook
                    - evaluate_queen
                    - evaluate_king
                - Return position_value
              - Update total_score
            - Continue looping through board
            - Apply additional evaluations:
              - evaluate_king_safety
              - Other strategic considerations
            - Return total_score
        - If depth > 0:
          - For opponent's pieces:
            - generate_moves (calls piece-specific generators)
          - For each opponent's move:
            - move_sort (assigns a preliminary score)
            - make_move (simulate opponent's move)
            - minimax (recursive call)
          - Alpha-beta pruning
    - Select move with best score
  - make_move (execute AI's move)
    - Update board state
    - Handle special moves (castling, en passant, promotion)
    - Update game state variables (move_count, move_history)
    - Check for draw conditions (e.g., fifty-move rule, repetition)
  - Proceed to player's turn



Below is the main function called in hardware/myprogram.c for the 2-joint robotic arm:

Example move: Move piece from (1,1) to (3,3)

move_piece(1, 1, 3, 3, &servo_centre, &servo_outer, centre_encoder, outer_encoder);

This will:
1. Move from init to (1,1) by calling calculating_angles from IK.c (inverse kinematics)
2. Pick piece (activates electromagnet)
3. Move to (3,3)
4. Drop piece
5. Return back to initial position by reversing the ticks
