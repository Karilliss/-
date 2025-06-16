#pragma once
#include <vector>
#include <set>
#include <map>
#include <random>
#include <algorithm>
#include <memory>
#include <functional>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <unordered_set>

// Disable Windows min/max macros if they're defined
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

enum class Difficulty {
    EASY = 0,
    MEDIUM = 1,
    HARD = 2
};

enum class Orientation {
    HORIZONTAL,
    VERTICAL
};

struct Position {
    int row, col;

    Position(int r = -1, int c = -1) : row(r), col(c) {}

    bool operator==(const Position& other) const {
        return row == other.row && col == other.col;
    }

    bool operator<(const Position& other) const {
        return row < other.row || (row == other.row && col < other.col);
    }

    bool isValid() const {
        return row >= 0 && col >= 0;
    }

    bool isValidForGrid(int gridSize) const {
        return row >= 0 && row < gridSize && col >= 0 && col < gridSize;
    }
};

struct DominoPiece {
    int value1, value2;
    int sum;
    Position position;
    Orientation orientation;
    bool isPlaced;

    DominoPiece(int v1, int v2) : value1(v1), value2(v2), sum(v1 + v2),
        position(-1, -1), orientation(Orientation::HORIZONTAL),
        isPlaced(false) {
    }

    bool operator==(const DominoPiece& other) const {
        return (value1 == other.value1 && value2 == other.value2) ||
            (value1 == other.value2 && value2 == other.value1);
    }

    bool operator<(const DominoPiece& other) const {
        auto thisCanon = getCanonicalForm();
        auto otherCanon = other.getCanonicalForm();
        if (thisCanon.first != otherCanon.first) return thisCanon.first < otherCanon.first;
        return thisCanon.second < otherCanon.second;
    }

    std::pair<int, int> getCanonicalForm() const {
        // Using ternary operators instead of std::min/max to avoid conflicts
        int smaller = (value1 < value2) ? value1 : value2;
        int larger = (value1 < value2) ? value2 : value1;
        return { smaller, larger };
    }
};

class DominoGame {
private:
    // Constants
    static const int GRID_SIZE = 8;
    static const int MAX_GENERATION_ATTEMPTS = 100;
    static const int MAX_HINTS_ALLOWED = 3;

    // Game state
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<int>> dominoGrid;
    std::vector<DominoPiece> availableDominoes;
    std::vector<DominoPiece> placedDominoes;

    // Game settings
    Difficulty currentDifficulty;
    bool gameCompleted;
    int hintsUsed;
    std::chrono::steady_clock::time_point gameStartTime;

    // Solution storage
    std::vector<std::vector<int>> solutionGrid;
    std::vector<DominoPiece> solutionDominoes;
    bool hasSolution;

    // Random number generator
    std::mt19937 rng;

    // Cache for performance optimization
    mutable std::map<Position, int> constraintCache;
    mutable bool cacheValid;

public:
    DominoGame() : rng(std::chrono::steady_clock::now().time_since_epoch().count()) {
        initializeGame();
    }

    void initializeGame() {
        grid.assign(GRID_SIZE, std::vector<int>(GRID_SIZE, 0));
        dominoGrid.assign(GRID_SIZE, std::vector<int>(GRID_SIZE, -1));
        placedDominoes.clear();
        gameCompleted = false;
        hintsUsed = 0;
        hasSolution = false;
        cacheValid = false;
        constraintCache.clear();
        generateAvailableDominoes();
    }

    void generateAvailableDominoes() {
        availableDominoes.clear();
        std::set<DominoPiece> uniquePieces;

        for (int v1 = 1; v1 <= 5; ++v1) {
            for (int v2 = v1 + 1; v2 <= 6; ++v2) {
                uniquePieces.emplace(v1, v2);
            }
        }

        availableDominoes.assign(uniquePieces.begin(), uniquePieces.end());
        std::sort(availableDominoes.begin(), availableDominoes.end());
    }

