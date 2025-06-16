#pragma once
#include <vector>

// Enumerations
enum class Difficulty
{
    EASY = 0,
    MEDIUM = 1,
    HARD = 2
};

enum class Orientation
{
    HORIZONTAL = 0,
    VERTICAL = 1
};

// Domino structure
struct Domino
{
    int value1;
    int value2;
    Orientation orientation;

    Domino() : value1(0), value2(0), orientation(Orientation::HORIZONTAL) {}
    Domino(int v1, int v2, Orientation orient = Orientation::HORIZONTAL)
        : value1(v1), value2(v2), orientation(orient) {
    }
};

// Placed domino structure (for tracking placed dominoes)
struct PlacedDomino
{
    Domino domino;
    int row;
    int col;
    Orientation orientation;
    int id; // Unique identifier for this placed domino

    PlacedDomino() : row(0), col(0), orientation(Orientation::HORIZONTAL), id(0) {}
};

// Main game state structure
class GameState
{
public:
    // Constants
    static const int GRID_SIZE = 8;
    static const int TOTAL_DOMINOES = 28; // Standard domino set (0-0 to 6-6)

    // Game grids
    int gameGrid[GRID_SIZE][GRID_SIZE];      // Numbers shown on the board (0 = empty)
    int dominoGrid[GRID_SIZE][GRID_SIZE];    // Domino placement (-1 = empty, >0 = domino ID)

    // Domino collections
    std::vector<Domino> availableDominoes;   // Dominoes that can be placed
    std::vector<PlacedDomino> placedDominoes; // Dominoes that have been placed

    // Game state variables
    int selectedDomino;                      // Index of currently selected domino (-1 = none)
    Orientation currentOrientation;          // Current orientation for placing dominoes
    Difficulty currentDifficulty;            // Current game difficulty

    // Game statistics
    DWORD gameStartTime;                     // Game start time (from GetTickCount())
    int hintsUsed;                          // Number of hints used
    bool gameWon;                           // Whether the game has been won

    // Constructor
    GameState()
    {
        // Initialize grids
        for (int i = 0; i < GRID_SIZE; i++)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                gameGrid[i][j] = 0;
                dominoGrid[i][j] = -1;
            }
        }

        // Initialize state
        selectedDomino = -1;
        currentOrientation = Orientation::HORIZONTAL;
        currentDifficulty = Difficulty::EASY;
        gameStartTime = GetTickCount();
        hintsUsed = 0;
        gameWon = false;
    }

    // Reset the game state
    void Reset()
    {
        // Clear grids
        for (int i = 0; i < GRID_SIZE; i++)
        {
            for (int j = 0; j < GRID_SIZE; j++)
            {
                gameGrid[i][j] = 0;
                dominoGrid[i][j] = -1;
            }
        }

        // Clear collections
        availableDominoes.clear();
        placedDominoes.clear();

        // Reset state
        selectedDomino = -1;
        currentOrientation = Orientation::HORIZONTAL;
        gameStartTime = GetTickCount();
        hintsUsed = 0;
        gameWon = false;
    }
};
