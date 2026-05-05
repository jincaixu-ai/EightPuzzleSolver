#include "MainWindow.h"
#include "KnowledgeBase.h"
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <random>
#include <QThread>
#include <QApplication>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::EightPuzzleSolverClass)
    , solver(nullptr)
{
    ui->setupUi(this);

    // Default target state: ordered 1..8 with blank at end
    targetState = { 1,2,3,4,5,6,7,8,0 };

    // Create target input controls (if not already in .ui)
    QLabel* labelTarget = new QLabel("Target:", this);
    QLineEdit* lineTarget = new QLineEdit(this);
    lineTarget->setObjectName("lineTarget");
    lineTarget->setPlaceholderText("9 digits 0-8 no repeat");
    lineTarget->setText("123456780");
    QPushButton* btnSetTarget = new QPushButton("Set", this);
    btnSetTarget->setObjectName("btnSetTarget");

    // Position them (adjust coordinates as needed)
    labelTarget->setGeometry(500, 340, 50, 25);
    lineTarget->setGeometry(550, 340, 120, 25);
    btnSetTarget->setGeometry(680, 340, 60, 25);

    connect(btnSetTarget, &QPushButton::clicked, this, &MainWindow::onSetTarget);

    // Set button texts
    ui->btnRandomInit->setText("Random");
    ui->btnSolve->setText("Solve");
    ui->btnReset->setText("Reset");
    if (ui->btnBenchmark) ui->btnBenchmark->setText("Benchmark");

    // Statistics labels
    ui->labelExpanded->setText("Expanded: 0");
    ui->labelGenerated->setText("Generated: 0");
    ui->labelTime->setText("Time: 0 ms");
    ui->labelExpanded->setMinimumWidth(90);
    ui->labelGenerated->setMinimumWidth(90);
    ui->labelTime->setMinimumWidth(80);

    // Solver
    solver = new AStarSolver(this);
    connect(solver, &AStarSolver::openTableChanged, this, &MainWindow::updateOpenTableDisplay);
    connect(solver, &AStarSolver::closedTableChanged, this, &MainWindow::updateClosedTableDisplay);

    // Button connections
    connect(ui->btnRandomInit, &QPushButton::clicked, this, &MainWindow::onBtnRandomInit);
    connect(ui->btnSolve, &QPushButton::clicked, this, &MainWindow::onBtnSolve);
    connect(ui->btnReset, &QPushButton::clicked, this, &MainWindow::onBtnReset);
    connect(ui->comboHeuristic, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onHeuristicChanged);
    if (ui->btnBenchmark) {
        connect(ui->btnBenchmark, &QPushButton::clicked, this, &MainWindow::onBtnBenchmark);
    }

    // Initial board = target
    currentState = targetState;
    updateBoardDisplay();
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

std::array<int, 9> MainWindow::generateRandomSolvableStateFromTarget(int steps)
{
    std::array<int, 9> state = targetState;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dir(0, 3);
    int zeroPos = -1;
    for (int i = 0; i < 9; ++i) if (state[i] == 0) { zeroPos = i; break; }
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
    currentState = generateRandomSolvableStateFromTarget(200);
    updateBoardDisplay();
}

void MainWindow::onBtnReset()
{
    currentState = targetState;
    updateBoardDisplay();
    ui->textOpen->clear();
    ui->textClosed->clear();
    ui->labelExpanded->setText("Expanded: 0");
    ui->labelGenerated->setText("Generated: 0");
    ui->labelTime->setText("Time: 0 ms");
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
    ui->textOpen->append(QString("Open size: %1").arg(solver->getGeneratedCount() - solver->getExpandedCount()));
}

void MainWindow::updateClosedTableDisplay()
{
    auto lines = solver->getClosedTableStrings();
    ui->textClosed->clear();
    for (const QString& line : lines) ui->textClosed->append(line);
}

void MainWindow::onBtnSolve()
{
    if (!KnowledgeBase::isSolvable(currentState, targetState)) {
        QMessageBox::warning(this, "Unsolvable", "Start state cannot reach target state.");
        return;
    }

    ui->btnSolve->setEnabled(false);
    ui->textOpen->clear();
    ui->textClosed->clear();

    bool success = solver->solve(currentState, targetState);

    if (success) {
        auto path = solver->getSolutionPath();
        for (const auto& s : path) {
            setState(s);
            QThread::msleep(150);
        }
        QMessageBox::information(this, "Done", "Solution found!");
    }
    else {
        QMessageBox::information(this, "Failed", "No solution found.");
    }

    displayStatistics();
    ui->btnSolve->setEnabled(true);
}

void MainWindow::displayStatistics()
{
    ui->labelExpanded->setText(QString("Expanded: %1").arg(solver->getExpandedCount()));
    ui->labelGenerated->setText(QString("Generated: %1").arg(solver->getGeneratedCount()));
    ui->labelTime->setText(QString("Time: %1 ms").arg(solver->getElapsedTimeMs()));
}

void MainWindow::onBtnBenchmark()
{
    auto testState = currentState;
    if (!KnowledgeBase::isSolvable(testState, targetState)) {
        QMessageBox::warning(this, "Error", "Current state unsolvable for benchmark.");
        return;
    }

    QString report;
    report += "Heuristic,Expanded,Generated,Time(ms)\n";

    for (int h = 0; h < 3; ++h) {
        solver->setHeuristicType(h);
        bool ok = solver->solve(testState, targetState);
        if (ok) {
            report += QString("%1,%2,%3,%4\n")
                .arg(h == 0 ? "Manhattan" : h == 1 ? "Misplaced" : "Euclidean")
                .arg(solver->getExpandedCount())
                .arg(solver->getGeneratedCount())
                .arg(solver->getElapsedTimeMs());
        }
        else {
            report += QString("%1,unsolvable,unsolvable,unsolvable\n")
                .arg(h == 0 ? "Manhattan" : h == 1 ? "Misplaced" : "Euclidean");
        }
    }

    QString fileName = "benchmark.csv";
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << report;
        file.close();
        QMessageBox::information(this, "Done", QString("Benchmark saved to %1").arg(fileName));
    }
    else {
        QMessageBox::warning(this, "Error", "Cannot save file.");
    }
}

bool MainWindow::parseTargetString(const QString& text, std::array<int, 9>& outState)
{
    if (text.length() != 9) return false;
    QSet<int> seen;
    for (int i = 0; i < 9; ++i) {
        bool ok;
        int val = text.mid(i, 1).toInt(&ok);
        if (!ok || val < 0 || val > 8 || seen.contains(val)) return false;
        outState[i] = val;
        seen.insert(val);
    }
    return true;
}

void MainWindow::onSetTarget()
{
    QLineEdit* lineTarget = findChild<QLineEdit*>("lineTarget");
    if (!lineTarget) return;
    QString text = lineTarget->text().trimmed();
    std::array<int, 9> newTarget;
    if (!parseTargetString(text, newTarget)) {
        QMessageBox::warning(this, "Invalid", "Must be 9 digits 0-8 without repetition.");
        return;
    }
    targetState = newTarget;
    QMessageBox::information(this, "Success", "Target updated.");
    currentState = targetState;
    updateBoardDisplay();
    ui->textOpen->clear();
    ui->textClosed->clear();
    ui->labelExpanded->setText("Expanded: 0");
    ui->labelGenerated->setText("Generated: 0");
    ui->labelTime->setText("Time: 0 ms");
}