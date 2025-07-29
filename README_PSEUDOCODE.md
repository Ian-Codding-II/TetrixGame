TetrixGame Pseudocode for Beginners
This guide explains how the TetrixGame (a Tetris-like game) works in simple steps, using pseudocode (a way to describe code in plain English). It’s designed for beginners who want to understand or recreate the game. The game has falling blocks (pieces) that you move to form lines, score points, and avoid filling the board.
Overview of the Game

Main Window: Shows the game board, score, level, lines cleared, next piece, and buttons (Start, Quit, Pause).
Game Board: A grid where pieces fall, and you stack them to clear lines.
Game Over Screen: Appears when the board fills up, showing your score and letting you enter your name.
Images: Uses island.jpg as the game background and gray.jpg for the game-over screen.

Pseudocode for the Game
Below is the pseudocode for each major part of the game. Think of it as a recipe for building the game.
1. Setting Up the Main Window
The main window is like a container that holds everything (board, buttons, labels, etc.).
CREATE a main window
SET the window size to 600 pixels wide, 400 pixels tall
SET the background to an image (island.jpg)

CREATE a stack of screens (to switch between game and game-over)
ADD two screens to the stack:
  - Game screen (shows board, buttons, etc.)
  - Game-over screen (shows score, name input, restart button)

SHOW the game screen by default

2. Game Screen Setup
The game screen has the board, labels (score, level, lines, next piece), and buttons.
CREATE a game screen
SET the background to island.jpg (scale it to fit the screen size)

CREATE a game board (a grid, e.g., 10 columns by 20 rows)
  - The board tracks where pieces are placed
  - Each cell in the grid is either empty or has a piece part

CREATE labels to show:
  - Score (starts at 0)
  - Level (starts at 1)
  - Lines cleared (starts at 0)
  - Next piece (shows the upcoming piece)

CREATE buttons:
  - Start button (to begin the game)
  - Quit button (to close the game)
  - Pause button (to pause/resume, disabled until game starts)

CREATE a right panel to hold:
  - Next piece label
  - Start button
  - Quit button
  - Pause button
  - Score label
  - Level label
  - Lines label
ARRANGE the panel items vertically, centered

ARRANGE the game screen:
  - Place the game board on the left (takes most space)
  - Place the right panel on the right

3. Game Board Logic
The board handles pieces falling, moving, and clearing lines.
INITIALIZE the game board:
  SET grid to empty (10x20 cells)
  SET score to 0
  SET level to 1
  SET lines cleared to 0
  SET game as not started
  SET pause as false
  CREATE a list of piece shapes (e.g., L-shape, square, T-shape)

WHEN the game starts:
  SET game as started
  ENABLE the pause button
  CREATE a random piece at the top center of the board
  SET a timer to move the piece down every second

WHEN the timer ticks:
  IF game is paused, do nothing
  MOVE the current piece down one row
  IF the piece cannot move down (hits bottom or another piece):
    FIX the piece in place on the grid
    CHECK for complete lines (rows with no empty cells)
    IF complete lines exist:
      REMOVE those lines
      MOVE all pieces above down
      INCREASE lines cleared by number of lines removed
      INCREASE score (e.g., 100 points per line)
      IF lines cleared reaches a threshold (e.g., 10 lines):
        INCREASE level (makes pieces fall faster)
    CREATE a new random piece at the top
    IF the new piece cannot be placed (board is too full):
      TRIGGER game over

WHEN a key is pressed:
  IF game is paused or not started, do nothing
  IF left arrow:
    MOVE piece left (if it doesn’t hit a wall or another piece)
  IF right arrow:
    MOVE piece right (if it doesn’t hit a wall or another piece)
  IF down arrow:
    MOVE piece down (if it doesn’t hit the bottom or another piece)
  IF up arrow or space:
    ROTATE piece (if it doesn’t hit walls or other pieces)

WHEN pause button is clicked:
  IF game is started:
    TOGGLE pause state
    IF paused:
      STOP the timer
      CHANGE pause button text to "Resume"
    ELSE:
      START the timer
      CHANGE pause button text to "Pause"

4. Game Over Screen
The game-over screen appears when the board fills up.
CREATE a game-over screen
SET the background to gray.jpg (scale it to fit the screen size)

CREATE a label to show:
  - "Game Over!"
  - Final score
  - High score (read from a file, highscores.txt)

CREATE a text input for the player’s name
CREATE a restart button

ARRANGE the game-over screen:
  - Place the label, text input, and restart button vertically, centered

WHEN game over is triggered:
  READ the high score from highscores.txt
  SHOW the game-over screen
  FADE IN the screen (from invisible to visible over 1 second)
  IF the current score is higher than the high score:
    WHEN restart button is clicked:
      SAVE the player’s name and score to highscores.txt
      SWITCH to the game screen
      START a new game
  ELSE:
    WHEN restart button is clicked:
      SWITCH to the game screen
      START a new game

5. Updating Labels
The score, level, lines, and next piece labels update during the game.
WHEN score changes:
  UPDATE score label to show "SCORE: <new score>"

WHEN level changes:
  UPDATE level label to show "LEVEL: <new level>"

WHEN lines cleared changes:
  UPDATE lines label to show "LINES: <new lines>"

WHEN a new piece is created:
  SHOW the next piece in the next piece label

6. Window Resizing
The game adjusts when the window size changes.
WHEN the window is resized:
  SCALE the background images (island.jpg, gray.jpg) to fit the new size
  ADJUST the font size of buttons and labels based on window height
  KEEP the game board and right panel proportional

7. Saving High Scores
High scores are saved to a file.
WHEN saving a high score:
  OPEN highscores.txt for writing
  WRITE the player’s name and score (e.g., "Player,100")
  CLOSE the file

How to Recreate the Game
To build this game, you’ll need:

A programming language: This game uses C++, but you could use Python, Java, or others.
A graphics library: This uses Qt for windows and buttons. Alternatives include Pygame (Python) or JavaFX.
Steps:
Create a window with two screens (game and game-over).
Set up a grid for the game board.
Write logic to spawn, move, and rotate pieces.
Check for complete lines and update score/level.
Handle user input (arrow keys, buttons).
Switch to the game-over screen when the board fills.
Save high scores to a file.
Add background images that scale with the window.



Tips for Beginners

Start small: Try making a grid where a single block moves left/right/down.
Add one feature at a time (e.g., rotation, scoring, game-over).
Use a simple graphics tool to draw the board and pieces.
Test often to catch mistakes early.
Look up tutorials for your chosen language (e.g., “Tetris in Python Pygame”).

Notes

The images (island.jpg, gray.jpg) are in the images/ folder.
The game may have a resizing issue (window too large or not shrinking). This is noted in the main README.md.
This pseudocode is a simplified version of the real code, focusing on the main ideas.

Happy coding! If you’re stuck, check the main README.md for build instructions or ask for help.