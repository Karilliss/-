#pragma once
#include <vector>
#include <set>

enum class Difficulty {
    EASY,
    MEDIUM,
    HARD
};

enum class Orientation {
    HORIZONTAL,
    VERTICAL
};

struct Domino {
    int value1;
    int value2;
    int sum;
    bool placed;
    int row;
    int col;
    Orientation orientation;
    int id;

    Domino(int v1, int v2) : value1(v1), value2(v2), sum(v1 + v2), placed(false),
        row(-1), col(-1), orientation(Orientation::HORIZONTAL), id(-1) {
    }
};

struct GameState {
    static const int GRID_SIZE = 8;
    static const int TOTAL_DOMINOES = 15;

    int gameGrid[GRID_SIZE][GRID_SIZE];
    int dominoGrid[GRID_SIZE][GRID_SIZE];
    int solutionGrid[GRID_SIZE][GRID_SIZE];

    std::vector<Domino> availableDominoes;
    std::vector<Domino> placedDominoes;
    std::vector<Domino> solutionDominoes;

    int selectedDomino;
    Orientation currentOrientation;
    DWORD gameStartTime;
    int hintsUsed;

    GameState() : selectedDomino(-1), currentOrientation(Orientation::HORIZONTAL),
        gameStartTime(0), hintsUsed(0) {
        Reset();
    }

    void Reset() {
        for (int i = 0; i < GRID_SIZE; i++) {
            for (int j = 0; j < GRID_SIZE; j++) {
                gameGrid[i][j] = 0;
                dominoGrid[i][j] = -1;
                solutionGrid[i][j] = -1;
            }
        }
        placedDominoes.clear();
        solutionDominoes.clear();
        selectedDomino = -1;
        hintsUsed = 0;
        gameStartTime = GetTickCount();
    }
};