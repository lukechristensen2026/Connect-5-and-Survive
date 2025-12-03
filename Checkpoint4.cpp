/***********************************
 * CSC498B Senior Project
 * Professor: Maria Webb
 * Student: Luke Christensen
 *
 *
 *
 * Original idea of a Connect Four Game started in CSC121 with partner Eli VanHaaren.
 * Referenced code from: Paul W. Bible, Michael Estes, and user Snowbody on StackExchange.
 * Developed a basic Connect 5 in a terminal during COMP3330/6380 Machine Intelligence.
 * This version had a simple minimax AI without alpha-beta pruning.
 *
 * Code was completed by me for this course.
 * Tutorials and research literature were used to learn mini-max and alpha-beta pruning concepts.
 * ChatGPT used for some function idea generation.
 * ChatGPT was used to develop moveHistorySnapshots, pushBoardSnapshot, popBoardSnapshot, and normalizeGravity.
 * I understand the concepts and developed the majority of this code.
 *
 * Additional information can be found in my Final Report.  Contact me with additional questions.
 * To run this code, you will need the raylib library: https://www.raylib.com/.
 * Ideally, you will run using all code found on my GitHub.
 *
 * Build and run the .exe file!  Enjoy!
 */

// initial includes
#include <string>
#include <iostream>
#include <vector>
#include "raylib.h"
using namespace std;

// Dot class to represent each checker on the board
class Dot
{
public:
    int x;
    int y;
    bool isSlotTaken;
    bool isPlayer1;
    Color checkerColor;

    Dot()
    {
        x = 0;
        y = 0;
        isSlotTaken = false;
        isPlayer1 = false;
        checkerColor = WHITE;
    }

    Dot(int xPosition, int yPosition, bool isSlot, bool whatPlayer, Color color)
    {
        x = xPosition;
        y = yPosition;
        isSlotTaken = isSlot;
        isPlayer1 = whatPlayer;
        checkerColor = color;
    }
};

// Defined game constants
const int boardWidth = 8;
const int boardHeight = 7;
int currentPlayer = 1;
Dot board[boardWidth][boardHeight];
bool isGameOver = false;
Color playerColor;

// Player 1 power up states
bool playerPower1Active = false;
bool playerPower2Active = false;
bool playerPower3Active = false;
bool playerPower4Active = false;
bool playerPower1Used = false;
bool playerPower2Used = false;
bool playerPower3Used = false;
bool playerPower4Used = false;

// Player 2 power up states
bool player2Power1Active = false;
bool player2Power2Active = false;
bool player2Power3Active = false;
bool player2Power4Active = false;
bool player2Power1Used = false;
bool player2Power2Used = false;
bool player2Power3Used = false;
bool player2Power4Used = false;

// AI usage flags
bool aiPower1Used = false;
bool aiPower2Used = false;
bool aiPower3Used = false;
bool aiPower4Used = false;

/////Begin ChatGPT implemented code/////
// Move history: each entry is a full-board snapshot (vector of Dot of size boardWidth*boardHeight)
vector<vector<Dot>> moveHistorySnapshots;
// Snapshot current board (used before making a move so we can undo)
static inline void pushBoardSnapshot()
{
    vector<Dot> snap;
    // snap.reserve(boardWidth * boardHeight);
    for (int c = 0; c < boardWidth; c++)
        for (int r = 0; r < boardHeight; r++)
            snap.push_back(board[c][r]);
    moveHistorySnapshots.push_back((snap));
}
// Restore last snapshot
static inline void popBoardSnapshot()
{
    if (moveHistorySnapshots.empty())
        return;
    vector<Dot> &snap = moveHistorySnapshots.back();
    int idx = 0;
    for (int c = 0; c < boardWidth; c++)
        for (int r = 0; r < boardHeight; r++)
        {
            board[c][r] = snap[idx++];
        }
    moveHistorySnapshots.pop_back();
}
// Normalize gravity for entire board: ensure checkers fall to lowest available slots in each column
static inline void normalizeGravity()
{
    for (int c = 0; c < boardWidth; c++)
    {
        vector<Dot> colDots;
        // collect all occupied dots in this column from bottom-to-top order
        for (int r = boardHeight - 1; r >= 1; r--)
        {
            if (board[c][r].isSlotTaken)
                colDots.push_back(board[c][r]);
            // clear in preparation
            board[c][r] = Dot();
        }
        // place them back starting from bottom
        int rptr = boardHeight - 1;
        for (size_t i = 0; i < colDots.size(); i++, rptr--)
        {
            Dot d = colDots[i];
            d.x = c;
            d.y = rptr;
            board[c][rptr] = d;
        }
        board[c][0] = Dot(); // ensure top is empty
    }
}
/////End ChatGPT implemented code/////

// Switch the current player
void switchPlayer()
{
    if (currentPlayer == 1)
    {
        currentPlayer = 0;
    }
    else if (currentPlayer == 0)
    {
        currentPlayer = 1;
    }
    return;
}

