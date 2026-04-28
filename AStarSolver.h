#pragma once
#include "StateNode.h"
#include <QObject>
#include <QElapsedTimer>
#include <unordered_set>
#include <queue>
#include <vector>
#include <QVector>

struct CompareNode {
    bool operator()(const StateNode* a, const StateNode* b) {
        return a->f > b->f;  // 最小堆
    }
};

class AStarSolver : public QObject {
    Q_OBJECT
public:
    explicit AStarSolver(QObject* parent = nullptr);
    ~AStarSolver();

    void setHeuristicType(int type); // 0:曼哈顿, 1:错位数, 2:欧几里得
    bool solve(const std::array<int, 9>& start, const std::array<int, 9>& goal);
    int getExpandedCount() const { return expandedCount; }
    int getGeneratedCount() const { return generatedCount; }
    qint64 getElapsedTimeMs() const { return elapsedTime; }
    QVector<QString> getOpenTableStrings() const;
    QVector<QString> getClosedTableStrings() const;
    QVector<std::array<int, 9>> getSolutionPath() const;

public slots:
    // 在后台线程中调用：使用 Qt 支持的类型（QVector<int>）作为参数，避免元对象问题
    void startSolve(const QVector<int>& startVec, const QVector<int>& goalVec);

signals:
    void openTableChanged();
    void closedTableChanged();
    // 新：后台求解完成后的信号，包含是否成功、路径以及统计数据
    void solveFinished(bool success, QVector<QVector<int>> path, int expandedCount, int generatedCount, qint64 elapsedTime);

private:
    int heuristicType;
    std::array<int, 9> goalState;
    StateNode* goalNode;
    int expandedCount;
    int generatedCount;
    qint64 elapsedTime;

    std::priority_queue<StateNode*, std::vector<StateNode*>, CompareNode> openList;
    std::unordered_set<StateNode> closedSet;
    std::vector<StateNode*> allNodes; // 所有动态分配的节点，用于析构释放

    int computeHeuristic(const std::array<int, 9>& state);
    std::vector<StateNode*> expandNode(StateNode* node);
    bool isGoal(const std::array<int, 9>& state);
    void clear();
};