#include "pch.h"
#include "GameLogic.h"
#include <algorithm>
#include <random>
#include <chrono>

GameLogic::GameLogic() : m_gameState(nullptr)
{
}

GameLogic::~GameLogic()
{
    // Don't delete m_gameState as we don't own it
}

void GameLogic::SetGameState(GameState* gameState)
{
    m_gameState = gameState;
}

GameState* GameLogic::GetGameState() const
{
    return m_gameState;
}

bool GameLogic::GeneratePuzzle(Difficulty difficulty)
{
    if (!m_gameState) return false;

    // Reset the game state
    ResetGame();

    // Initialize the grid and dominoes
    InitializeGrid();
    GenerateDominoes();

    // Create puzzle based on difficulty
    switch (difficulty)
    {
    case Difficulty::EASY:
        CreateEasyPuzzle();
        break;
    case Difficulty::MEDIUM:
        CreateMediumPuzzle();
        break;
    case Difficulty::HARD:
        CreateHardPuzzle();
        break;
    }

    // Record game start time
    m_gameState->gameStartTime = GetTickCount();
    m_gameState->currentDifficulty = difficulty;

    return true;
}

void GameLogic::ResetGame()
{
    if (!m_gameState) return;

    // Clear grids
    for (int i = 0; i < GameState::GRID_SIZE; i++)
    {
        for (int j = 0; j < GameState::GRID_SIZE; j++)
        {
            m_gameState->gameGrid[i][j] = 0;
            m_gameState->dominoGrid[i][j] = -1;
        }
    }

    // Clear collections
    m_gameState->availableDominoes.clear();
    m_gameState->placedDominoes.clear();

    // Reset game state
    m_gameState->selectedDomino = -1;
    m_gameState->currentOrientation = Orientation::HORIZONTAL;
    m_gameState->hintsUsed = 0;
    m_gameState->gameStartTime = GetTickCount();
}

bool GameLogic::PlaceDomino(int row, int col)
{
    if (!m_gameState || m_gameState->selectedDomino < 0 ||
        m_gameState->selectedDomino >= (int)m_gameState->availableDominoes.size())
        return false;

    const Domino& domino = m_gameState->availableDominoes[m_gameState->selectedDomino];

    if (!IsValidPlacement(row, col, domino, m_gameState->currentOrientation))
        return false;

    // Get unique domino ID
    int dominoId = GetNextAvailableDominoId();

    // Place the domino
    if (m_gameState->currentOrientation == Orientation::HORIZONTAL)
    {
        if (col + 1 >= GameState::GRID_SIZE) return false;

        m_gameState->dominoGrid[row][col] = dominoId;
        m_gameState->dominoGrid[row][col + 1] = dominoId;
    }
    else // VERTICAL
    {
        if (row + 1 >= GameState::GRID_SIZE) return false;

        m_gameState->dominoGrid[row][col] = dominoId;
        m_gameState->dominoGrid[row + 1][col] = dominoId;
    }

    // Create placed domino record
    PlacedDomino placedDomino;
    placedDomino.domino = domino;
    placedDomino.row = row;
    placedDomino.col = col;
    placedDomino.orientation = m_gameState->currentOrientation;
    placedDomino.id = dominoId;

    m_gameState->placedDominoes.push_back(placedDomino);

    // Remove from available dominoes
    m_gameState->availableDominoes.erase(m_gameState->availableDominoes.begin() + m_gameState->selectedDomino);
    m_gameState->selectedDomino = -1;

    return true;
}