// Make a move on the board
// also implemented in the minimax algorithm
// saves if a slot in the grid is taken
bool makeMove(int column, int powerup, bool isPlayer, Color playCol)
{
    if (column < 0 || column >= boardWidth)
    {
        return false;
    }

    int row = -1;
    for (int r = boardHeight - 1; r >= 0; r--)
    {
        if (!board[column][r].isSlotTaken)
        {
            row = r;
            break;
        }
    }
    if (row == -1)
    {
        return false;
    }

    pushBoardSnapshot();
    Color color = isPlayer ? YELLOW : RED;

    board[column][row] = Dot(column, row, true, isPlayer, playCol);

    if (powerup == 1) // Double Checker
    {
        // need to check under as well.  Can't call at 0.

        if (row - 1 >= 0 && !board[column][row - 1].isSlotTaken)
        {
            board[column][row - 1] = Dot(column, row - 1, true, isPlayer, playCol);
        }
    }

    if (powerup == 2) // magnet
    {
        if (column - 2 >= 0 && board[column - 2][row].isSlotTaken)
        {
            if (!board[column - 1][row].isSlotTaken)
            {
                board[column - 1][row] = board[column - 2][row];
            }
            board[column - 2][row] = Dot();
        }
        if (column + 2 < boardWidth && board[column + 2][row].isSlotTaken)
        {
            if (!board[column + 1][row].isSlotTaken)
            {
                board[column + 1][row] = board[column + 2][row];
            }
            board[column + 2][row] = Dot();
        }
    }

    if (powerup == 3) // bomb
    {
        if (column - 1 >= 0)
        {
            board[column - 1][row] = Dot();
        }
        if (column + 1 < boardWidth)
        {
            board[column + 1][row] = Dot();
        }
    }

    if (powerup == 4) // drill
    {
        if (row + 1 < boardHeight)
        {
            board[column][row + 1] = Dot();
        }
    }

    normalizeGravity();
    return true;

    return false;
}

// Necessary for minimax algorithm
// Replaced with ChatGPT implemented code
void undoMove()
{
    popBoardSnapshot();
}

// Prints out the game board to the terminal
void printBoard()
{
    // Constants for drawing the board
    const int columns = 7;
    const int rows = 6;
    const int cellSize = 100;
    const int startX = 400;
    const int startY = 30;
    const int lineThickness = 5;

    // Draws the checkers on the board
    for (int y = 0; y < boardHeight; y++)
    {
        for (int x = 0; x < boardWidth; x++)
        {
            if (board[x][y].isSlotTaken)
            {
                int circleX = startX + x * cellSize + cellSize / 2;
                int circleY = startY + y * cellSize - cellSize / 2;
                DrawCircle(circleX, circleY, 40, board[x][y].checkerColor);
            }
        }
    }

    // Draw vertical board lines
    for (int col = 0; col <= columns; col++)
    {
        float x = startX + col * cellSize;
        DrawLineEx({x, startY}, {x, startY + rows * cellSize}, lineThickness, BLACK);
    }
    // Draw horizontal board lines
    for (int row = 0; row <= rows; row++)
    {
        float y = startY + row * cellSize;
        DrawLineEx({startX, y}, {startX + columns * cellSize, y}, lineThickness, BLACK);
    }
}

// To be used in evaluateBoard
int evaluateLine(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4, int x5, int y5)
{
    int aiCount = 0;
    int opponentCount = 0;

    Dot line[5] = {board[x1][y1], board[x2][y2], board[x3][y3], board[x4][y4], board[x5][y5]};

    for (int i = 0; i < 5; i++)
    {
        if (line[i].isSlotTaken)
        {
            if (line[i].isPlayer1)
            {
                opponentCount++;
            }
            else
            {
                aiCount++;
            }
        }
    }

    if (aiCount == 5)
    {
        return 10000; // AI win
    }
    if (opponentCount == 5)
    {
        return -10000; // User win
    }
    if (aiCount == 4 && opponentCount == 0)
    {
        return 100; // AI 4-in-a-row
    }
    if (opponentCount == 4 && aiCount == 0)
    {
        return -100; // User 4-in-a-row
    }
    if (aiCount == 3 && opponentCount == 0)
    {
        return 50; // AI 3-in-a-row
    }
    if (opponentCount == 3 && aiCount == 0)
    {
        return -50; // User 3-in-a-row
    }
    if (aiCount == 2 && opponentCount == 0)
    {
        return 10; // AI 2-in-a-row
    }
    if (opponentCount == 2 && aiCount == 0)
    {
        return -10; // User 2-in-a-row
    }

    return 0;
}

