#pragma once
#include "GameState.h"  // Make sure this includes your GameState, Domino, Difficulty, Orientation definitions

class GameLogic
{
private:
    GameState* m_gameState;

public:
    // Constructor
    GameLogic();

    // Destructor
    ~GameLogic();

    // Game state management
    void SetGameState(GameState* gameState);
    GameState* GetGameState() const;

    // Game control methods
    bool GeneratePuzzle(Difficulty difficulty);
    void ResetGame();
    bool PlaceDomino(int row, int col);
    bool RemoveDomino(int row, int col);
    bool CheckWin() const;
    void GetHint();
    void AutoSolve();

    // Validation methods
    bool IsValidPlacement(int row, int col, const Domino& domino, Orientation orientation) const;
    bool CheckDominoFits(int row, int col, Orientation orientation) const;

private:
    // Helper methods
    void InitializeGrid();
    void GenerateDominoes();
    void ShuffleDominoes();
    bool SolvePuzzle();
    void ClearPlacedDominoes();
    int GetNextAvailableDominoId();

    // Puzzle generation helpers
    void CreateEasyPuzzle();
    void CreateMediumPuzzle();
    void CreateHardPuzzle();

    // Solving algorithms
    bool BacktrackSolve(int dominoIndex);
    bool CanPlaceDominoAt(int row, int col, const Domino& domino, Orientation orientation) const;
};