bool GameLogic::RemoveDomino(int row, int col)
{
    if (!m_gameState) return false;

    int dominoId = m_gameState->dominoGrid[row][col];
    if (dominoId == -1) return false;

    // Find the placed domino
    auto it = std::find_if(m_gameState->placedDominoes.begin(), m_gameState->placedDominoes.end(),
        [dominoId](const PlacedDomino& pd) { return pd.id == dominoId; });

    if (it == m_gameState->placedDominoes.end()) return false;

    // Remove domino from grid
    for (int i = 0; i < GameState::GRID_SIZE; i++)
    {
        for (int j = 0; j < GameState::GRID_SIZE; j++)
        {
            if (m_gameState->dominoGrid[i][j] == dominoId)
            {
                m_gameState->dominoGrid[i][j] = -1;
            }
        }
    }

    // Add back to available dominoes
    m_gameState->availableDominoes.push_back(it->domino);

    // Remove from placed dominoes
    m_gameState->placedDominoes.erase(it);

    return true;
}

bool GameLogic::CheckWin() const
{
    if (!m_gameState) return false;

    // Check if all dominoes are placed
    return m_gameState->availableDominoes.empty();
}

void GameLogic::GetHint()
{
    if (!m_gameState) return;

    // Simple hint: try to find a valid placement for the first available domino
    if (!m_gameState->availableDominoes.empty())
    {
        const Domino& domino = m_gameState->availableDominoes[0];

        for (int row = 0; row < GameState::GRID_SIZE; row++)
        {
            for (int col = 0; col < GameState::GRID_SIZE; col++)
            {
                // Try horizontal placement
                if (IsValidPlacement(row, col, domino, Orientation::HORIZONTAL))
                {
                    m_gameState->selectedDomino = 0;
                    m_gameState->currentOrientation = Orientation::HORIZONTAL;
                    m_gameState->hintsUsed++;
                    return;
                }

                // Try vertical placement
                if (IsValidPlacement(row, col, domino, Orientation::VERTICAL))
                {
                    m_gameState->selectedDomino = 0;
                    m_gameState->currentOrientation = Orientation::VERTICAL;
                    m_gameState->hintsUsed++;
                    return;
                }
            }
        }
    }
}

void GameLogic::AutoSolve()
{
    if (!m_gameState) return;

    // Simple auto-solve: place dominoes one by one using backtracking
    SolvePuzzle();
}

bool GameLogic::IsValidPlacement(int row, int col, const Domino& domino, Orientation orientation) const
{
    if (!m_gameState) return false;

    // Check bounds
    if (orientation == Orientation::HORIZONTAL)
    {
        if (col + 1 >= GameState::GRID_SIZE) return false;

        // Check if cells are empty
        if (m_gameState->dominoGrid[row][col] != -1 || m_gameState->dominoGrid[row][col + 1] != -1)
            return false;

        // Check if domino values match game grid (if game grid has values)
        if (m_gameState->gameGrid[row][col] > 0 && m_gameState->gameGrid[row][col] != domino.value1)
            return false;
        if (m_gameState->gameGrid[row][col + 1] > 0 && m_gameState->gameGrid[row][col + 1] != domino.value2)
            return false;
    }
    else // VERTICAL
    {
        if (row + 1 >= GameState::GRID_SIZE) return false;

        // Check if cells are empty
        if (m_gameState->dominoGrid[row][col] != -1 || m_gameState->dominoGrid[row + 1][col] != -1)
            return false;

        // Check if domino values match game grid (if game grid has values)
        if (m_gameState->gameGrid[row][col] > 0 && m_gameState->gameGrid[row][col] != domino.value1)
            return false;
        if (m_gameState->gameGrid[row + 1][col] > 0 && m_gameState->gameGrid[row + 1][col] != domino.value2)
            return false;
    }

    return true;
}

bool GameLogic::CheckDominoFits(int row, int col, Orientation orientation) const
{
    if (!m_gameState) return false;

    if (orientation == Orientation::HORIZONTAL)
    {
        return (col + 1 < GameState::GRID_SIZE &&
            m_gameState->dominoGrid[row][col] == -1 &&
            m_gameState->dominoGrid[row][col + 1] == -1);
    }
    else
    {
        return (row + 1 < GameState::GRID_SIZE &&
            m_gameState->dominoGrid[row][col] == -1 &&
            m_gameState->dominoGrid[row + 1][col] == -1);
    }
}