// To be used in minimax
int evaluateBoard()
{
    int score = 0;
    // Horizontal Lines
    for (int y = 0; y < boardHeight; y++)
    {
        for (int x = 0; x < boardWidth - 4; x++)
        {
            score += evaluateLine(x, y, x + 1, y, x + 2, y, x + 3, y, x + 4, y);
        }
    }
    // Vertical Lines
    for (int x = 0; x < boardWidth; x++)
    {
        for (int y = 0; y < boardHeight - 4; y++)
        {
            score += evaluateLine(x, y, x, y + 1, x, y + 2, x, y + 3, x, y + 4);
        }
    }
    // Diagonal Lines (bottom-left to top-right)
    for (int x = 0; x < boardWidth - 4; x++)
    {
        for (int y = 4; y < boardHeight; y++)
        {
            score += evaluateLine(x, y, x + 1, y - 1, x + 2, y - 2, x + 3, y - 3, x + 4, y - 4);
        }
    }
    // Diagonal Lines (top-left to bottom-right)
    for (int x = 0; x < boardWidth - 4; x++)
    {
        for (int y = 0; y < boardHeight - 4; y++)
        {
            score += evaluateLine(x, y, x + 1, y + 1, x + 2, y + 2, x + 3, y + 3, x + 4, y + 4);
        }
    }
    return score;
}

// Recursively called to make the tree of moves
// The most optimal move should be tracked in bestScore
int minimax(int depth, bool isMaximizing, int alpha, int beta)
{
    int score = evaluateBoard();
    if (depth == 0 || score == 100000 || score == -100000)
    {
        return score;
    }

    if (isMaximizing)
    {
        int bestScore = -1000;
        for (int col = 0; col < boardWidth; col++)
        {
            for (int power = 0; power <= 4; power++)
            {
                if (makeMove(col, power, false, WHITE)) // AI move
                {
                    int val = minimax(depth - 1, false, alpha, beta);
                    undoMove();
                    bestScore = max(bestScore, val);
                    alpha = max(alpha, bestScore);
                    if (beta <= alpha)
                    {
                        break;
                    }
                }
            }
            if (beta <= alpha)
            {
                break;
            }
        }
        return bestScore;
    }
    else
    {
        int bestScore = 1000;
        for (int col = 0; col < boardWidth; col++)
        {
            for (int power = 0; power <= 4; power++)
            {
                if (makeMove(col, power, true, WHITE)) // User move
                {
                    int val = minimax(depth - 1, true, alpha, beta);
                    undoMove();
                    bestScore = min(bestScore, val);
                    beta = min(beta, bestScore);
                    if (beta <= alpha)
                    {
                        break;
                    }
                }
            }
            if (beta <= alpha)
            {
                break;
            }
        }
        return bestScore;
    }
}

int powerupPenalty(int power)
{
    if (power == 0)
    {
        return 0;
    }
    if (power == 1)
    {
        return -20;
    }
    if (power == 2)
    {
        return -30;
    }
    if (power == 3)
    {
        return -40;
    }
    if (power == 4)
    {
        return -20;
    }
    return 0;
}

