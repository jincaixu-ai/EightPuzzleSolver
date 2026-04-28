#pragma execution_character_set("utf-8")
#include "AStarSolver.h"
#include <cmath>
#include <QDebug>

// 辅助函数：曼哈顿距离
static int manhattan(const std::array<int, 9>& state) {
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

// 错位数
static int misplaced(const std::array<int, 9>& state) {
    int cnt = 0;
    for (int i = 0; i < 9; ++i) {
        if (state[i] != 0 && state[i] != i + 1) cnt++;
    }
    return cnt;
}

// 欧几里得距离（取整）
static int euclidean(const std::array<int, 9>& state) {
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

AStarSolver::AStarSolver(QObject* parent) : QObject(parent), heuristicType(0), goalNode(nullptr) {
    goalState = { 1,2,3,4,5,6,7,8,0 };
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
    case 0: return manhattan(state);
    case 1: return misplaced(state);
    case 2: return euclidean(state);
    default: return manhattan(state);
    }
}

bool AStarSolver::isGoal(const std::array<int, 9>& state) {
    return state == goalState;
}

std::vector<StateNode*> AStarSolver::expandNode(StateNode* node) {
    std::vector<StateNode*> succ;
    int zp = node->zeroPos;
    int row = zp / 3, col = zp % 3;
    // 四个方向：上、下、左、右
    int dr[] = { -1, 1, 0, 0 };
    int dc[] = { 0, 0, -1, 1 };
    QString moves[] = { "上", "下", "左", "右" };
    for (int i = 0; i < 4; ++i) {
        int nr = row + dr[i];
        int nc = col + dc[i];
        if (nr >= 0 && nr < 3 && nc >= 0 && nc < 3) {
            int newPos = nr * 3 + nc;
            std::array<int, 9> newState = node->state;
            std::swap(newState[zp], newState[newPos]);
            int newG = node->g + 1;
            int newH = computeHeuristic(newState);
            StateNode* child = new StateNode(newState, newG, newH, node, newPos);
            child->move = moves[i];
            succ.push_back(child);
            allNodes.push_back(child);
            generatedCount++;
        }
    }
    return succ;
}

bool AStarSolver::solve(const std::array<int, 9>& start, const std::array<int, 9>& goal) {
    clear();
    goalState = goal;
    QElapsedTimer timer;
    timer.start();

    // 初始节点
    int startH = computeHeuristic(start);
    StateNode* startNode = new StateNode(start, 0, startH, nullptr);
    // 找空格位置
    for (int i = 0; i < 9; ++i) if (start[i] == 0) { startNode->zeroPos = i; break; }
    allNodes.push_back(startNode);
    openList.push(startNode);
    generatedCount = 1;

    while (!openList.empty()) {
        StateNode* cur = openList.top();
        openList.pop();

        if (closedSet.find(*cur) != closedSet.end()) continue;
        closedSet.insert(*cur);
        expandedCount++;
        emit closedTableChanged();

        if (isGoal(cur->state)) {
            goalNode = cur;
            elapsedTime = timer.elapsed();
            return true;
        }

        auto children = expandNode(cur);
        for (auto child : children) {
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

void AStarSolver::startSolve(const QVector<int>& startVec, const QVector<int>& goalVec) {
    // 将 QVector<int> 转成 std::array<int,9>
    std::array<int, 9> startState;
    std::array<int, 9> goalStateArr;
    for (int i = 0; i < 9 && i < startVec.size(); ++i) startState[i] = startVec[i];
    for (int i = 0; i < 9 && i < goalVec.size(); ++i) goalStateArr[i] = goalVec[i];

    bool ok = solve(startState, goalStateArr);

    // 将解路径转换为 QVector<QVector<int>>，以便安全通过信号传递到主线程
    QVector<QVector<int>> pathVec;
    if (ok) {
        auto path = getSolutionPath();
        for (const auto& arr : path) {
            QVector<int> step;
            for (int i = 0; i < 9; ++i) step.append(arr[i]);
            pathVec.append(step);
        }
    }

    // 发出完成信号（包含统计信息）
    emit solveFinished(ok, pathVec, expandedCount, generatedCount, elapsedTime);
}
QVector<QString> AStarSolver::getOpenTableStrings() const {
    QVector<QString> lines;
    if (openList.empty()) {
        lines << "OPEN表为空";
        return lines;
    }

    // 复制 priority_queue 到 vector（只能通过不断pop复制，但会破坏原队列，所以使用临时副本）
    // 由于 priority_queue 没有迭代器，只能通过弹出复制，但原队列是 const 方法不能修改，所以需要另一种方式：
    // 实际上我们可以直接访问底层容器，但 priority_queue 的底层容器是受保护的。建议使用 std::multiset 或手动维护 vector。
    // 一个简单但低效的做法：使用一个临时优先队列复制原队列。
    std::priority_queue<StateNode*, std::vector<StateNode*>, CompareNode> temp = openList;
    std::vector<StateNode*> nodes;
    while (!temp.empty()) {
        nodes.push_back(temp.top());
        temp.pop();
    }
    // 按 f 值降序或升序显示（已有序，但pop出来是降序？CompareNode定义的是大的优先级小，所以top是最小f，pop后从最小到最大）
    // 为了显示清晰，我们保留从最小 f 到最大 f 的顺序，直接遍历 nodes 即可（已经是从小到大）
    for (auto node : nodes) {
        QString stateStr;
        for (int i = 0; i < 9; ++i) stateStr += QString::number(node->state[i]);
        lines << QString("状态: %1  f=%2  g=%3  h=%4").arg(stateStr).arg(node->f).arg(node->g).arg(node->h);
    }
    return lines;
}

QVector<QString> AStarSolver::getClosedTableStrings() const {
    QVector<QString> lines;
    for (const auto& node : closedSet) {
        QString s;
        for (int v : node.state) s += QString::number(v);
        lines << QString("状态: %1  f=%2").arg(s).arg(node.f);
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