#ifndef STATENODE_H
#define STATENODE_H

#include <array>
#include <QString>

// Node structure representing a state in the 8-puzzle
struct StateNode {
    std::array<int, 9> state;   // 1D array representing the 3x3 board, 0 for empty
    int g;                      // actual cost from start to this state
    int h;                      // heuristic estimate to goal
    int f;                      // g + h
    int zeroPos;                // index of empty cell (0~8)
    StateNode* parent;          // pointer to parent node (for path reconstruction)
    QString move;               // move direction from parent to this node

    StateNode();
    StateNode(const std::array<int, 9>& s, int gVal, int hVal, StateNode* par = nullptr, int zPos = -1);
    bool operator==(const StateNode& other) const;
};

// Hash function for StateNode to be used in unordered_set
namespace std {
    template<> struct hash<StateNode> {
        size_t operator()(const StateNode& node) const;
    };
}

#endif // STATENODE_H