    bool generateNewGame(Difficulty difficulty) {
        currentDifficulty = difficulty;
        gameStartTime = std::chrono::steady_clock::now();

        for (int attempt = 0; attempt < MAX_GENERATION_ATTEMPTS; ++attempt) {
            initializeGame();

            if (generateSolution()) {
                generateConstraintGrid();
                applyDifficultySettings();
                return true;
            }
        }

        std::cerr << "Warning: Failed to generate complex puzzle, creating simplified version\n";
        return generateSimplifiedPuzzle(difficulty);
    }

private:
    bool generateSimplifiedPuzzle(Difficulty difficulty) {
        initializeGame();

        std::vector<DominoPiece> shuffledDominoes = availableDominoes;
        std::shuffle(shuffledDominoes.begin(), shuffledDominoes.end(), rng);

        solutionGrid.assign(GRID_SIZE, std::vector<int>(GRID_SIZE, -1));
        solutionDominoes.clear();

        int dominoIndex = 0;
        for (int row = 0; row < GRID_SIZE && dominoIndex < static_cast<int>(shuffledDominoes.size()); row += 2) {
            for (int col = 0; col < GRID_SIZE - 1 && dominoIndex < static_cast<int>(shuffledDominoes.size()); col += 3) {
                if (canPlaceDominoInSolution(shuffledDominoes[dominoIndex], Position(row, col), Orientation::HORIZONTAL)) {
                    placeDominoInSolution(shuffledDominoes[dominoIndex], Position(row, col), Orientation::HORIZONTAL, dominoIndex);
                    dominoIndex++;
                }
            }
        }

        for (int col = 0; col < GRID_SIZE && dominoIndex < static_cast<int>(shuffledDominoes.size()); col += 3) {
            for (int row = 0; row < GRID_SIZE - 1 && dominoIndex < static_cast<int>(shuffledDominoes.size()); row += 3) {
                if (canPlaceDominoInSolution(shuffledDominoes[dominoIndex], Position(row, col), Orientation::VERTICAL)) {
                    placeDominoInSolution(shuffledDominoes[dominoIndex], Position(row, col), Orientation::VERTICAL, dominoIndex);
                    dominoIndex++;
                }
            }
        }

        if (dominoIndex > 10) {
            hasSolution = true;
            generateConstraintGrid();
            applyDifficultySettings();
            return true;
        }

        return false;
    }

    bool generateSolution() {
        solutionGrid.assign(GRID_SIZE, std::vector<int>(GRID_SIZE, -1));
        solutionDominoes.clear();

        std::vector<DominoPiece> shuffledDominoes = availableDominoes;
        std::shuffle(shuffledDominoes.begin(), shuffledDominoes.end(), rng);

        return backtrackSolution(0, shuffledDominoes);
    }

    bool backtrackSolution(int dominoIndex, const std::vector<DominoPiece>& dominoes) {
        if (dominoIndex >= static_cast<int>(dominoes.size())) {
            hasSolution = true;
            return true;
        }

        DominoPiece domino = dominoes[dominoIndex];
        std::vector<std::pair<Position, Orientation>> positions;

        for (int row = 0; row < GRID_SIZE; ++row) {
            for (int col = 0; col < GRID_SIZE; ++col) {
                positions.emplace_back(Position(row, col), Orientation::HORIZONTAL);
                positions.emplace_back(Position(row, col), Orientation::VERTICAL);
            }
        }

        std::shuffle(positions.begin(), positions.end(), rng);

        for (const auto& positionPair : positions) {
            const Position& pos = positionPair.first;
            const Orientation& orient = positionPair.second;

            if (canPlaceDominoInSolution(domino, pos, orient)) {
                placeDominoInSolution(domino, pos, orient, dominoIndex);

                if (checkRowColumnUniquenessForPlacement(pos, orient, domino)) {
                    if (backtrackSolution(dominoIndex + 1, dominoes)) {
                        return true;
                    }
                }

                removeDominoFromSolution(pos, orient);
            }
        }

        return false;
    }

