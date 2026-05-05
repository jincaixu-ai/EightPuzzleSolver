#ifndef ASTARSOLVER_H
#define ASTARSOLVER_H

#include <QObject>
#include <QElapsedTimer>
#include <unordered_set>
#include <queue>
#include <vector>
#include "StateNode.h"

struct CompareNode {
    bool operator()(const StateNode* a, const StateNode* b) {
        return a->f > b->f;  // min-heap based on f
    }
};

class AStarSolver : public QObject {
    Q_OBJECT
public:
    explicit AStarSolver(QObject* parent = nullptr);
    ~AStarSolver();

    void setHeuristicType(int type); // 0: Manhattan, 1: Misplaced, 2: Euclidean

    // Core A* algorithm
    bool solve(const std::array<int, 9>& start, const std::array<int, 9>& goal);

    // Statistics
    int getExpandedCount() const { return expandedCount; }
    int getGeneratedCount() const { return generatedCount; }
    qint64 getElapsedTimeMs() const { return elapsedTime; }

    // For displaying OPEN/CLOSED tables (simplified, can be extended)
    QVector<QString> getOpenTableStrings() const;
    QVector<QString> getClosedTableStrings() const;
    QVector<std::array<int, 9>> getSolutionPath() const;

signals:
    void openTableChanged();
    void closedTableChanged();

private:
    int heuristicType;
    std::array<int, 9> goalState;
    StateNode* goalNode;
    int expandedCount;
    int generatedCount;
    qint64 elapsedTime;

    // Open list (priority queue by f)
    std::priority_queue<StateNode*, std::vector<StateNode*>, CompareNode> openList;
    // Closed set (hash set of expanded states)
    std::unordered_set<StateNode> closedSet;
    // All dynamically allocated nodes (for cleanup)
    std::vector<StateNode*> allNodes;

    int computeHeuristic(const std::array<int, 9>& state);
    bool isGoal(const std::array<int, 9>& state);
    void clear();
    void reconstructPath(StateNode* node);
};

#endif // ASTARSOLVER_H