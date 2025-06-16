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
#include <fstream>
#include <string>
#include <unordered_map>

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

namespace std {
    template<>
    struct hash<Position> {
        size_t operator()(const Position& pos) const {
            return hash<int>()(pos.row) ^ (hash<int>()(pos.col) << 1);
        }
    };
}

class Domino {
private:
    int value1, value2;
    int sum;
    Position position;
    Orientation orientation;
    bool isPlaced;
    int uniqueId;

    static int nextId;

public:
    Domino(int v1, int v2) : value1(v1), value2(v2), sum(v1 + v2),
        position(-1, -1), orientation(Orientation::HORIZONTAL),
        isPlaced(false), uniqueId(nextId++) {
    }

    // Getters
    int getValue1() const { return value1; }
    int getValue2() const { return value2; }
    int getSum() const { return sum; }
    Position getPosition() const { return position; }
    Orientation getOrientation() const { return orientation; }
    bool getIsPlaced() const { return isPlaced; }
    int getId() const { return uniqueId; }

    // Setters
    void setPosition(const Position& pos) { position = pos; }
    void setOrientation(Orientation orient) { orientation = orient; }
    void setPlaced(bool placed) { isPlaced = placed; }
    void setValues(int v1, int v2) {
        value1 = v1;
        value2 = v2;
        sum = v1 + v2;
    }

    // Utility methods
    std::pair<int, int> getCanonicalForm() const {
        return { std::min(value1, value2), std::max(value1, value2) };
    }

    std::vector<Position> getOccupiedPositions() const {
        std::vector<Position> positions = { position };
        if (isPlaced && position.isValid()) {
            if (orientation == Orientation::HORIZONTAL) {
                positions.emplace_back(position.row, position.col + 1);
            }
            else {
                positions.emplace_back(position.row + 1, position.col);
            }
        }
        return positions;
    }

    bool canBePlacedAt(const Position& pos, Orientation orient, int gridSize) const {
        if (!pos.isValidForGrid(gridSize)) return false;

        Position secondPos;
        if (orient == Orientation::HORIZONTAL) {
            secondPos = Position(pos.row, pos.col + 1);
        }
        else {
            secondPos = Position(pos.row + 1, pos.col);
        }

        return secondPos.isValidForGrid(gridSize);
    }

    void place(const Position& pos, Orientation orient) {
        position = pos;
        orientation = orient;
        isPlaced = true;
    }

    void remove() {
        position = Position(-1, -1);
        isPlaced = false;
    }

    bool operator==(const Domino& other) const {
        auto thisCanon = getCanonicalForm();
        auto otherCanon = other.getCanonicalForm();
        return thisCanon == otherCanon;
    }

    bool operator<(const Domino& other) const {
        auto thisCanon = getCanonicalForm();
        auto otherCanon = other.getCanonicalForm();
        if (thisCanon.first != otherCanon.first) return thisCanon.first < otherCanon.first;
        return thisCanon.second < otherCanon.second;
    }

    static void resetIdCounter() {
        nextId = 0;
    }

    static std::vector<Domino> createStandardSet() {
        std::vector<Domino> dominoes;
        std::set<Domino> uniquePieces;

        for (int v1 = 0; v1 <= 6; ++v1) {
            for (int v2 = v1; v2 <= 6; ++v2) {
                uniquePieces.emplace(v1, v2);
            }
        }

        dominoes.assign(uniquePieces.begin(), uniquePieces.end());
        std::sort(dominoes.begin(), dominoes.end());
        return dominoes;
    }

    static std::vector<Domino> createExtendedSet() {
        std::vector<Domino> dominoes;
        std::set<Domino> uniquePieces;

        for (int v1 = 0; v1 <= 9; ++v1) {
            for (int v2 = v1; v2 <= 9; ++v2) {
                uniquePieces.emplace(v1, v2);
            }
        }

        dominoes.assign(uniquePieces.begin(), uniquePieces.end());
        std::sort(dominoes.begin(), dominoes.end());
        return dominoes;
    }
};

int Domino::nextId = 0;

class DominoGame {
private:
    // Constants
    static const int DEFAULT_GRID_SIZE = 8;
    static const int MAX_GRID_SIZE = 20;
    static const int MAX_GENERATION_ATTEMPTS = 100;
    static const int MAX_HINTS_ALLOWED = 3;
    static const int MIN_DOMINO_VALUE = 0;
    static const int MAX_DOMINO_VALUE = 9;