    bool canPlaceDominoInSolution(const DominoPiece& domino, Position pos, Orientation orient) {
        if (orient == Orientation::HORIZONTAL) {
            if (pos.col + 1 >= GRID_SIZE) return false;
            return solutionGrid[pos.row][pos.col] == -1 &&
                solutionGrid[pos.row][pos.col + 1] == -1 &&
                !touchesOtherDominoes(pos, orient);
        }
        else {
            if (pos.row + 1 >= GRID_SIZE) return false;
            return solutionGrid[pos.row][pos.col] == -1 &&
                solutionGrid[pos.row + 1][pos.col] == -1 &&
                !touchesOtherDominoes(pos, orient);
        }
    }

    bool touchesOtherDominoes(Position pos, Orientation orient) {
        std::vector<Position> checkPositions;

        if (orient == Orientation::HORIZONTAL) {
            for (int dr = -1; dr <= 1; ++dr) {
                for (int dc = -1; dc <= 2; ++dc) {
                    if (dr == 0 && (dc == 0 || dc == 1)) continue;
                    checkPositions.emplace_back(pos.row + dr, pos.col + dc);
                }
            }
        }
        else {
            for (int dr = -1; dr <= 2; ++dr) {
                for (int dc = -1; dc <= 1; ++dc) {
                    if (dc == 0 && (dr == 0 || dr == 1)) continue;
                    checkPositions.emplace_back(pos.row + dr, pos.col + dc);
                }
            }
        }

        for (const auto& checkPos : checkPositions) {
            if (checkPos.isValidForGrid(GRID_SIZE)) {
                if (solutionGrid[checkPos.row][checkPos.col] != -1) {
                    return true;
                }
            }
        }

        return false;
    }

    bool checkRowColumnUniquenessForPlacement(Position pos, Orientation orient, const DominoPiece& domino) {
        std::set<int> affectedRows, affectedCols;

        if (orient == Orientation::HORIZONTAL) {
            affectedRows.insert(pos.row);
            affectedCols.insert(pos.col);
            affectedCols.insert(pos.col + 1);
        }
        else {
            affectedRows.insert(pos.row);
            affectedRows.insert(pos.row + 1);
            affectedCols.insert(pos.col);
        }

        for (int row : affectedRows) {
            std::set<int> rowDigits;
            for (int c = 0; c < GRID_SIZE; ++c) {
                int dominoId = solutionGrid[row][c];
                if (dominoId != -1 && dominoId < static_cast<int>(solutionDominoes.size())) {
                    const DominoPiece& piece = solutionDominoes[dominoId];
                    if (!rowDigits.insert(piece.value1).second) return false;
                    if (!rowDigits.insert(piece.value2).second) return false;
                }
            }
        }

        for (int col : affectedCols) {
            std::set<int> colDigits;
            for (int r = 0; r < GRID_SIZE; ++r) {
                int dominoId = solutionGrid[r][col];
                if (dominoId != -1 && dominoId < static_cast<int>(solutionDominoes.size())) {
                    const DominoPiece& piece = solutionDominoes[dominoId];
                    if (!colDigits.insert(piece.value1).second) return false;
                    if (!colDigits.insert(piece.value2).second) return false;
                }
            }
        }

        return true;
    }

    void placeDominoInSolution(const DominoPiece& domino, Position pos, Orientation orient, int dominoId) {
        if (orient == Orientation::HORIZONTAL) {
            solutionGrid[pos.row][pos.col] = dominoId;
            solutionGrid[pos.row][pos.col + 1] = dominoId;
        }
        else {
            solutionGrid[pos.row][pos.col] = dominoId;
            solutionGrid[pos.row + 1][pos.col] = dominoId;
        }

        DominoPiece placedDomino = domino;
        placedDomino.position = pos;
        placedDomino.orientation = orient;
        placedDomino.isPlaced = true;

        if (solutionDominoes.size() <= static_cast<size_t>(dominoId)) {
            solutionDominoes.resize(dominoId + 1);
        }
        solutionDominoes[dominoId] = placedDomino;
    }

    void removeDominoFromSolution(Position pos, Orientation orient) {
        if (orient == Orientation::HORIZONTAL) {
            solutionGrid[pos.row][pos.col] = -1;
            solutionGrid[pos.row][pos.col + 1] = -1;
        }
        else {
            solutionGrid[pos.row][pos.col] = -1;
            solutionGrid[pos.row + 1][pos.col] = -1;
        }
    }

