#pragma execution_character_set("utf-8")
#include "MainWindow.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <random>
#include <QThread>
#include <QApplication>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::EightPuzzleSolverClass)   // 修改此处
    , solver(nullptr)
{
    ui->setupUi(this);

    // 主窗口大小
    this->resize(650, 700);

    // 棋盘区域 (假设左边缘 20, 顶边缘 40)
    // 控制按钮区域
   
    ui->btnRandomInit->setGeometry(320, 80, 110, 35);
    ui->btnSolve->setGeometry(440, 80, 70, 35);
    ui->btnReset->setGeometry(520, 80, 70, 35);


    // 底部统计标签
    ui->labelExpanded->setGeometry(30, 540, 140, 25);
    ui->labelGenerated->setGeometry(180, 540, 160, 25);   // 宽度加大
    ui->labelTime->setGeometry(360, 540, 120, 25);
    ui->btnBenchmark->setGeometry(520, 540, 100, 30);

    // 设置文字
    ui->btnRandomInit->setText("随机初始化");
    ui->btnSolve->setText("求解");
    ui->btnReset->setText("重置");
    ui->labelExpanded->setText("扩展节点数: 0");
    ui->labelGenerated->setText("生成节点总数: 0");
    ui->labelTime->setText("执行时间: 0 ms");
    if (ui->btnBenchmark) ui->btnBenchmark->setText("性能对比");

    goalState = { 1,2,3,4,5,6,7,8,0 };
    currentState = goalState;
    updateBoardDisplay();

    solver = new AStarSolver(this);
    connect(solver, &AStarSolver::openTableChanged, this, &MainWindow::updateOpenTableDisplay);
    connect(solver, &AStarSolver::closedTableChanged, this, &MainWindow::updateClosedTableDisplay);

    // 按钮连接
    connect(ui->btnRandomInit, &QPushButton::clicked, this, &MainWindow::onBtnRandomInit);
    connect(ui->btnSolve, &QPushButton::clicked, this, &MainWindow::onBtnSolve);
    connect(ui->btnReset, &QPushButton::clicked, this, &MainWindow::onBtnReset);
       if (ui->btnBenchmark) {
        connect(ui->btnBenchmark, &QPushButton::clicked, this, &MainWindow::onBtnBenchmark);
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateBoardDisplay()
{
    for (int i = 0; i < 9; ++i) {
        QString btnName = QString("btn%1").arg(i);
        QPushButton* btn = findChild<QPushButton*>(btnName);
        if (btn) {
            int val = currentState[i];
            btn->setText(val == 0 ? "" : QString::number(val));
        }
    }
}

void MainWindow::setState(const std::array<int, 9>& state)
{
    currentState = state;
    updateBoardDisplay();
    QApplication::processEvents();
}

bool MainWindow::isSolvable(const std::array<int, 9>& state)
{
    int inv = 0;
    for (int i = 0; i < 9; ++i) {
        if (state[i] == 0) continue;
        for (int j = i + 1; j < 9; ++j) {
            if (state[j] != 0 && state[i] > state[j]) inv++;
        }
    }
    return (inv % 2 == 0);
}

std::array<int, 9> MainWindow::generateRandomSolvableState(int steps)
{
    std::array<int, 9> state = goalState;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dir(0, 3);
    int zeroPos = 8;
    for (int s = 0; s < steps; ++s) {
        int row = zeroPos / 3, col = zeroPos % 3;
        int d = dir(gen);
        int nr = row, nc = col;
        switch (d) {
        case 0: nr--; break;
        case 1: nr++; break;
        case 2: nc--; break;
        case 3: nc++; break;
        }
        if (nr >= 0 && nr < 3 && nc >= 0 && nc < 3) {
            int newPos = nr * 3 + nc;
            std::swap(state[zeroPos], state[newPos]);
            zeroPos = newPos;
        }
    }
    return state;
}

void MainWindow::onBtnRandomInit()
{
    currentState = generateRandomSolvableState(200);
    updateBoardDisplay();
}

void MainWindow::onBtnReset()
{
    currentState = goalState;
    updateBoardDisplay();
    ui->textOpen->clear();
    ui->textClosed->clear();
    ui->labelExpanded->setText("扩展节点数: 0");
    ui->labelGenerated->setText("生成节点数: 0");
    ui->labelTime->setText("执行时间: 0 ms");
}

void MainWindow::onHeuristicChanged(int index)
{
    solver->setHeuristicType(index);
}

void MainWindow::updateOpenTableDisplay()
{
    auto lines = solver->getOpenTableStrings();
    ui->textOpen->clear();
    for (const QString& line : lines) ui->textOpen->append(line);
    ui->textOpen->append(QString("当前OPEN表节点数: %1")
        .arg(solver->getGeneratedCount() - solver->getExpandedCount()));
}

void MainWindow::updateClosedTableDisplay()
{
   
       auto openLines = solver->getOpenTableStrings();
       auto closedLines = solver->getClosedTableStrings();
       ui->textClosed->clear();
       ui->textClosed->append("========== OPEN 表 ==========");
       for (const auto& line : openLines) ui->textClosed->append(line);
        ui->textClosed->append("========== CLOSED 表 ==========");
       for (const auto& line : closedLines) ui->textClosed->append(line);
}
void MainWindow::onBtnSolve()
{
    if (!isSolvable(currentState)) {
        QMessageBox::warning(this, "不可解", "当前状态无解，请重新随机初始化。");
        return;
    }

    ui->btnSolve->setEnabled(false);
    ui->textOpen->clear();
    ui->textClosed->clear();

    bool success = solver->solve(currentState, goalState);

    if (success) {
        auto path = solver->getSolutionPath();
        for (const auto& s : path) {
            setState(s);
            QThread::msleep(150);
        }
        QMessageBox::information(this, "完成", "已找到解路径！");
    }
    else {
        QMessageBox::information(this, "无解", "算法未能找到解");
    }

    displayStatistics();
    ui->btnSolve->setEnabled(true);
}

void MainWindow::displayStatistics()
{
    ui->labelExpanded->setText(QString("扩展节点数: %1").arg(solver->getExpandedCount()));
    ui->labelGenerated->setText(QString("生成节点总数: %1").arg(solver->getGeneratedCount()));
    ui->labelTime->setText(QString("执行时间: %1 ms").arg(solver->getElapsedTimeMs()));
}

void MainWindow::onBtnBenchmark()
{
    auto testState = currentState;
    if (!isSolvable(testState)) {
        QMessageBox::warning(this, "错误", "当前状态不可解，无法进行性能对比。");
        return;
    }

    QString report;
    report += "启发式函数,扩展节点数,生成节点总数,耗时(ms)\n";

    for (int h = 0; h < 3; ++h) {
        solver->setHeuristicType(h);
        bool ok = solver->solve(testState, goalState);
        if (ok) {
            report += QString("%1,%2,%3,%4\n")
                .arg(h == 0 ? "曼哈顿距离" : h == 1 ? "错位数" : "欧几里得距离")
                .arg(solver->getExpandedCount())
                .arg(solver->getGeneratedCount())
                .arg(solver->getElapsedTimeMs());
        }
        else {
            report += QString("%1,无解,无解,无解\n")
                .arg(h == 0 ? "曼哈顿距离" : h == 1 ? "错位数" : "欧几里得距离");
        }
    }

    QString fileName = "benchmark.csv";
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << report;
        file.close();
        QMessageBox::information(this, "完成", QString("性能对比数据已保存到 %1").arg(fileName));
    }
    else {
        QMessageBox::warning(this, "错误", "无法保存文件，请检查目录权限。");
    }
}