void GameLogic::InitializeGrid()
{
    if (!m_gameState) return;

    // Initialize with empty values
    for (int i = 0; i < GameState::GRID_SIZE; i++)
    {
        for (int j = 0; j < GameState::GRID_SIZE; j++)
        {
            m_gameState->gameGrid[i][j] = 0;
            m_gameState->dominoGrid[i][j] = -1;
        }
    }
}

void GameLogic::GenerateDominoes()
{
    if (!m_gameState) return;

    m_gameState->availableDominoes.clear();

    // Generate standard domino set (0-0 to 6-6)
    for (int i = 0; i <= 6; i++)
    {
        for (int j = i; j <= 6; j++)
        {
            Domino domino;
            domino.value1 = i;
            domino.value2 = j;
            domino.orientation = Orientation::HORIZONTAL;
            m_gameState->availableDominoes.push_back(domino);
        }
    }

    ShuffleDominoes();
}

void GameLogic::ShuffleDominoes()
{
    if (!m_gameState) return;

    // Shuffle the dominoes
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(m_gameState->availableDominoes.begin(), m_gameState->availableDominoes.end(),
        std::default_random_engine(seed));
}

void GameLogic::CreateEasyPuzzle()
{
    // For easy puzzle, keep only first 10 dominoes
    if (m_gameState && m_gameState->availableDominoes.size() > 10)
    {
        m_gameState->availableDominoes.resize(10);
    }
}

void GameLogic::CreateMediumPuzzle()
{
    // For medium puzzle, keep first 15 dominoes
    if (m_gameState && m_gameState->availableDominoes.size() > 15)
    {
        m_gameState->availableDominoes.resize(15);
    }
}

void GameLogic::CreateHardPuzzle()
{
    // For hard puzzle, keep first 20 dominoes
    if (m_gameState && m_gameState->availableDominoes.size() > 20)
    {
        m_gameState->availableDominoes.resize(20);
    }
}

bool GameLogic::SolvePuzzle()
{
    if (!m_gameState) return false;
    return BacktrackSolve(0);
}

bool GameLogic::BacktrackSolve(int dominoIndex)
{
    if (!m_gameState) return false;

    if (dominoIndex >= (int)m_gameState->availableDominoes.size())
        return true; // All dominoes placed

    const Domino& domino = m_gameState->availableDominoes[dominoIndex];

    for (int row = 0; row < GameState::GRID_SIZE; row++)
    {
        for (int col = 0; col < GameState::GRID_SIZE; col++)
        {
            // Try horizontal placement
            if (CanPlaceDominoAt(row, col, domino, Orientation::HORIZONTAL))
            {
                // Place domino
                int dominoId = GetNextAvailableDominoId();
                m_gameState->dominoGrid[row][col] = dominoId;
                m_gameState->dominoGrid[row][col + 1] = dominoId;

                if (BacktrackSolve(dominoIndex + 1))
                    return true;

                // Remove domino
                m_gameState->dominoGrid[row][col] = -1;
                m_gameState->dominoGrid[row][col + 1] = -1;
            }

            // Try vertical placement
            if (CanPlaceDominoAt(row, col, domino, Orientation::VERTICAL))
            {
                // Place domino
                int dominoId = GetNextAvailableDominoId();
                m_gameState->dominoGrid[row][col] = dominoId;
                m_gameState->dominoGrid[row + 1][col] = dominoId;

                if (BacktrackSolve(dominoIndex + 1))
                    return true;

                // Remove domino
                m_gameState->dominoGrid[row][col] = -1;
                m_gameState->dominoGrid[row + 1][col] = -1;
            }
        }
    }

    return false;
}

bool GameLogic::CanPlaceDominoAt(int row, int col, const Domino& domino, Orientation orientation) const
{
    return IsValidPlacement(row, col, domino, orientation);
}

int GameLogic::GetNextAvailableDominoId()
{
    if (!m_gameState) return 0;

    static int nextId = 0;
    return ++nextId;
}