    // Game state
    int gridSize;
    std::vector<std::vector<int>> grid;
    std::vector<std::vector<int>> dominoGrid;
    std::vector<Domino> availableDominoes;
    std::vector<Domino> placedDominoes;
    std::unordered_set<int> usedSums;

    // Game settings
    Difficulty currentDifficulty;
    bool gameCompleted;
    int hintsUsed;
    int movesCount;
    std::chrono::steady_clock::time_point gameStartTime;
    bool useExtendedSet;

    // Solution storage
    std::vector<std::vector<int>> solutionGrid;
    std::vector<Domino> solutionDominoes;
    bool hasSolution;

    // Random number generator
    std::mt19937 rng;

    // Cache for performance optimization
    mutable std::unordered_map<Position, int> constraintCache;
    mutable bool cacheValid;

public:
    DominoGame(bool useExtended = false, int size = DEFAULT_GRID_SIZE)
        : gridSize(size), useExtendedSet(useExtended),
        rng(std::chrono::steady_clock::now().time_since_epoch().count()) {
        if (gridSize <= 0 || gridSize > MAX_GRID_SIZE) {
            throw std::invalid_argument("Invalid grid size");
        }
        initializeGame();
    }

    // Game control
    void initializeGame() {
        grid.assign(gridSize, std::vector<int>(gridSize, 0));
        dominoGrid.assign(gridSize, std::vector<int>(gridSize, -1));
        placedDominoes.clear();
        usedSums.clear();
        gameCompleted = false;
        hintsUsed = 0;
        movesCount = 0;
        hasSolution = false;
        cacheValid = false;
        constraintCache.clear();
        Domino::resetIdCounter();
        generateAvailableDominoes();
        gameStartTime = std::chrono::steady_clock::now();
    }

    bool generateNewGame(Difficulty difficulty) {
        currentDifficulty = difficulty;
        initializeGame();

        for (int attempt = 0; attempt < MAX_GENERATION_ATTEMPTS; ++attempt) {
            if (generateSolution()) {
                generateConstraintGrid();
                applyDifficultySettings();
                return true;
            }
        }

        std::cerr << "Warning: Failed to generate complex puzzle, creating simplified version\n";
        return generateSimplifiedPuzzle(difficulty);
    }

    // Domino operations
    bool placeDomino(const Domino& domino, Position position, Orientation orientation) {
        if (!position.isValidForGrid(gridSize) || !canPlaceDomino(domino, position, orientation)) {
            return false;
        }

        if (usedSums.count(domino.getSum()) > 0) {
            return false;
        }

        Domino newDomino = domino;
        newDomino.place(position, orientation);

        placedDominoes.push_back(newDomino);
        movesCount++;
        usedSums.insert(newDomino.getSum());

        int dominoId = static_cast<int>(placedDominoes.size()) - 1;
        auto positions = newDomino.getOccupiedPositions();
        for (const auto& pos : positions) {
            dominoGrid[pos.row][pos.col] = dominoId;
            grid[pos.row][pos.col] = newDomino.getSum();
        }

        constraintCache.clear();
        cacheValid = false;

        if (placedDominoes.size() == availableDominoes.size()) {
            gameCompleted = isValidSolution();
        }

        return true;
    }

    bool removeDomino(Position position) {
        if (!position.isValidForGrid(gridSize)) return false;

        int dominoId = dominoGrid[position.row][position.col];
        if (dominoId == -1 || dominoId >= static_cast<int>(placedDominoes.size())) {
            return false;
        }

        Domino& domino = placedDominoes[dominoId];
        auto positions = domino.getOccupiedPositions();

        for (const auto& pos : positions) {
            dominoGrid[pos.row][pos.col] = -1;
            grid[pos.row][pos.col] = 0;
        }

        usedSums.erase(domino.getSum());
        placedDominoes.erase(placedDominoes.begin() + dominoId);
        movesCount++;
        gameCompleted = false;
        updateDominoIds();

        return true;
    }

