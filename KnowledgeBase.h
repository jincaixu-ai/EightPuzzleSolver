#ifndef KNOWLEDGEBASE_H
#define KNOWLEDGEBASE_H

#include <array>
#include <vector>
#include "StateNode.h"

/**
 * Knowledge Base: stores domain knowledge for the 8-puzzle problem.
 * All methods are static, providing rules and evaluation functions.
 */
class KnowledgeBase {
public:
    // ----- Heuristic functions (evaluation knowledge) -----
    static int manhattan(const std::array<int, 9>& state);
    static int misplaced(const std::array<int, 9>& state);
    static int euclidean(const std::array<int, 9>& state);

    // ----- State expansion rules (operators) -----
    // Generate all legal successor states by moving the blank
    static std::vector<StateNode*> expand(const StateNode* node);

    // ----- Goal test -----
    static bool isGoal(const std::array<int, 9>& state, const std::array<int, 9>& goal);

    // ----- Solvability test (using inversion count parity) -----
    static bool isSolvable(const std::array<int, 9>& start, const std::array<int, 9>& goal);
};

#endif // KNOWLEDGEBASE_H
