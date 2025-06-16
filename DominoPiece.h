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

class DominoPiece {
private:
    int value1, value2;
    int sum;
    Position position;
    Orientation orientation;
    bool isPlaced;
    int uniqueId;
    static int nextId;

public:
    // Default constructor
    DominoPiece()
        : value1(0), value2(0), sum(0),
        position(-1, -1), orientation(Orientation::HORIZONTAL),
        isPlaced(false), uniqueId(nextId++) {
    }

    // Parameterized constructor
    DominoPiece(int v1, int v2)
        : value1(v1), value2(v2), sum(v1 + v2),
        position(-1, -1), orientation(Orientation::HORIZONTAL),
        isPlaced(false), uniqueId(nextId++) {
    }

    // Copy constructor
    DominoPiece(const DominoPiece& other)
        : value1(other.value1), value2(other.value2), sum(other.sum),
        position(other.position), orientation(other.orientation),
        isPlaced(other.isPlaced), uniqueId(other.uniqueId) {
    }

    // Assignment operator
    DominoPiece& operator=(const DominoPiece& other) {
        if (this != &other) {
            value1 = other.value1;
            value2 = other.value2;
            sum = other.sum;
            position = other.position;
            orientation = other.orientation;
            isPlaced = other.isPlaced;
            uniqueId = other.uniqueId;
        }
        return *this;
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

    int getValue1() const { return value1; }
    int getValue2() const { return value2; }
    int getSum() const { return sum; }
    Position getPosition() const { return position; }
    Orientation getOrientation() const { return orientation; }
    bool getIsPlaced() const { return isPlaced; }
    int getId() const { return uniqueId; }

    int& getValue1Ref() { return value1; }
    int& getValue2Ref() { return value2; }
    int& getSumRef() { return sum; }
    Position& getPositionRef() { return position; }
    Orientation& getOrientationRef() { return orientation; }
    bool& getIsPlacedRef() { return isPlaced; }

    void setPosition(const Position& pos) { position = pos; }
    void setOrientation(Orientation orient) { orientation = orient; }
    void setPlaced(bool placed) { isPlaced = placed; }
    void setValues(int v1, int v2) {
        value1 = v1;
        value2 = v2;
        sum = v1 + v2;
    }

    std::pair<int, int> getCanonicalForm() const {
        int smaller = (value1 < value2) ? value1 : value2;
        int larger = (value1 < value2) ? value2 : value1;
        return { smaller, larger };
    }

    std::string toString() const {
        return std::to_string(value1) + "-" + std::to_string(value2);
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

    bool move(const Position& newPos, Orientation newOrient, int gridSize) {
        if (!canBePlacedAt(newPos, newOrient, gridSize)) {
            return false;
        }

        position = newPos;
        orientation = newOrient;
        return true;
    }

    void flip() {
        std::swap(value1, value2);
    }

    void rotate() {
        orientation = (orientation == Orientation::HORIZONTAL) ?
            Orientation::VERTICAL : Orientation::HORIZONTAL;
    }

    bool containsValue(int value) const {
        return value1 == value || value2 == value;
    }

    int getOtherValue(int knownValue) const {
        if (value1 == knownValue) return value2;
        if (value2 == knownValue) return value1;
        return -1;
    }

    static void resetIdCounter() {
        nextId = 0;
    }

    static std::vector<DominoPiece> createStandardSet() {
        std::vector<DominoPiece> dominoes;
        std::set<DominoPiece> uniquePieces;

        for (int v1 = 1; v1 <= 5; ++v1) {
            for (int v2 = v1 + 1; v2 <= 6; ++v2) {
                uniquePieces.emplace(v1, v2);
            }
        }

        dominoes.assign(uniquePieces.begin(), uniquePieces.end());
        std::sort(dominoes.begin(), dominoes.end());
        return dominoes;
    }

    static std::vector<DominoPiece> createExtendedSet() {
        std::vector<DominoPiece> dominoes;
        std::set<DominoPiece> uniquePieces;

        for (int sum = 12; sum <= 56; ++sum) {
            for (int v1 = 6; v1 <= 28; ++v1) {
                int v2 = sum - v1;
                if (v2 >= v1 && v2 <= 28) {
                    uniquePieces.emplace(v1, v2);
                }
            }
        }

        dominoes.assign(uniquePieces.begin(), uniquePieces.end());
        std::sort(dominoes.begin(), dominoes.end());
        return dominoes;
    }
};

int DominoPiece::nextId = 0;

class DominoGame {
private:
    static const int GRID_SIZE = 8;
    static const int MAX_GENERATION_ATTEMPTS = 100;
    static const int MAX_HINTS_ALLOWED = 3;

    std::vector<std::vector<int>> grid;
    std::vector<std::vector<int>> dominoGrid;
    std::vector<DominoPiece> availableDominoes;
    std::vector<DominoPiece> placedDominoes;
    Difficulty currentDifficulty;
    bool gameCompleted;
    int hintsUsed;
    int movesCount;
    std::chrono::steady_clock::time_point gameStartTime;
    std::vector<std::vector<int>> solutionGrid;
    std::vector<DominoPiece> solutionDominoes;
    bool hasSolution;
    std::mt19937 rng;
    mutable std::map<Position, int> constraintCache;
    mutable bool cacheValid;
    bool useExtendedSet;

public:
    DominoGame(bool useExtended = false)
        : rng(std::chrono::steady_clock::now().time_since_epoch().count()),
        useExtendedSet(useExtended) {
        initializeGame();
    }

    void initializeGame() {
        grid.assign(GRID_SIZE, std::vector<int>(GRID_SIZE, 0));
        dominoGrid.assign(GRID_SIZE, std::vector<int>(GRID_SIZE, -1));
        placedDominoes.clear();
        gameCompleted = false;
        hintsUsed = 0;
        movesCount = 0;
        hasSolution = false;
        cacheValid = false;
        constraintCache.clear();
        generateAvailableDominoes();
    }

    void generateAvailableDominoes() {
        if (useExtendedSet) {
            availableDominoes = DominoPiece::createExtendedSet();
        }
        else {
            availableDominoes = DominoPiece::createStandardSet();
        }
    }

    void setExtendedMode(bool extended) {
        useExtendedSet = extended;
        initializeGame();
    }

    int getElapsedSeconds() const {
        auto now = std::chrono::steady_clock::now();
        return std::chrono::duration_cast<std::chrono::seconds>(now - gameStartTime).count();
    }

    int getMovesCount() const { return movesCount; }
    int getPlacedDominoesCount() const { return static_cast<int>(placedDominoes.size()); }
    int getTotalDominoesCount() const { return static_cast<int>(availableDominoes.size()); }
    int getRemainingDominoesCount() const { return getTotalDominoesCount() - getPlacedDominoesCount(); }

    double getCompletionPercentage() const {
        if (getTotalDominoesCount() == 0) return 0.0;
        return (static_cast<double>(getPlacedDominoesCount()) / getTotalDominoesCount()) * 100.0;
    }

    std::vector<DominoPiece> getUnplacedDominoes() const {
        std::vector<DominoPiece> unplaced;
        for (const auto& available : availableDominoes) {
            bool isPlaced = false;
            for (const auto& placed : placedDominoes) {
                if (available == placed) {
                    isPlaced = true;
                    break;
                }
            }
            if (!isPlaced) {
                unplaced.push_back(available);
            }
        }
        return unplaced;
    }

    bool generateNewGame(Difficulty difficulty) {
        currentDifficulty = difficulty;
        gameStartTime = std::chrono::steady_clock::now();
        movesCount = 0;

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
        newDomino.place(position, orientation);

        placedDominoes.push_back(newDomino);
        movesCount++;

        int dominoId = placedDominoes.size() - 1;
        auto positions = newDomino.getOccupiedPositions();
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

    bool removeDomino(Position position) {
        if (!position.isValidForGrid(GRID_SIZE)) return false;

        int dominoId = dominoGrid[position.row][position.col];
        if (dominoId == -1) return false;

        for (auto it = placedDominoes.begin(); it != placedDominoes.end(); ++it) {
            auto positions = it->getOccupiedPositions();
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
                movesCount++;
                return true;
            }
        }

        return false;
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
        auto originalPositions = domino.getOccupiedPositions();

        for (const auto& pos : originalPositions) {
            dominoGrid[pos.row][pos.col] = -1;
        }

        auto originalState = domino;

        try {
            domino.setPosition(toPosition);
            domino.setOrientation(newOrientation);

            if (!canPlaceDomino(domino, toPosition, newOrientation)) {
                throw std::runtime_error("Invalid move");
            }

            auto newPositions = domino.getOccupiedPositions();
            for (const auto& pos : newPositions) {
                dominoGrid[pos.row][pos.col] = dominoId;
            }

            constraintCache.clear();
            cacheValid = false;
            movesCount++;
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

    bool useHint() {
        if (hintsUsed >= MAX_HINTS_ALLOWED) {
            return false;
        }
        hintsUsed++;
        return true;
    }

    bool canUseHint() const {
        return hintsUsed < MAX_HINTS_ALLOWED;
    }

    int getHintsRemaining() const {
        return MAX_HINTS_ALLOWED - hintsUsed;
    }

    void resetGame() {
        initializeGame();
        gameStartTime = std::chrono::steady_clock::now();
    }

    struct SaveData {
        std::vector<std::vector<int>> grid;
        std::vector<std::vector<int>> dominoGrid;
        std::vector<DominoPiece> placedDominoes;
        std::vector<DominoPiece> availableDominoes;
        Difficulty difficulty;
        bool gameCompleted;
        int hintsUsed;
        int movesCount;
        bool useExtendedSet;
        std::chrono::steady_clock::time_point gameStartTime;
    };

    SaveData getSaveData() const {
        SaveData data;
        data.grid = grid;
        data.dominoGrid = dominoGrid;
        data.placedDominoes = placedDominoes;
        data.availableDominoes = availableDominoes;
        data.difficulty = currentDifficulty;
        data.gameCompleted = gameCompleted;
        data.hintsUsed = hintsUsed;
        data.movesCount = movesCount;
        data.useExtendedSet = useExtendedSet;
        data.gameStartTime = gameStartTime;
        return data;
    }

    void loadSaveData(const SaveData& data) {
        grid = data.grid;
        dominoGrid = data.dominoGrid;
        placedDominoes = data.placedDominoes;
        availableDominoes = data.availableDominoes;
        currentDifficulty = data.difficulty;
        gameCompleted = data.gameCompleted;
        hintsUsed = data.hintsUsed;
        movesCount = data.movesCount;
        useExtendedSet = data.useExtendedSet;
        gameStartTime = data.gameStartTime;
        constraintCache.clear();
        cacheValid = false;
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

    bool isGameCompleted() const { return gameCompleted; }
    int getHintsUsed() const { return hintsUsed; }
    Difficulty getDifficulty() const { return currentDifficulty; }
    const std::vector<std::vector<int>>& getGrid() const { return grid; }
    const std::vector<std::vector<int>>& getDominoGrid() const { return dominoGrid; }
    const std::vector<DominoPiece>& getAvailableDominoes() const { return availableDominoes; }
    const std::vector<DominoPiece>& getPlacedDominoes() const { return placedDominoes; }
    int getGridSize() const { return GRID_SIZE; }
    bool isUsingExtendedSet() const { return useExtendedSet; }

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
                    if (!rowDigits.insert(piece.getValue1()).second) return false;
                    if (!rowDigits.insert(piece.getValue2()).second) return false;
                }
            }
        }

        for (int col : affectedCols) {
            std::set<int> colDigits;
            for (int r = 0; r < GRID_SIZE; ++r) {
                int dominoId = solutionGrid[r][col];
                if (dominoId != -1 && dominoId < static_cast<int>(solutionDominoes.size())) {
                    const DominoPiece& piece = solutionDominoes[dominoId];
                    if (!colDigits.insert(piece.getValue1()).second) return false;
                    if (!colDigits.insert(piece.getValue2()).second) return false;
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
                sum += solutionDominoes[dominoId].getSum();
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
                    if (!rowDigits.insert(piece.getValue1()).second) return false;
                    if (!rowDigits.insert(piece.getValue2()).second) return false;
                }
            }
            if (rowDigits.count(domino.getValue1()) > 0 || rowDigits.count(domino.getValue2()) > 0) {
                return false;
            }
        }

        for (int col : affectedCols) {
            std::set<int> colDigits;
            for (int r = 0; r < GRID_SIZE; ++r) {
                int dominoId = dominoGrid[r][col];
                if (dominoId != -1 && dominoId < static_cast<int>(placedDominoes.size())) {
                    const DominoPiece& piece = placedDominoes[dominoId];
                    if (!colDigits.insert(piece.getValue1()).second) return false;
                    if (!colDigits.insert(piece.getValue2()).second) return false;
                }
            }
            if (colDigits.count(domino.getValue1()) > 0 || colDigits.count(domino.getValue2()) > 0) {
                return false;
            }
        }

        return true;
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

    void updateDominoIds() {
        std::vector<std::vector<int>> tempGrid(GRID_SIZE, std::vector<int>(GRID_SIZE, -1));

        for (size_t i = 0; i < placedDominoes.size(); ++i) {
            auto positions = placedDominoes[i].getOccupiedPositions();
            for (const auto& pos : positions) {
                if (tempGrid[pos.row][pos.col] != -1) {
                    throw std::runtime_error("Domino position conflict detected");
                }
                tempGrid[pos.row][pos.col] = static_cast<int>(i);
            }
        }

        dominoGrid = std::move(tempGrid);
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
                        if (!rowDigits.insert(piece.getValue1()).second) return false;
                        if (!rowDigits.insert(piece.getValue2()).second) return false;
                    }
                }
            }

            for (int c = 0; c < GRID_SIZE; ++c) {
                std::set<int> colDigits;
                for (int r = 0; r < GRID_SIZE; ++r) {
                    int dominoId = dominoGrid[r][c];
                    if (dominoId != -1 && dominoId < static_cast<int>(placedDominoes.size())) {
                        const DominoPiece& piece = placedDominoes[dominoId];
                        if (!colDigits.insert(piece.getValue1()).second) return false;
                        if (!colDigits.insert(piece.getValue2()).second) return false;
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
};