    bool moveDomino(Position from, Position to1, Position to2) {
        if (!from.isValidForGrid(gridSize)) return false;

        int dominoId = dominoGrid[from.row][from.col];
        if (dominoId == -1 || dominoId >= static_cast<int>(placedDominoes.size())) {
            return false;
        }

        Domino& domino = placedDominoes[dominoId];
        auto originalPositions = domino.getOccupiedPositions();

        // Temporarily remove the domino
        for (const auto& pos : originalPositions) {
            dominoGrid[pos.row][pos.col] = -1;
            grid[pos.row][pos.col] = 0;
        }

        // Try to place at new position
        Orientation newOrientation = (to1.row == to2.row) ? Orientation::HORIZONTAL : Orientation::VERTICAL;
        Domino tempDomino = domino;
        tempDomino.place(to1, newOrientation);

        if (!canPlaceDomino(tempDomino, to1, newOrientation)) {
            // Restore original position
            for (const auto& pos : originalPositions) {
                dominoGrid[pos.row][pos.col] = dominoId;
                grid[pos.row][pos.col] = domino.getSum();
            }
            return false;
        }

        // Place at new position
        domino.place(to1, newOrientation);
        auto newPositions = domino.getOccupiedPositions();
        for (const auto& pos : newPositions) {
            dominoGrid[pos.row][pos.col] = dominoId;
            grid[pos.row][pos.col] = domino.getSum();
        }

        movesCount++;
        constraintCache.clear();
        cacheValid = false;

        return true;
    }

