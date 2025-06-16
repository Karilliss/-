#pragma once
#include <vector>
#include <map>
#include <algorithm>
#include <random>
#include <set>
#include "DominoGame.h"  // Assuming this contains the DominoGame class

class HintSystem {
private:
    // Reference to the game instance
    DominoGame& game;

    // Hint configuration
    int maxHintsAllowed;
    int hintsUsed;

    // Random number generator for hint selection
    std::mt19937 rng;

    // Helper method to get unplaced dominoes
    std::vector<DominoPiece> getUnplacedDominoes() const {
        std::vector<DominoPiece> unplaced;
        const auto& available = game.getAvailableDominoes();
        const auto& placed = game.getPlacedDominoes();

        std::set<DominoPiece> placedSet(placed.begin(), placed.end());

        for (const auto& domino : available) {
            if (placedSet.find(domino) == placedSet.end()) {
                unplaced.push_back(domino);
            }
        }

        return unplaced;
    }

    // Helper method to calculate constraint value (since the original is private)
    int calculateConstraintValueHelper(int row, int col) const {
        int sum = 0;
        std::set<int> adjacentDominoes;
        const auto& dominoGrid = game.getDominoGrid();
        const auto& placedDominoes = game.getPlacedDominoes();

        // Check all adjacent cells
        for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
                if (dr == 0 && dc == 0) continue;

                int newRow = row + dr;
                int newCol = col + dc;

                if (newRow >= 0 && newRow < game.getGridSize() &&
                    newCol >= 0 && newCol < game.getGridSize()) {

                    int dominoId = dominoGrid[newRow][newCol];
                    if (dominoId != -1 && dominoId < static_cast<int>(placedDominoes.size())) {
                        adjacentDominoes.insert(dominoId);
                    }
                }
            }
        }

        for (int dominoId : adjacentDominoes) {
            if (dominoId < static_cast<int>(placedDominoes.size())) {
                sum += placedDominoes[dominoId].sum;
            }
        }

        return sum;
    }

