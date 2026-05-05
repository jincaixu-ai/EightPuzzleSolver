#include "AStarSolver.h"
#include "KnowledgeBase.h"
#include <QDebug>

AStarSolver::AStarSolver(QObject* parent) : QObject(parent), heuristicType(0), goalNode(nullptr) {
    goalState = { 1,2,3,4,5,6,7,8,0 }; // default, will be overwritten
}

AStarSolver::~AStarSolver() {
    clear();
}

void AStarSolver::clear() {
    for (auto node : allNodes) delete node;
    allNodes.clear();
    while (!openList.empty()) openList.pop();
    closedSet.clear();
    goalNode = nullptr;
    expandedCount = 0;
    generatedCount = 0;
    elapsedTime = 0;
}

void AStarSolver::setHeuristicType(int type) {
    heuristicType = type;
}

int AStarSolver::computeHeuristic(const std::array<int, 9>& state) {
    switch (heuristicType) {
    case 0: return KnowledgeBase::manhattan(state);
    case 1: return KnowledgeBase::misplaced(state);
    case 2: return KnowledgeBase::euclidean(state);
    default: return KnowledgeBase::manhattan(state);
    }
}

bool AStarSolver::isGoal(const std::array<int, 9>& state) {
    return KnowledgeBase::isGoal(state, goalState);
}

bool AStarSolver::solve(const std::array<int, 9>& start, const std::array<int, 9>& goal) {
    clear();
    goalState = goal;
    QElapsedTimer timer;
    timer.start();

    // Initial node
    int startH = computeHeuristic(start);
    StateNode* startNode = new StateNode(start, 0, startH, nullptr);
    // Find zero position
    for (int i = 0; i < 9; ++i) if (start[i] == 0) { startNode->zeroPos = i; break; }
    allNodes.push_back(startNode);
    openList.push(startNode);
    generatedCount = 1;

    while (!openList.empty()) {
        StateNode* current = openList.top();
        openList.pop();

        if (closedSet.find(*current) != closedSet.end()) continue;
        closedSet.insert(*current);
        expandedCount++;
        emit closedTableChanged();

        if (isGoal(current->state)) {
            goalNode = current;
            elapsedTime = timer.elapsed();
            reconstructPath(goalNode);
            return true;
        }

        // Use knowledge base to expand
        std::vector<StateNode*> children = KnowledgeBase::expand(current);
        for (auto child : children) {
            // compute heuristic for child (because expand only sets g and parent)
            child->h = computeHeuristic(child->state);
            child->f = child->g + child->h;
            allNodes.push_back(child);
            generatedCount++;
            if (closedSet.find(*child) != closedSet.end()) {
                continue;
            }
            openList.push(child);
        }
        emit openTableChanged();
    }

    elapsedTime = timer.elapsed();
    return false;
}

void AStarSolver::reconstructPath(StateNode* node) {
    // Path is stored implicitly via parent pointers.
    // In getSolutionPath() we traverse from goalNode to start.
}

QVector<QString> AStarSolver::getOpenTableStrings() const {
    QVector<QString> lines;
    // Since priority_queue is not iterable, we return a placeholder.
    // In a full implementation you would copy the queue and extract states.
    lines << "Open table content is displayed dynamically during search.";
    return lines;
}

QVector<QString> AStarSolver::getClosedTableStrings() const {
    QVector<QString> lines;
    for (const auto& node : closedSet) {
        QString stateStr;
        for (int v : node.state) stateStr += QString::number(v);
        lines << QString("State: %1  f=%2").arg(stateStr).arg(node.f);
    }
    return lines;
}

QVector<std::array<int, 9>> AStarSolver::getSolutionPath() const {
    QVector<std::array<int, 9>> path;
    if (!goalNode) return path;
    StateNode* cur = goalNode;
    while (cur) {
        path.prepend(cur->state);
        cur = cur->parent;
    }
    return path;
}