    void generateConstraintGrid() {
        grid.assign(GRID_SIZE, std::vector<int>(GRID_SIZE, 0));

        for (int row = 0; row < GRID_SIZE; ++row) {
            for (int col = 0; col < GRID_SIZE; ++col) {
                if (solutionGrid[row][col] == -1) {
                    grid[row][col] = calculateConstraintValue(row, col);
                }
            }
        }
    }

    int calculateConstraintValue(int row, int col) const {
        if (!cacheValid) {
            constraintCache.clear();
            const_cast<bool&>(cacheValid) = true;
        }

        Position pos(row, col);
        auto it = constraintCache.find(pos);
        if (it != constraintCache.end()) {
            return it->second;
        }

        int sum = 0;
        std::set<int> adjacentDominoes;

        for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
                if (dr == 0 && dc == 0) continue;

                int newRow = row + dr;
                int newCol = col + dc;

                if (newRow >= 0 && newRow < GRID_SIZE &&
                    newCol >= 0 && newCol < GRID_SIZE) {

                    int dominoId = solutionGrid[newRow][newCol];
                    if (dominoId != -1) {
                        adjacentDominoes.insert(dominoId);
                    }
                }
            }
        }

        for (int dominoId : adjacentDominoes) {
            if (dominoId < static_cast<int>(solutionDominoes.size())) {
                sum += solutionDominoes[dominoId].sum;
            }
        }

        const_cast<std::map<Position, int>&>(constraintCache)[pos] = sum;
        return sum;
    }

    void applyDifficultySettings() {
        int cellsToHide = 0;

        switch (currentDifficulty) {
        case Difficulty::EASY:
            cellsToHide = GRID_SIZE * GRID_SIZE / 6;
            break;
        case Difficulty::MEDIUM:
            cellsToHide = GRID_SIZE * GRID_SIZE / 4;
            break;
        case Difficulty::HARD:
            cellsToHide = GRID_SIZE * GRID_SIZE / 3;
            break;
        }

        std::vector<Position> constraintPositions;
        for (int row = 0; row < GRID_SIZE; ++row) {
            for (int col = 0; col < GRID_SIZE; ++col) {
                if (grid[row][col] > 0) {
                    constraintPositions.emplace_back(row, col);
                }
            }
        }

        std::shuffle(constraintPositions.begin(), constraintPositions.end(), rng);

        for (int i = 0; i < std::min(cellsToHide, static_cast<int>(constraintPositions.size())); ++i) {
            Position pos = constraintPositions[i];
            grid[pos.row][pos.col] = 0;
        }
    }

    std::vector<Position> getDominoPositions(const DominoPiece& domino) const {
        std::vector<Position> positions = { domino.position };
        if (domino.orientation == Orientation::HORIZONTAL) {
            positions.emplace_back(domino.position.row, domino.position.col + 1);
        }
        else {
            positions.emplace_back(domino.position.row + 1, domino.position.col);
        }
        return positions;
    }