public:
    explicit HintSystem(DominoGame& gameRef, int maxHints = 3)
        : game(gameRef), maxHintsAllowed(maxHints), hintsUsed(0),
        rng(std::random_device{}()) {
    }

    // Get the number of hints remaining
    int getHintsRemaining() const {
        return maxHintsAllowed - hintsUsed;
    }

    // Check if a hint can be provided
    bool canProvideHint() const {
        return hintsUsed < maxHintsAllowed && !game.isGameCompleted();
    }

    // Reset the hint counter
    void reset() {
        hintsUsed = 0;
    }

    // Provide a hint about which domino to place next
    struct DominoHint {
        DominoPiece domino;
        Position position;
        Orientation orientation;
        bool isValid;

        // Default constructor
        DominoHint() : domino(1, 2), position(), orientation(Orientation::HORIZONTAL), isValid(false) {}

        // Parameterized constructor
        DominoHint(const DominoPiece& d, const Position& p, Orientation o, bool valid)
            : domino(d), position(p), orientation(o), isValid(valid) {
        }
    };

    DominoHint getNextDominoHint() {
        if (!canProvideHint()) {
            return DominoHint();
        }

        // Get all unplaced dominoes
        auto unplaced = getUnplacedDominoes();
        if (unplaced.empty()) {
            return DominoHint();
        }

        // Find any valid placement for the first unplaced domino
        std::vector<std::tuple<DominoPiece, Position, Orientation>> validPlacements;

        for (const auto& domino : unplaced) {
            // Check all possible positions
            for (int row = 0; row < game.getGridSize(); ++row) {
                for (int col = 0; col < game.getGridSize(); ++col) {
                    Position pos(row, col);

                    // Try horizontal
                    if (game.canPlaceDomino(domino, pos, Orientation::HORIZONTAL)) {
                        validPlacements.emplace_back(domino, pos, Orientation::HORIZONTAL);
                    }

                    // Try vertical
                    if (game.canPlaceDomino(domino, pos, Orientation::VERTICAL)) {
                        validPlacements.emplace_back(domino, pos, Orientation::VERTICAL);
                    }
                }
            }

            // If we found valid placements for this domino, break
            if (!validPlacements.empty()) {
                break;
            }
        }

        if (!validPlacements.empty()) {
            // Select a random valid placement
            std::uniform_int_distribution<size_t> dist(0, validPlacements.size() - 1);
            auto selection = validPlacements[dist(rng)];

            hintsUsed++;
            return DominoHint(std::get<0>(selection), std::get<1>(selection),
                std::get<2>(selection), true);
        }

        // No valid hints available
        return DominoHint();
    }

    // Highlight a cell that has incorrect constraints
    struct ConstraintHint {
        Position position;
        int expectedValue;
        int currentSum;
        bool isValid;

        // Default constructor
        ConstraintHint() : position(), expectedValue(0), currentSum(0), isValid(false) {}

        // Parameterized constructor
        ConstraintHint(const Position& p, int expected, int current, bool valid)
            : position(p), expectedValue(expected), currentSum(current), isValid(valid) {
        }
    };

    ConstraintHint getConstraintHint() {
        if (!canProvideHint()) {
            return ConstraintHint();
        }

        // Find cells where the constraint doesn't match the actual sum
        const auto& grid = game.getGrid();

        for (int row = 0; row < game.getGridSize(); ++row) {
            for (int col = 0; col < game.getGridSize(); ++col) {
                Position pos(row, col);
                int constraintValue = grid[row][col];

                if (constraintValue > 0) {
                    int actualSum = calculateConstraintValueHelper(row, col);
                    if (actualSum != constraintValue) {
                        hintsUsed++;
                        return ConstraintHint(pos, constraintValue, actualSum, true);
                    }
                }
            }
        }

        return ConstraintHint();
    }

    // Get a hint about the most constrained position
    struct PositionHint {
        Position position;
        int constraintValue;
        bool isValid;

        // Default constructor
        PositionHint() : position(), constraintValue(0), isValid(false) {}

        // Parameterized constructor
        PositionHint(const Position& p, int constraint, bool valid)
            : position(p), constraintValue(constraint), isValid(valid) {
        }
    };

    PositionHint getMostConstrainedHint() {
        if (!canProvideHint()) {
            return PositionHint();
        }

        // Find the cell with the highest constraint value that's empty
        Position bestPos;
        int maxConstraint = -1;
        const auto& grid = game.getGrid();
        const auto& dominoGrid = game.getDominoGrid();

        for (int row = 0; row < game.getGridSize(); ++row) {
            for (int col = 0; col < game.getGridSize(); ++col) {
                int constraint = grid[row][col];
                if (constraint > maxConstraint && dominoGrid[row][col] == -1) {
                    maxConstraint = constraint;
                    bestPos = Position(row, col);
                }
            }
        }

        if (maxConstraint > 0) {
            hintsUsed++;
            return PositionHint(bestPos, maxConstraint, true);
        }

        return PositionHint();
    }

    // Get a random hint (combines different hint types)
    struct RandomHint {
        enum HintType { DOMINO_PLACEMENT, CONSTRAINT_MISMATCH, MOST_CONSTRAINED };
        HintType type;
        DominoHint dominoHint;
        ConstraintHint constraintHint;
        PositionHint positionHint;
        bool isValid;

        // Default constructor
        RandomHint() : type(DOMINO_PLACEMENT), dominoHint(), constraintHint(), positionHint(), isValid(false) {}
    };

    RandomHint getRandomHint() {
        if (!canProvideHint()) {
            return RandomHint();
        }

        RandomHint hint;

        // First try to suggest a domino placement
        hint.dominoHint = getNextDominoHint();
        if (hint.dominoHint.isValid) {
            hint.type = RandomHint::DOMINO_PLACEMENT;
            hint.isValid = true;
            return hint;
        }

        // Then try to point out constraint mismatches
        hint.constraintHint = getConstraintHint();
        if (hint.constraintHint.isValid) {
            hint.type = RandomHint::CONSTRAINT_MISMATCH;
            hint.isValid = true;
            return hint;
        }

        // Finally suggest looking at most constrained positions
        hint.positionHint = getMostConstrainedHint();
        if (hint.positionHint.isValid) {
            hint.type = RandomHint::MOST_CONSTRAINED;
            hint.isValid = true;
            return hint;
        }

        return RandomHint();
    }

    // Get hints used count
    int getHintsUsed() const {
        return hintsUsed;
    }

    // Set hints used (for loading saved games)
    void setHintsUsed(int used) {
        hintsUsed = std::max(0, std::min(used, maxHintsAllowed));
    }
};