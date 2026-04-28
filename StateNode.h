#pragma once
#include <array>
#include <QString>

struct StateNode {
    std::array<int, 9> state;   // 一维数组表示3x3棋盘，0表示空格
    int g;                      // 实际代价
    int h;                      // 启发式估计
    int f;                      // g + h
    int zeroPos;                // 空格索引 (0~8)
    StateNode* parent;          // 父节点指针
    QString move;               // 从父节点到本节点的移动方向

    StateNode();
    StateNode(const std::array<int, 9>& s, int gVal, int hVal, StateNode* par = nullptr, int zPos = -1);
    bool operator==(const StateNode& other) const;
};

// 哈希函数，用于 unordered_set
namespace std {
    template<> struct hash<StateNode> {
        size_t operator()(const StateNode& node) const;
    };
}