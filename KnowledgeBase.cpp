#include "KnowledgeBase.h"
#include <cmath>
#include <QString>

int KnowledgeBase::manhattan(const std::array<int, 9>& state) {
    int dist = 0;
    for (int i = 0; i < 9; ++i) {
        int val = state[i];
        if (val == 0) continue;
        int targetRow = (val - 1) / 3;
        int targetCol = (val - 1) % 3;
        int curRow = i / 3;
        int curCol = i % 3;
        dist += abs(curRow - targetRow) + abs(curCol - targetCol);
    }
    return dist;
}

int KnowledgeBase::misplaced(const std::array<int, 9>& state) {
    int cnt = 0;
    for (int i = 0; i < 9; ++i) {
        if (state[i] != 0 && state[i] != i + 1) cnt++;
    }
    return cnt;
}

int KnowledgeBase::euclidean(const std::array<int, 9>& state) {
    double sum = 0;
    for (int i = 0; i < 9; ++i) {
        int val = state[i];
        if (val == 0) continue;
        int targetRow = (val - 1) / 3;
        int targetCol = (val - 1) % 3;
        int curRow = i / 3;
        int curCol = i % 3;
        sum += (curRow - targetRow) * (curRow - targetRow) + (curCol - targetCol) * (curCol - targetCol);
    }
    return static_cast<int>(sqrt(sum));
}

std::vector<StateNode*> KnowledgeBase::expand(const StateNode* node) {
    std::vector<StateNode*> successors;
    int zp = node->zeroPos;
    int row = zp / 3, col = zp % 3;
    int dr[] = { -1, 1, 0, 0 };
    int dc[] = { 0, 0, -1, 1 };
    QString moves[] = { "Up", "Down", "Left", "Right" };
    for (int i = 0; i < 4; ++i) {
        int nr = row + dr[i];
        int nc = col + dc[i];
        if (nr >= 0 && nr < 3 && nc >= 0 && nc < 3) {
            int newPos = nr * 3 + nc;
            std::array<int, 9> newState = node->state;
            std::swap(newState[zp], newState[newPos]);
            int newG = node->g + 1;
            // h value will be computed later by AStarSolver using chosen heuristic
            int newH = 0; // placeholder, will be set by solver
            StateNode* child = new StateNode(newState, newG, newH, const_cast<StateNode*>(node), newPos);
            child->move = moves[i];
            successors.push_back(child);
        }
    }
    return successors;
}

bool KnowledgeBase::isGoal(const std::array<int, 9>& state, const std::array<int, 9>& goal) {
    return state == goal;
}

bool KnowledgeBase::isSolvable(const std::array<int, 9>& start, const std::array<int, 9>& goal) {
    auto inversionCount = [](const std::array<int, 9>& arr) {
        int inv = 0;
        for (int i = 0; i < 9; ++i) {
            if (arr[i] == 0) continue;
            for (int j = i + 1; j < 9; ++j) {
                if (arr[j] != 0 && arr[i] > arr[j]) inv++;
            }
        }
        return inv;
        };
    return (inversionCount(start) % 2) == (inversionCount(goal) % 2);
}