    // Game state
    bool isGameCompleted() const { return gameCompleted; }
    int getHintsUsed() const { return hintsUsed; }
    int getMovesCount() const { return movesCount; }
    Difficulty getDifficulty() const { return currentDifficulty; }
    int getGridSize() const { return gridSize; }
    bool isUsingExtendedSet() const { return useExtendedSet; }
    double getElapsedTime() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration<double>(now - gameStartTime).count();
    }

    const std::vector<std::vector<int>>& getGrid() const { return grid; }
    const std::vector<Domino>& getAvailableDominoes() const { return availableDominoes; }
    const std::vector<Domino>& getPlacedDominoes() const { return placedDominoes; }

    // Hints
    bool getHint(Position& pos1, Position& pos2, int& value) {
        if (hintsUsed >= MAX_HINTS_ALLOWED || !hasSolution) {
            return false;
        }

        for (const auto& solutionDomino : solutionDominoes) {
            if (usedSums.count(solutionDomino.getSum()) == 0) {
                pos1 = solutionDomino.getPosition();
                pos2 = (solutionDomino.getOrientation() == Orientation::HORIZONTAL) ?
                    Position(pos1.row, pos1.col + 1) : Position(pos1.row + 1, pos1.col);
                value = solutionDomino.getSum();
                hintsUsed++;
                return true;
            }
        }

        return false;
    }

    // Auto-solve functionality
    bool autoSolve() {
        if (!hasSolution) {
            return false;
        }

        // Clear current placement
        for (auto& domino : placedDominoes) {
            domino.remove();
        }
        placedDominoes.clear();
        usedSums.clear();
        dominoGrid.assign(gridSize, std::vector<int>(gridSize, -1));

        // Place solution dominoes
        for (const auto& solutionDomino : solutionDominoes) {
            if (!placeDomino(solutionDomino, solutionDomino.getPosition(), solutionDomino.getOrientation())) {
                return false;
            }
        }

        return true;
    }

    // Save/Load functionality
    bool saveGame(const std::string& filename) const {
        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }

        // Save basic game state
        file.write(reinterpret_cast<const char*>(&gridSize), sizeof(gridSize));
        file.write(reinterpret_cast<const char*>(&currentDifficulty), sizeof(currentDifficulty));
        file.write(reinterpret_cast<const char*>(&useExtendedSet), sizeof(useExtendedSet));
        file.write(reinterpret_cast<const char*>(&hintsUsed), sizeof(hintsUsed));
        file.write(reinterpret_cast<const char*>(&movesCount), sizeof(movesCount));

        // Save grid
        for (const auto& row : grid) {
            for (int cell : row) {
                file.write(reinterpret_cast<const char*>(&cell), sizeof(cell));
            }
        }

        // Save placed dominoes
        size_t placedCount = placedDominoes.size();
        file.write(reinterpret_cast<const char*>(&placedCount), sizeof(placedCount));
        for (const auto& domino : placedDominoes) {
            int v1 = domino.getValue1();
            int v2 = domino.getValue2();
            Position pos = domino.getPosition();
            Orientation orient = domino.getOrientation();

            file.write(reinterpret_cast<const char*>(&v1), sizeof(v1));
            file.write(reinterpret_cast<const char*>(&v2), sizeof(v2));
            file.write(reinterpret_cast<const char*>(&pos.row), sizeof(pos.row));
            file.write(reinterpret_cast<const char*>(&pos.col), sizeof(pos.col));
            file.write(reinterpret_cast<const char*>(&orient), sizeof(orient));
        }

        return file.good();
    }

    bool loadGame(const std::string& filename) {
        std::ifstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }

        // Load basic game state
        file.read(reinterpret_cast<char*>(&gridSize), sizeof(gridSize));
        file.read(reinterpret_cast<char*>(&currentDifficulty), sizeof(currentDifficulty));
        file.read(reinterpret_cast<char*>(&useExtendedSet), sizeof(useExtendedSet));
        file.read(reinterpret_cast<char*>(&hintsUsed), sizeof(hintsUsed));
        file.read(reinterpret_cast<char*>(&movesCount), sizeof(movesCount));

        // Initialize with new settings
        initializeGame();

        // Load grid
        grid.assign(gridSize, std::vector<int>(gridSize));
        for (auto& row : grid) {
            for (int& cell : row) {
                file.read(reinterpret_cast<char*>(&cell), sizeof(cell));
            }
        }

        // Load placed dominoes
        size_t placedCount;
        file.read(reinterpret_cast<char*>(&placedCount), sizeof(placedCount));

        for (size_t i = 0; i < placedCount; ++i) {
            int v1, v2;
            Position pos;
            Orientation orient;

            file.read(reinterpret_cast<char*>(&v1), sizeof(v1));
            file.read(reinterpret_cast<char*>(&v2), sizeof(v2));
            file.read(reinterpret_cast<char*>(&pos.row), sizeof(pos.row));
            file.read(reinterpret_cast<char*>(&pos.col), sizeof(pos.col));
            file.read(reinterpret_cast<char*>(&orient), sizeof(orient));

            Domino domino(v1, v2);
            if (!placeDomino(domino, pos, orient)) {
                return false;
            }
        }

        return file.good();
    }

    // Validation methods
    bool isValidSolution() const {
        if (placedDominoes.size() != availableDominoes.size()) {
            return false;
        }

        // Check all constraints match
        for (int row = 0; row < gridSize; ++row) {
            for (int col = 0; col < gridSize; ++col) {
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

private:
    void generateAvailableDominoes() {
        availableDominoes = useExtendedSet ?
            Domino::createExtendedSet() : Domino::createStandardSet();
    }

    bool generateSolution() {
        solutionGrid.assign(gridSize, std::vector<int>(gridSize, -1));
        solutionDominoes.clear();

        std::vector<Domino> shuffledDominoes = availableDominoes;
        std::shuffle(shuffledDominoes.begin(), shuffledDominoes.end(), rng);

        return backtrackSolution(0, shuffledDominoes);
    }

    bool backtrackSolution(int dominoIndex, const std::vector<Domino>& dominoes) {
        if (dominoIndex >= static_cast<int>(dominoes.size())) {
            hasSolution = true;
            return true;
        }

        const Domino& domino = dominoes[dominoIndex];
        std::vector<std::pair<Position, Orientation>> possiblePlacements;

        // Generate all possible valid placements
        for (int row = 0; row < gridSize; ++row) {
            for (int col = 0; col < gridSize; ++col) {
                Position pos(row, col);

                // Try horizontal placement
                if (canPlaceDominoInSolution(domino, pos, Orientation::HORIZONTAL)) {
                    possiblePlacements.emplace_back(pos, Orientation::HORIZONTAL);
                }

                // Try vertical placement
                if (canPlaceDominoInSolution(domino, pos, Orientation::VERTICAL)) {
                    possiblePlacements.emplace_back(pos, Orientation::VERTICAL);
                }
            }
        }

        // Randomize placement order
        std::shuffle(possiblePlacements.begin(), possiblePlacements.end(), rng);

        // Try each possible placement
        for (const auto& placement : possiblePlacements) {
            Position pos = placement.first;
            Orientation orient = placement.second;

            placeDominoInSolution(domino, pos, orient, dominoIndex);

            if (checkRowColumnUniquenessForPlacement(pos, orient, domino)) {
                if (backtrackSolution(dominoIndex + 1, dominoes)) {
                    return true;
                }
            }

            removeDominoFromSolution(pos, orient);
        }

        return false;
    }

    bool generateSimplifiedPuzzle(Difficulty difficulty) {
        initializeGame();

        std::vector<Domino> shuffledDominoes = availableDominoes;
        std::shuffle(shuffledDominoes.begin(), shuffledDominoes.end(), rng);

        solutionGrid.assign(gridSize, std::vector<int>(gridSize, -1));
        solutionDominoes.clear();

        int dominoIndex = 0;
        // Place dominoes in a grid pattern
        for (int row = 0; row < gridSize && dominoIndex < static_cast<int>(shuffledDominoes.size()); row += 2) {
            for (int col = 0; col < gridSize - 1 && dominoIndex < static_cast<int>(shuffledDominoes.size()); col += 2) {
                if (canPlaceDominoInSolution(shuffledDominoes[dominoIndex], Position(row, col), Orientation::HORIZONTAL)) {
                    placeDominoInSolution(shuffledDominoes[dominoIndex], Position(row, col), Orientation::HORIZONTAL, dominoIndex);
                    dominoIndex++;
                }
            }
        }

        // If we still have dominoes left, try vertical placements
        for (int col = 0; col < gridSize && dominoIndex < static_cast<int>(shuffledDominoes.size()); col += 2) {
            for (int row = 0; row < gridSize - 1 && dominoIndex < static_cast<int>(shuffledDominoes.size()); row += 2) {
                if (canPlaceDominoInSolution(shuffledDominoes[dominoIndex], Position(row, col), Orientation::VERTICAL)) {
                    placeDominoInSolution(shuffledDominoes[dominoIndex], Position(row, col), Orientation::VERTICAL, dominoIndex);
                    dominoIndex++;
                }
            }
        }

        if (dominoIndex >= availableDominoes.size() / 2) {
            hasSolution = true;
            generateConstraintGrid();
            applyDifficultySettings();
            return true;
        }

        return false;
    }

    bool canPlaceDomino(const Domino& domino, Position position, Orientation orientation) const {
        if (!position.isValidForGrid(gridSize)) return false;

        Position secondPos = (orientation == Orientation::HORIZONTAL) ?
            Position(position.row, position.col + 1) : Position(position.row + 1, position.col);

        if (!secondPos.isValidForGrid(gridSize)) return false;

        // Check if positions are empty
        if (dominoGrid[position.row][position.col] != -1 ||
            dominoGrid[secondPos.row][secondPos.col] != -1) {
            return false;
        }

        // Check if sum is unique
        if (usedSums.count(domino.getSum()) > 0) {
            return false;
        }

        return wouldMaintainRowColumnUniqueness(domino, position, orientation) &&
            !wouldTouchOtherDominoes(position, orientation);
    }

    bool canPlaceDominoInSolution(const Domino& domino, Position pos, Orientation orient) const {
        if (orient == Orientation::HORIZONTAL) {
            if (pos.col + 1 >= gridSize) return false;
            return solutionGrid[pos.row][pos.col] == -1 &&
                solutionGrid[pos.row][pos.col + 1] == -1 &&
                !touchesOtherDominoes(pos, orient);
        }
        else {
            if (pos.row + 1 >= gridSize) return false;
            return solutionGrid[pos.row][pos.col] == -1 &&
                solutionGrid[pos.row + 1][pos.col] == -1 &&
                !touchesOtherDominoes(pos, orient);
        }
    }

    bool touchesOtherDominoes(Position pos, Orientation orient) const {
        std::vector<Position> checkPositions;

        if (orient == Orientation::HORIZONTAL) {
            // Check all positions around the horizontal domino (pos and pos+1 horizontally)
            for (int dr = -1; dr <= 1; ++dr) {
                for (int dc = -1; dc <= 2; ++dc) {
                    // Skip the domino's own positions
                    if (dr == 0 && (dc == 0 || dc == 1)) continue;
                    checkPositions.emplace_back(pos.row + dr, pos.col + dc);
                }
            }
        }
        else {
            // Check all positions around the vertical domino (pos and pos+1 vertically)
            for (int dr = -1; dr <= 2; ++dr) {
                for (int dc = -1; dc <= 1; ++dc) {
                    // Skip the domino's own positions
                    if (dc == 0 && (dr == 0 || dr == 1)) continue;
                    checkPositions.emplace_back(pos.row + dr, pos.col + dc);
                }
            }
        }

        for (const auto& checkPos : checkPositions) {
            if (checkPos.isValidForGrid(gridSize)) {
                if (solutionGrid[checkPos.row][checkPos.col] != -1) {
                    return true;
                }
            }
        }

        return false;
    }

    bool wouldTouchOtherDominoes(Position position, Orientation orientation) const {
        std::vector<Position> checkPositions;

        if (orientation == Orientation::HORIZONTAL) {
            // Check all positions around the horizontal domino
            for (int dr = -1; dr <= 1; ++dr) {
                for (int dc = -1; dc <= 2; ++dc) {
                    // Skip the domino's own positions
                    if (dr == 0 && (dc == 0 || dc == 1)) continue;
                    checkPositions.emplace_back(position.row + dr, position.col + dc);
                }
            }
        }
        else {
            // Check all positions around the vertical domino
            for (int dr = -1; dr <= 2; ++dr) {
                for (int dc = -1; dc <= 1; ++dc) {
                    // Skip the domino's own positions
                    if (dc == 0 && (dr == 0 || dr == 1)) continue;
                    checkPositions.emplace_back(position.row + dr, position.col + dc);
                }
            }
        }

        for (const auto& checkPos : checkPositions) {
            if (checkPos.isValidForGrid(gridSize)) {
                if (dominoGrid[checkPos.row][checkPos.col] != -1) {
                    return true;
                }
            }
        }

        return false;
    }

    void placeDominoInSolution(const Domino& domino, Position pos, Orientation orient, int dominoId) {
        if (orient == Orientation::HORIZONTAL) {
            solutionGrid[pos.row][pos.col] = dominoId;
            solutionGrid[pos.row][pos.col + 1] = dominoId;
        }
        else {
            solutionGrid[pos.row][pos.col] = dominoId;
            solutionGrid[pos.row + 1][pos.col] = dominoId;
        }

        Domino placedDomino = domino;
        placedDomino.place(pos, orient);

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
        grid.assign(gridSize, std::vector<int>(gridSize, 0));

        for (int row = 0; row < gridSize; ++row) {
            for (int col = 0; col < gridSize; ++col) {
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
        std::unordered_set<int> adjacentDominoes;

        // Check all 8 surrounding positions
        for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
                if (dr == 0 && dc == 0) continue; // Skip current cell

                int newRow = row + dr;
                int newCol = col + dc;

                if (newRow >= 0 && newRow < gridSize &&
                    newCol >= 0 && newCol < gridSize) {
                    int dominoId = solutionGrid[newRow][newCol];
                    if (dominoId != -1) {
                        adjacentDominoes.insert(dominoId);
                    }
                }
            }
        }

        // Sum unique adjacent domino sums
        for (int dominoId : adjacentDominoes) {
            if (dominoId < static_cast<int>(solutionDominoes.size())) {
                sum += solutionDominoes[dominoId].getSum();
            }
        }

        constraintCache[pos] = sum;
        return sum;
    }

    void applyDifficultySettings() {
        int cellsToHide = 0;
        switch (currentDifficulty) {
        case Difficulty::EASY:
            cellsToHide = gridSize * gridSize / 6;
            break;
        case Difficulty::MEDIUM:
            cellsToHide = gridSize * gridSize / 4;
            break;
        case Difficulty::HARD:
            cellsToHide = gridSize * gridSize / 3;
            break;
        }

        std::vector<Position> constraintPositions;
        for (int row = 0; row < gridSize; ++row) {
            for (int col = 0; col < gridSize; ++col) {
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

    bool wouldMaintainRowColumnUniqueness(const Domino& domino, Position position, Orientation orientation) const {
        std::unordered_set<int> affectedRows;
        std::unordered_set<int> affectedCols;

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

        // Check row uniqueness
        for (int row : affectedRows) {
            std::unordered_set<int> rowDigits;
            for (int c = 0; c < gridSize; ++c) {
                int dominoId = dominoGrid[row][c];
                if (dominoId != -1) {
                    const Domino& piece = placedDominoes[dominoId];
                    if (!rowDigits.insert(piece.getValue1()).second) return false;
                    if (!rowDigits.insert(piece.getValue2()).second) return false;
                }
            }
            // Check new domino values
            if (rowDigits.count(domino.getValue1()) || rowDigits.count(domino.getValue2())) {
                return false;
            }
        }

        // Check column uniqueness
        for (int col : affectedCols) {
            std::unordered_set<int> colDigits;
            for (int r = 0; r < gridSize; ++r) {
                int dominoId = dominoGrid[r][col];
                if (dominoId != -1) {
                    const Domino& piece = placedDominoes[dominoId];
                    if (!colDigits.insert(piece.getValue1()).second) return false;
                    if (!colDigits.insert(piece.getValue2()).second) return false;
                }
            }
            // Check new domino values
            if (colDigits.count(domino.getValue1()) || colDigits.count(domino.getValue2())) {
                return false;
            }
        }

        return true;
    }

    bool checkRowColumnUniquenessForPlacement(Position pos, Orientation orient, const Domino& domino) const {
        std::unordered_set<int> affectedRows;
        std::unordered_set<int> affectedCols;

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

        // Check rows
        for (int row : affectedRows) {
            std::unordered_set<int> rowDigits;
            for (int c = 0; c < gridSize; ++c) {
                int dominoId = solutionGrid[row][c];
                if (dominoId != -1) {
                    const Domino& piece = solutionDominoes[dominoId];
                    if (!rowDigits.insert(piece.getValue1()).second) return false;
                    if (!rowDigits.insert(piece.getValue2()).second) return false;
                }
            }
            // Check new domino values
            if (rowDigits.count(domino.getValue1()) || rowDigits.count(domino.getValue2())) {
                return false;
            }
        }

        // Check columns
        for (int col : affectedCols) {
            std::unordered_set<int> colDigits;
            for (int r = 0; r < gridSize; ++r) {
                int dominoId = solutionGrid[r][col];
                if (dominoId != -1) {
                    const Domino& piece = solutionDominoes[dominoId];
                    if (!colDigits.insert(piece.getValue1()).second) return false;
                    if (!colDigits.insert(piece.getValue2()).second) return false;
                }
            }
            // Check new domino values
            if (colDigits.count(domino.getValue1()) || colDigits.count(domino.getValue2())) {
                return false;
            }
        }

        return true;
    }

    void updateDominoIds() {
        std::vector<std::vector<int>> newDominoGrid(gridSize, std::vector<int>(gridSize, -1));

        for (size_t i = 0; i < placedDominoes.size(); ++i) {
            auto positions = placedDominoes[i].getOccupiedPositions();
            for (const auto& pos : positions) {
                newDominoGrid[pos.row][pos.col] = static_cast<int>(i);
            }
        }

        dominoGrid = std::move(newDominoGrid);
    }

    bool checkRowColumnUniqueness() const {
        // Check row uniqueness
        for (int row = 0; row < gridSize; ++row) {
            std::unordered_set<int> rowDigits;

            for (int col = 0; col < gridSize; ++col) {
                int dominoId = dominoGrid[row][col];
                if (dominoId != -1 && dominoId < static_cast<int>(placedDominoes.size())) {
                    const Domino& domino = placedDominoes[dominoId];

                    // Check if this is the first cell of the domino (to avoid counting twice)
                    Position dominoPos = domino.getPosition();
                    if (row == dominoPos.row && col == dominoPos.col) {
                        // Add both values from the domino
                        if (!rowDigits.insert(domino.getValue1()).second) {
                            return false; // Duplicate found in row
                        }
                        if (!rowDigits.insert(domino.getValue2()).second) {
                            return false; // Duplicate found in row
                        }
                    }
                }
            }
        }

        // Check column uniqueness
        for (int col = 0; col < gridSize; ++col) {
            std::unordered_set<int> colDigits;

            for (int row = 0; row < gridSize; ++row) {
                int dominoId = dominoGrid[row][col];
                if (dominoId != -1 && dominoId < static_cast<int>(placedDominoes.size())) {
                    const Domino& domino = placedDominoes[dominoId];

                    // Check if this is the first cell of the domino (to avoid counting twice)
                    Position dominoPos = domino.getPosition();
                    if (row == dominoPos.row && col == dominoPos.col) {
                        // Add both values from the domino
                        if (!colDigits.insert(domino.getValue1()).second) {
                            return false; // Duplicate found in column
                        }
                        if (!colDigits.insert(domino.getValue2()).second) {
                            return false; // Duplicate found in column
                        }
                    }
                }
            }
        }

        return true; // All rows and columns have unique digits
    }
};