// Utilizes minimax to determine the best move
int getBestMove(int difficulty, int &bestPowerUp)
{
    int bestScore = -100000;
    int bestMove = -1;
    bestPowerUp = 0;

    for (int i = 0; i < boardWidth; i++)
    {
        if (!board[i][0].isSlotTaken)
        {
            for (int power = 0; power <= 4; power++)
            {
                if (!(power == 1 && aiPower1Used))
                {
                    if (!(power == 2 && aiPower2Used))
                    {
                        if (!(power == 3 && aiPower3Used))
                        {
                            if (!(power == 4 && aiPower4Used))
                            {
                                if (makeMove(i, power, false, WHITE))
                                {
                                    int score = minimax(difficulty, false, -10000, 10000); // Originally depth of 5 (any more causes a long wait)

                                    score += powerupPenalty(power); // depth now based on game mode
                                    undoMove();

                                    if (score > bestScore)
                                    {
                                        bestScore = score;
                                        bestMove = i;
                                        bestPowerUp = power;
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return bestMove;
}

bool checkWin()
{
    // Check horizontal
    for (int x = 0; x < boardWidth - 4; x++)
    {
        for (int y = 0; y < boardHeight; y++)
        {
            if (board[x][y].isSlotTaken &&
                board[x + 1][y].isSlotTaken &&
                board[x + 2][y].isSlotTaken &&
                board[x + 3][y].isSlotTaken &&
                board[x + 4][y].isSlotTaken &&
                board[x][y].isPlayer1 == board[x + 1][y].isPlayer1 &&
                board[x][y].isPlayer1 == board[x + 2][y].isPlayer1 &&
                board[x][y].isPlayer1 == board[x + 3][y].isPlayer1 &&
                board[x][y].isPlayer1 == board[x + 4][y].isPlayer1)
            {
                board[x][y].checkerColor = BLACK;
                board[x + 1][y].checkerColor = BLACK;
                board[x + 2][y].checkerColor = BLACK;
                board[x + 3][y].checkerColor = BLACK;
                board[x + 4][y].checkerColor = BLACK;
                return true;
            }
        }
    }
    // Check vertical
    for (int x = 0; x < boardWidth; x++)
    {
        for (int y = 0; y < boardHeight - 4; y++)
        {
            if (board[x][y].isSlotTaken &&
                board[x][y + 1].isSlotTaken &&
                board[x][y + 2].isSlotTaken &&
                board[x][y + 3].isSlotTaken &&
                board[x][y + 4].isSlotTaken &&
                board[x][y].isPlayer1 == board[x][y + 1].isPlayer1 &&
                board[x][y].isPlayer1 == board[x][y + 2].isPlayer1 &&
                board[x][y].isPlayer1 == board[x][y + 3].isPlayer1 &&
                board[x][y].isPlayer1 == board[x][y + 4].isPlayer1)
            {
                board[x][y].checkerColor = BLACK;
                board[x][y + 1].checkerColor = BLACK;
                board[x][y + 2].checkerColor = BLACK;
                board[x][y + 3].checkerColor = BLACK;
                board[x][y + 4].checkerColor = BLACK;
                return true;
            }
        }
    }
    // Check diagonals (bottom-left to top-right)
    for (int x = 0; x < boardWidth - 4; x++)
    {
        for (int y = 4; y < boardHeight; y++)
        {
            if (board[x][y].isSlotTaken &&
                board[x + 1][y - 1].isSlotTaken &&
                board[x + 2][y - 2].isSlotTaken &&
                board[x + 3][y - 3].isSlotTaken &&
                board[x + 4][y - 4].isSlotTaken &&
                board[x][y].isPlayer1 == board[x + 1][y - 1].isPlayer1 &&
                board[x][y].isPlayer1 == board[x + 2][y - 2].isPlayer1 &&
                board[x][y].isPlayer1 == board[x + 3][y - 3].isPlayer1 &&
                board[x][y].isPlayer1 == board[x + 4][y - 4].isPlayer1)
            {
                board[x][y].checkerColor = BLACK;
                board[x + 1][y - 1].checkerColor = BLACK;
                board[x + 2][y - 2].checkerColor = BLACK;
                board[x + 3][y - 3].checkerColor = BLACK;
                board[x + 4][y - 4].checkerColor = BLACK;
                return true;
            }
        }
    }
    // Check diagonals (top-left to bottom-right)
    for (int x = 0; x < boardWidth - 4; x++)
    {
        for (int y = 0; y < boardHeight - 4; y++)
        {
            if (board[x][y].isSlotTaken &&
                board[x + 1][y + 1].isSlotTaken &&
                board[x + 2][y + 2].isSlotTaken &&
                board[x + 3][y + 3].isSlotTaken &&
                board[x + 4][y + 4].isSlotTaken &&
                board[x][y].isPlayer1 == board[x + 1][y + 1].isPlayer1 &&
                board[x][y].isPlayer1 == board[x + 2][y + 2].isPlayer1 &&
                board[x][y].isPlayer1 == board[x + 3][y + 3].isPlayer1 &&
                board[x][y].isPlayer1 == board[x + 4][y + 4].isPlayer1)
            {
                board[x][y].checkerColor = BLACK;
                board[x + 1][y + 1].checkerColor = BLACK;
                board[x + 2][y + 2].checkerColor = BLACK;
                board[x + 3][y + 3].checkerColor = BLACK;
                board[x + 4][y + 4].checkerColor = BLACK;
                return true;
            }
        }
    }
    return false;
}

// code implemented from the raylib demo
typedef enum GameScreen
{
    LOGO = 0,
    TITLE,
    INSTRUCTIONS,
    PVP,
    PVPColorSelect,
    AIColorSelect,
    AIGAMEPLAY,
} GameScreen;

int main()
{
    const int screenWidth = 1500;
    const int screenHeight = 700;

    InitWindow(screenWidth, screenHeight, "Connect 5 and Survive");
    SetTargetFPS(60);

    GameScreen currentScreen = LOGO;
    int framesCounter = 0;
    int depthValue = 0;
    Color p1Color = YELLOW;
    Color p2Color = RED;
    Color aiColor = WHITE;
    bool p1Selected = false;
    bool p2Selected = false;
    bool aiSelected = false;

    // ensure board empty at start
    for (int c = 0; c < boardWidth; c++)
    {
        for (int r = 0; r < boardHeight; r++)
        {
            board[c][r] = Dot();
        }
    }

    while (!WindowShouldClose()) // Detect window close button or ESC key
    {
        // This code updates the screen
        switch (currentScreen)
        {
        case LOGO:
        {
            framesCounter++;
            if (framesCounter > 120)
            {
                currentScreen = TITLE;
            }
        }
        break;
        case TITLE:
        {
            if (IsKeyPressed(KEY_ENTER) || IsGestureDetected(GESTURE_TAP))
            {
                currentScreen = INSTRUCTIONS;
            }
        }
        break;
        case INSTRUCTIONS:
        {
            if (IsKeyPressed(KEY_ONE))
            {
                depthValue = 1;
                currentScreen = AIColorSelect;
            }
            else if (IsKeyPressed(KEY_TWO))
            {
                depthValue = 2;
                currentScreen = AIColorSelect;
            }
            else if (IsKeyPressed(KEY_THREE))
            {
                depthValue = 3;
                currentScreen = AIColorSelect;
            }
            else if (IsKeyPressed(KEY_FOUR))
            {
                currentScreen = PVPColorSelect;
            }
        }
        break;
        case PVPColorSelect:
        {
            if (!p1Selected)
            {
                if (IsKeyPressed(KEY_ONE))
                {
                    p1Color = RED;
                }
                else if (IsKeyPressed(KEY_TWO))
                {
                    p1Color = ORANGE;
                }
                else if (IsKeyPressed(KEY_THREE))
                {
                    p1Color = YELLOW;
                }
                else if (IsKeyPressed(KEY_FOUR))
                {
                    p1Color = GREEN;
                }
                else if (IsKeyPressed(KEY_FIVE))
                {
                    p1Color = PURPLE;
                }

                if (IsKeyPressed(KEY_ENTER))
                {
                    p1Selected = true;
                }
            }

            else if (!p2Selected)
            {
                if (IsKeyPressed(KEY_ONE))
                {
                    p2Color = RED;
                }
                else if (IsKeyPressed(KEY_TWO))
                {
                    p2Color = ORANGE;
                }
                else if (IsKeyPressed(KEY_THREE))
                {
                    p2Color = YELLOW;
                }
                else if (IsKeyPressed(KEY_FOUR))
                {
                    p2Color = GREEN;
                }
                else if (IsKeyPressed(KEY_FIVE))
                {
                    p2Color = PURPLE;
                }

                if (IsKeyPressed(KEY_ENTER))
                {
                    p2Selected = true;
                    currentScreen = PVP;
                }
            }
        }
        break;
        case AIColorSelect:
        {
            if (!aiSelected)
            {
                if (IsKeyPressed(KEY_ONE))
                {
                    aiColor = ORANGE;
                }
                else if (IsKeyPressed(KEY_TWO))
                {
                    aiColor = YELLOW;
                }
                else if (IsKeyPressed(KEY_THREE))
                {
                    aiColor = GREEN;
                }
                else if (IsKeyPressed(KEY_FOUR))
                {
                    aiColor = PURPLE;
                }

                if (IsKeyPressed(KEY_ENTER))
                {
                    aiSelected = true;
                    currentScreen = AIGAMEPLAY;
                }
            }
        }
        break;
        case PVP:
        {
            if (!isGameOver)
            {
                int column = -1;
                int powerup = -1;
                if (currentPlayer == 1)
                {
                    if (IsKeyPressed(KEY_Q) && !playerPower1Used)
                    {
                        playerPower1Active = !playerPower1Active;
                    }
                    else if (IsKeyPressed(KEY_W) && !playerPower2Used)
                    {
                        playerPower2Active = !playerPower2Active;
                    }
                    else if (IsKeyPressed(KEY_E) && !playerPower3Used)
                    {
                        playerPower3Active = !playerPower3Active;
                    }
                    else if (IsKeyPressed(KEY_R) && !playerPower4Used)
                    {
                        playerPower4Active = !playerPower4Active;
                    }

                    if (IsKeyPressed(KEY_ONE))
                    {
                        column = 0;
                    }
                    else if (IsKeyPressed(KEY_TWO))
                    {
                        column = 1;
                    }
                    else if (IsKeyPressed(KEY_THREE))
                    {
                        column = 2;
                    }
                    else if (IsKeyPressed(KEY_FOUR))
                    {
                        column = 3;
                    }
                    else if (IsKeyPressed(KEY_FIVE))
                    {
                        column = 4;
                    }
                    else if (IsKeyPressed(KEY_SIX))
                    {
                        column = 5;
                    }
                    else if (IsKeyPressed(KEY_SEVEN))
                    {
                        column = 6;
                    }
                    else if (IsKeyPressed(KEY_EIGHT))
                    {
                        column = 7;
                    }

                    if (column != -1)
                    {
                        if (playerPower1Active && !playerPower1Used)
                        {
                            powerup = 1;
                        }
                        else if (playerPower2Active && !playerPower2Used)
                        {
                            powerup = 2;
                        }
                        else if (playerPower3Active && !playerPower3Used)
                        {
                            powerup = 3;
                        }
                        else if (playerPower4Active && !playerPower4Used)
                        {
                            powerup = 4;
                        }
                        else
                        {
                            powerup = 0;
                        }

                        if (makeMove(column, powerup, true, p1Color))
                        {
                            if (powerup == 1)
                            {
                                playerPower1Used = true;
                                playerPower1Active = false;
                            }
                            if (powerup == 2)
                            {
                                playerPower2Used = true;
                                playerPower2Active = false;
                            }
                            if (powerup == 3)
                            {
                                playerPower3Used = true;
                                playerPower3Active = false;
                            }
                            if (powerup == 4)
                            {
                                playerPower4Used = true;
                                playerPower4Active = false;
                            }

                            if (checkWin())
                            {
                                isGameOver = true;
                            }
                            else
                            {
                                switchPlayer();
                            }
                        }
                    }
                }
                else
                {
                    if (IsKeyPressed(KEY_A) && !player2Power1Used)
                    {
                        player2Power1Active = !player2Power1Active;
                    }
                    if (IsKeyPressed(KEY_S) && !player2Power2Used)
                    {
                        player2Power2Active = !player2Power2Active;
                    }
                    if (IsKeyPressed(KEY_D) && !player2Power3Used)
                    {
                        player2Power3Active = !player2Power3Active;
                    }
                    if (IsKeyPressed(KEY_F) && !player2Power4Used)
                    {
                        player2Power4Active = !player2Power4Active;
                    }

                    // Moves for player 2
                    if (IsKeyPressed(KEY_Z))
                    {
                        column = 0;
                    }
                    else if (IsKeyPressed(KEY_X))
                    {
                        column = 1;
                    }
                    else if (IsKeyPressed(KEY_C))
                    {
                        column = 2;
                    }
                    else if (IsKeyPressed(KEY_V))
                    {
                        column = 3;
                    }
                    else if (IsKeyPressed(KEY_B))
                    {
                        column = 4;
                    }
                    else if (IsKeyPressed(KEY_N))
                    {
                        column = 5;
                    }
                    else if (IsKeyPressed(KEY_M))
                    {
                        column = 6;
                    }
                    else if (IsKeyPressed(KEY_COMMA))
                    {
                        column = 7;
                    }

                    if (column != -1)
                    {
                        if (player2Power1Active && !player2Power1Used)
                        {
                            powerup = 1;
                        }
                        else if (player2Power2Active && !player2Power2Used)
                        {
                            powerup = 2;
                        }
                        else if (player2Power3Active && !player2Power3Used)
                        {
                            powerup = 3;
                        }
                        else if (player2Power4Active && !player2Power4Used)
                        {
                            powerup = 4;
                        }
                        else
                        {
                            powerup = 0;
                        }

                        if (makeMove(column, powerup, false, p2Color))
                        {

                            if (powerup == 1)
                            {
                                player2Power1Used = true;
                                player2Power1Active = false;
                            }
                            if (powerup == 2)
                            {
                                player2Power2Used = true;
                                player2Power2Active = false;
                            }
                            if (powerup == 3)
                            {
                                player2Power3Used = true;
                                player2Power3Active = false;
                            }
                            if (powerup == 4)
                            {
                                player2Power4Used = true;
                                player2Power4Active = false;
                            }

                            if (checkWin())
                            {
                                isGameOver = true;
                            }
                            else
                            {
                                switchPlayer();
                            }
                        }
                    }
                }
            }
        }
        break;
        case AIGAMEPLAY:
        {
            if (!isGameOver)
            {
                int column = -1;
                int powerup = -1;
                if (IsKeyPressed(KEY_Q) && !playerPower1Used)
                {
                    playerPower1Active = !playerPower1Active;
                    powerup = 1;
                    cout << "Powerup 1 selected" << endl;
                }
                if (IsKeyPressed(KEY_W) && !playerPower2Used)
                {
                    playerPower2Active = !playerPower2Active;
                    powerup = 2;
                    cout << "Powerup 2 selected" << endl;
                }
                if (IsKeyPressed(KEY_E) && !playerPower3Used)
                {
                    playerPower3Active = !playerPower3Active;
                    powerup = 3;
                    cout << "Powerup 3 selected" << endl;
                }
                if (IsKeyPressed(KEY_R) && !playerPower4Used)
                {
                    playerPower4Active = !playerPower4Active;
                    powerup = 4;
                    cout << "Powerup 4 selected" << endl;
                }

                if (IsKeyPressed(KEY_ONE))
                {
                    column = 0;
                }
                else if (IsKeyPressed(KEY_TWO))
                {
                    column = 1;
                }
                else if (IsKeyPressed(KEY_THREE))
                {
                    column = 2;
                }
                else if (IsKeyPressed(KEY_FOUR))
                {
                    column = 3;
                }
                else if (IsKeyPressed(KEY_FIVE))
                {
                    column = 4;
                }
                else if (IsKeyPressed(KEY_SIX))
                {
                    column = 5;
                }
                else if (IsKeyPressed(KEY_SEVEN))
                {
                    column = 6;
                }
                else if (IsKeyPressed(KEY_EIGHT))
                {
                    column = 7;
                }

                if (currentPlayer == 1 && column != -1)
                {
                    int usePower = 0;
                    if (playerPower1Active && !playerPower1Used)
                    {
                        usePower = 1;
                    }
                    if (playerPower2Active && !playerPower2Used)
                    {
                        usePower = 2;
                    }
                    if (playerPower3Active && !playerPower3Used)
                    {
                        usePower = 3;
                    }
                    if (playerPower4Active && !playerPower4Used)
                    {
                        usePower = 4;
                    }
                    if (makeMove(column, usePower, true, aiColor))
                    {
                        if (usePower == 1)
                        {
                            playerPower1Used = true;
                            playerPower1Active = false;
                        }
                        if (usePower == 2)
                        {
                            playerPower2Used = true;
                            playerPower2Active = false;
                        }
                        if (usePower == 3)
                        {
                            playerPower3Used = true;
                            playerPower3Active = false;
                        }
                        if (usePower == 4)
                        {
                            playerPower4Used = true;
                            playerPower4Active = false;
                        }
                        if (checkWin())
                        {
                            isGameOver = true;
                        }
                        else
                        {
                            switchPlayer();
                        }
                    }
                }

                else if (currentPlayer == 0)
                {
                    int powerToUse = 0;
                    if (!aiPower1Used)
                    {
                        powerToUse = 1;
                    }
                    else if (!aiPower2Used)
                    {
                        powerToUse = 2;
                    }
                    else if (!aiPower3Used)
                    {
                        powerToUse = 3;
                    }
                    else if (!aiPower4Used)
                    {
                        powerToUse = 4;
                    }

                    int aiColumn = getBestMove(depthValue, powerToUse);

                    if (aiColumn >= 0)
                    {
                        makeMove(aiColumn, powerToUse, false, RED);
                        if (powerToUse == 1)
                        {
                            aiPower1Used = true;
                        }
                        if (powerToUse == 2)
                        {
                            aiPower2Used = true;
                        }
                        if (powerToUse == 3)
                        {
                            aiPower3Used = true;
                        }
                        if (powerToUse == 4)
                        {
                            aiPower4Used = true;
                        }
                        if (checkWin())
                        {
                            isGameOver = true;
                        }
                        else
                        {
                            switchPlayer();
                        }
                    }
                }
            }
        }
        break;

        default:
            break;
        }

        // This Code draws the switches in the game
        BeginDrawing();
        ClearBackground(WHITE);
        switch (currentScreen)
        {
        case LOGO:
        {
            DrawText("Wait", 500, 650, 30, GOLD);
        }
        break;
        case TITLE:
        {
            ClearBackground(PURPLE);
            DrawText("Welcome To", 600, 250, 50, BLACK);
            DrawText("CONNECT 5 AND SURVIVE", 400, 300, 50, BLACK);
            DrawText("Tap Anywhere to Play", 520, 350, 40, BLACK);
        }
        break;
        case INSTRUCTIONS:
        {
            ClearBackground(PURPLE);
            DrawText("This is Connect 5 and Survive, a brand new take on Connect 4,", 200, 50, 30, BLACK);
            DrawText("and nothing like you've seen before.", 200, 100, 30, BLACK);
            DrawText("There will be a 7x8 board.  Your goal is to connect 5 checkers in a row.", 200, 150, 30, BLACK);
            DrawText("Tap the keys 1-7 to drop a checker in the correlating column.", 200, 200, 30, BLACK);
            DrawText("Click the cooresponding key to select difficulty and begin your battle", 200, 250, 30, BLACK);
            DrawText("...hopefully you survive", 200, 300, 30, BLACK);

            DrawText("Easy: 1       Hard: 2       Expert: 3       PVP: 4", 200, 400, 30, RED);

            DrawText("Double: Places two checkers in the selected row", 200, 500, 30, BLACK);
            DrawText("Magnet: Pulls checkers to the left and right towards it", 200, 550, 30, BLACK);
            DrawText("Bomb: Destroys checkers to the left and right of it", 200, 600, 30, BLACK);
            DrawText("Drill: Drills and destroys the checker beneath the placed checker", 200, 650, 30, BLACK);
        }
        break;
        case PVPColorSelect:
        {
            ClearBackground(PURPLE);
            DrawText("During PVP, Player 1 will use keys 1-7, Player 2 will use keys z-m", 200, 100, 30, BLACK);
            DrawText("Player 1, choose your color and select Enter", 200, 150, 30, BLACK);

            DrawText("Red: 1       Orange: 2       Yellow: 3       Green: 4       Purple: 5", 200, 300, 30, BLACK);

            if (p1Selected)
            {
                DrawText("Then Player 2, choose your color and select Enter", 200, 200, 30, BLACK);
            }
        }
        break;
        case AIColorSelect:
        {
            ClearBackground(PURPLE);
            DrawText("Choose your color and select Enter!", 200, 100, 30, BLACK);

            DrawText("Orange: 1       Yellow: 2       Green: 3       Purple: 4", 200, 300, 30, BLACK);
        }
        break;
        case PVP:
        {
            DrawText("Player v Player", 100, 10, 30, BLACK);
            ClearBackground(BLUE);
            printBoard();

            DrawText("Player Power Ups", 80, 300, 30, p1Color);
            DrawLineEx({80, 340}, {355, 340}, 3, p1Color);
            if (!playerPower1Used)
            {
                DrawText("Double Available (Q)", 80, 350, 30, GOLD);
            }
            if (!playerPower2Used)
            {
                DrawText("Magnet Available (W)", 80, 380, 30, GOLD);
            }
            if (!playerPower3Used)
            {
                DrawText("Bomb Available (E)", 80, 410, 30, GOLD);
            }
            if (!playerPower4Used)
            {
                DrawText("Drill Available (R)", 80, 440, 30, GOLD);
            }

            DrawText("Player 2 Power Ups", 1150, 300, 30, p2Color);
            DrawLineEx({1150, 340}, {1425, 340}, 3, p2Color);
            if (!player2Power1Used)
            {
                DrawText("Double Available (A)", 1150, 350, 30, GOLD);
            }
            if (!player2Power2Used)
            {
                DrawText("Magnet Available (S)", 1150, 380, 30, GOLD);
            }
            if (!player2Power3Used)
            {
                DrawText("Bomb Available (D)", 1150, 410, 30, GOLD);
            }
            if (!player2Power4Used)
            {
                DrawText("Drill Available (F)", 1150, 440, 30, GOLD);
            }

            if (playerPower1Active && !playerPower1Used)
            {
                DrawText("DOUBLE POWERUP ACTIVE!", 100, 650, 30, GOLD);
            }
            if (playerPower2Active && !playerPower2Used)
            {
                DrawText("MAGNET POWERUP ACTIVE!", 100, 650, 30, GOLD);
            }
            if (playerPower3Active && !playerPower3Used)
            {
                DrawText("BOMB POWERUP ACTIVE!", 100, 650, 30, GOLD);
            }
            if (playerPower4Active && !playerPower4Used)
            {
                DrawText("DRILL POWERUP ACTIVE!", 100, 650, 30, GOLD);
            }

            if (isGameOver)
            {
                const char *msg = (currentPlayer == 1 ? "Player 1 wins!" : "Player 2 wins!");
                const char *msg2 = (currentPlayer == 1 ? "Nicely Done!" : "Better Luck Next Time!");
                DrawText(msg, 1115, 100, 30, GOLD);
                DrawText(msg2, 1115, 150, 30, GOLD);
            }
        }
        break;
        case AIGAMEPLAY:
        {
            if (depthValue == 1)
            {
                DrawText("Easy Game", 200, 100, 30, BLACK);
            }
            else if (depthValue == 2)
            {
                DrawText("Hard Game", 200, 100, 30, BLACK);
            }
            else if (depthValue == 3)
            {
                DrawText("Expert Game", 200, 100, 30, BLACK);
            }
            ClearBackground(BLUE);
            printBoard();

            DrawText("Player Power Ups", 80, 300, 30, aiColor);
            DrawLineEx({80, 340}, {355, 340}, 3, aiColor);
            if (!playerPower1Used)
            {
                DrawText("Double Available (Q)", 80, 350, 30, GOLD);
            }
            if (!playerPower2Used)
            {
                DrawText("Magnet Available (W)", 80, 380, 30, GOLD);
            }
            if (!playerPower3Used)
            {
                DrawText("Bomb Available (E)", 80, 410, 30, GOLD);
            }
            if (!playerPower4Used)
            {
                DrawText("Drill Available (R)", 80, 440, 30, GOLD);
            }

            DrawText("AI Power Ups", 1150, 300, 30, RED);
            DrawLineEx({1150, 340}, {1425, 340}, 3, RED);

            if (!aiPower1Used)
            {
                DrawText("Double Available", 1150, 350, 30, GOLD);
            }
            if (!aiPower2Used)
            {
                DrawText("Magnet Available", 1150, 380, 30, GOLD);
            }
            if (!aiPower3Used)
            {
                DrawText("Bomb Available", 1150, 410, 30, GOLD);
            }
            if (!aiPower4Used)
            {
                DrawText("Drill Available", 1150, 440, 30, GOLD);
            }

            if (playerPower1Active && !playerPower1Used)
            {
                DrawText("DOUBLE POWERUP ACTIVE!", 100, 650, 30, GOLD);
            }
            if (playerPower2Active && !playerPower2Used)
            {
                DrawText("MAGNET POWERUP ACTIVE!", 100, 650, 30, GOLD);
            }
            if (playerPower3Active && !playerPower3Used)
            {
                DrawText("BOMB POWERUP ACTIVE!", 100, 650, 30, GOLD);
            }
            if (playerPower4Active && !playerPower4Used)
            {
                DrawText("DRILL POWERUP ACTIVE!", 100, 650, 30, GOLD);
            }
            if (isGameOver)
            {
                const char *msg = (currentPlayer == 1 ? "Player wins!" : "AI wins!");
                const char *msg2 = (currentPlayer == 1 ? "Nicely Done!" : "Better Luck Next Time!");
                DrawText(msg, 1115, 100, 30, GOLD);
                DrawText(msg2, 1115, 150, 30, GOLD);
            }
        }
        break;
        default:
            break;
        }
        EndDrawing();
    }
    //--------------------------------------------------------------------------------------
    CloseWindow(); // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    return 0;
}