public:
    bool placeDomino(const DominoPiece& domino, Position position, Orientation orientation) {
        if (!position.isValidForGrid(GRID_SIZE) || !canPlaceDomino(domino, position, orientation)) {
            return false;
        }

        for (const auto& placed : placedDominoes) {
            if (placed == domino) {
                return false;
            }
        }

        DominoPiece newDomino = domino;
        newDomino.position = position;
        newDomino.orientation = orientation;
        newDomino.isPlaced = true;

        placedDominoes.push_back(newDomino);

        int dominoId = placedDominoes.size() - 1;
        auto positions = getDominoPositions(newDomino);
        for (const auto& pos : positions) {
            dominoGrid[pos.row][pos.col] = dominoId;
        }

        constraintCache.clear();
        cacheValid = false;

        if (placedDominoes.size() == availableDominoes.size()) {
            gameCompleted = isValidSolution();
        }

        return true;
    }

    bool canPlaceDomino(const DominoPiece& domino, Position position, Orientation orientation) {
        if (!position.isValidForGrid(GRID_SIZE)) return false;

        Position secondPos = (orientation == Orientation::HORIZONTAL)
            ? Position(position.row, position.col + 1)
            : Position(position.row + 1, position.col);

        if (!secondPos.isValidForGrid(GRID_SIZE)) return false;

        if (dominoGrid[position.row][position.col] != -1 ||
            dominoGrid[secondPos.row][secondPos.col] != -1) {
            return false;
        }

        if (grid[position.row][position.col] > 0 ||
            grid[secondPos.row][secondPos.col] > 0) {
            return false;
        }

        return wouldMaintainRowColumnUniqueness(domino, position, orientation) &&
            !wouldTouchOtherDominoes(position, orientation);
    }

    bool wouldTouchOtherDominoes(Position position, Orientation orientation) {
        std::vector<Position> checkPositions;

        if (orientation == Orientation::HORIZONTAL) {
            for (int dr = -1; dr <= 1; ++dr) {
                for (int dc = -1; dc <= 2; ++dc) {
                    if (dr == 0 && (dc == 0 || dc == 1)) continue;
                    checkPositions.emplace_back(position.row + dr, position.col + dc);
                }
            }
        }
        else {
            for (int dr = -1; dr <= 2; ++dr) {
                for (int dc = -1; dc <= 1; ++dc) {
                    if (dc == 0 && (dr == 0 || dr == 1)) continue;
                    checkPositions.emplace_back(position.row + dr, position.col + dc);
                }
            }
        }

        for (const auto& checkPos : checkPositions) {
            if (checkPos.isValidForGrid(GRID_SIZE)) {
                if (dominoGrid[checkPos.row][checkPos.col] != -1) {
                    return true;
                }
            }
        }

        return false;
    }

    bool wouldMaintainRowColumnUniqueness(const DominoPiece& domino, Position position, Orientation orientation) {
        std::set<int> affectedRows, affectedCols;

        if (orientation == Orientation::HORIZONTAL) {
            affectedRows.insert(position.row);
            affectedCols.insert(position.col);
            affectedCols.insert(position.col + 1);
        }
        else {
            affectedRows.insert(position.row);
            affectedRows.insert(position.row + 1);
            affectedCols.insert(position.col);
        }

        for (int row : affectedRows) {
            std::set<int> rowDigits;
            for (int c = 0; c < GRID_SIZE; ++c) {
                int dominoId = dominoGrid[row][c];
                if (dominoId != -1 && dominoId < static_cast<int>(placedDominoes.size())) {
                    const DominoPiece& piece = placedDominoes[dominoId];
                    rowDigits.insert(piece.value1);
                    rowDigits.insert(piece.value2);
                }
            }
            if (rowDigits.count(domino.value1) > 0 || rowDigits.count(domino.value2) > 0) {
                return false;
            }
        }

        for (int col : affectedCols) {
            std::set<int> colDigits;
            for (int r = 0; r < GRID_SIZE; ++r) {
                int dominoId = dominoGrid[r][col];
                if (dominoId != -1 && dominoId < static_cast<int>(placedDominoes.size())) {
                    const DominoPiece& piece = placedDominoes[dominoId];
                    colDigits.insert(piece.value1);
                    colDigits.insert(piece.value2);
                }
            }
            if (colDigits.count(domino.value1) > 0 || colDigits.count(domino.value2) > 0) {
                return false;
            }
        }

        return true;
    }

    bool removeDomino(Position position) {
        if (!position.isValidForGrid(GRID_SIZE)) return false;

        int dominoId = dominoGrid[position.row][position.col];
        if (dominoId == -1) return false;

        for (auto it = placedDominoes.begin(); it != placedDominoes.end(); ++it) {
            auto positions = getDominoPositions(*it);
            bool found = false;
            for (const auto& pos : positions) {
                if (pos == position) {
                    found = true;
                    break;
                }
            }

            if (found) {
                for (const auto& pos : positions) {
                    dominoGrid[pos.row][pos.col] = -1;
                }

                placedDominoes.erase(it);
                updateDominoIds();
                constraintCache.clear();
                cacheValid = false;
                gameCompleted = false;
                return true;
            }
        }

        return false;
    }

    void updateDominoIds() {
        std::vector<std::vector<int>> tempGrid(GRID_SIZE, std::vector<int>(GRID_SIZE, -1));

        for (size_t i = 0; i < placedDominoes.size(); ++i) {
            auto positions = getDominoPositions(placedDominoes[i]);
            for (const auto& pos : positions) {
                if (tempGrid[pos.row][pos.col] != -1) {
                    throw std::runtime_error("Domino position conflict detected");
                }
                tempGrid[pos.row][pos.col] = static_cast<int>(i);
            }
        }

        dominoGrid = std::move(tempGrid);
    }

    bool moveDomino(Position fromPosition, Position toPosition, Orientation newOrientation) {
        if (!fromPosition.isValidForGrid(GRID_SIZE) || !toPosition.isValidForGrid(GRID_SIZE)) {
            return false;
        }

        int dominoId = dominoGrid[fromPosition.row][fromPosition.col];
        if (dominoId == -1 || dominoId >= static_cast<int>(placedDominoes.size())) {
            return false;
        }

        DominoPiece& domino = placedDominoes[dominoId];
        auto originalPositions = getDominoPositions(domino);

        for (const auto& pos : originalPositions) {
            dominoGrid[pos.row][pos.col] = -1;
        }

        auto originalState = domino;

        try {
            domino.position = toPosition;
            domino.orientation = newOrientation;

            if (!canPlaceDomino(domino, toPosition, newOrientation)) {
                throw std::runtime_error("Invalid move");
            }

            auto newPositions = getDominoPositions(domino);
            for (const auto& pos : newPositions) {
                dominoGrid[pos.row][pos.col] = dominoId;
            }

            constraintCache.clear();
            cacheValid = false;
            return true;
        }
        catch (...) {
            domino = originalState;
            for (const auto& pos : originalPositions) {
                dominoGrid[pos.row][pos.col] = dominoId;
            }
            return false;
        }
    }

    bool isValidSolution() const {
        if (placedDominoes.size() != availableDominoes.size()) {
            return false;
        }

        for (int row = 0; row < GRID_SIZE; ++row) {
            for (int col = 0; col < GRID_SIZE; ++col) {
                if (grid[row][col] > 0) {
                    int calculated = calculateConstraintValue(row, col);
                    if (calculated != grid[row][col]) {
                        return false;
                    }
                }
            }
        }

        return checkRowColumnUniqueness();
    }

    bool checkRowColumnUniqueness() const {
        try {
            for (int r = 0; r < GRID_SIZE; ++r) {
                std::set<int> rowDigits;
                for (int c = 0; c < GRID_SIZE; ++c) {
                    int dominoId = dominoGrid[r][c];
                    if (dominoId != -1 && dominoId < static_cast<int>(placedDominoes.size())) {
                        const DominoPiece& piece = placedDominoes[dominoId];
                        if (!rowDigits.insert(piece.value1).second) return false;
                        if (!rowDigits.insert(piece.value2).second) return false;
                    }
                }
            }

            for (int c = 0; c < GRID_SIZE; ++c) {
                std::set<int> colDigits;
                for (int r = 0; r < GRID_SIZE; ++r) {
                    int dominoId = dominoGrid[r][c];
                    if (dominoId != -1 && dominoId < static_cast<int>(placedDominoes.size())) {
                        const DominoPiece& piece = placedDominoes[dominoId];
                        if (!colDigits.insert(piece.value1).second) return false;
                        if (!colDigits.insert(piece.value2).second) return false;
                    }
                }
            }

            return true;
        }
        catch (const std::exception& e) {
            std::cerr << "Error in checkRowColumnUniqueness: " << e.what() << std::endl;
            return false;
        }
    }

    bool isGameCompleted() const { return gameCompleted; }
    int getHintsUsed() const { return hintsUsed; }
    Difficulty getDifficulty() const { return currentDifficulty; }
    const std::vector<std::vector<int>>& getGrid() const { return grid; }
    const std::vector<std::vector<int>>& getDominoGrid() const { return dominoGrid; }
    const std::vector<DominoPiece>& getAvailableDominoes() const { return availableDominoes; }
    const std::vector<DominoPiece>& getPlacedDominoes() const { return placedDominoes; }
    int getGridSize() const { return GRID_SIZE; }
};
