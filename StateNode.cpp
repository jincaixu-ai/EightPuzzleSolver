#pragma execution_character_set("utf-8")
#include "StateNode.h"
#include <functional>

StateNode::StateNode() : g(0), h(0), f(0), zeroPos(-1), parent(nullptr) {
    state.fill(0);
}

StateNode::StateNode(const std::array<int, 9>& s, int gVal, int hVal, StateNode* par, int zPos)
    : state(s), g(gVal), h(hVal), f(gVal + hVal), zeroPos(zPos), parent(par) {
}

bool StateNode::operator==(const StateNode& other) const {
    return state == other.state;
}

namespace std {
    size_t hash<StateNode>::operator()(const StateNode& node) const {
        size_t seed = 0;
        for (int v : node.state) {
            seed ^= std::hash<int>{}(v)+0x9e3779b9 + (seed << 6) + (seed >> 2);
        }
        return seed;
    }
}