#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_MainWindow.h"
#include "AStarSolver.h"
#include <array>

QT_BEGIN_NAMESPACE
namespace Ui { class EightPuzzleSolverClass; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onBtnRandomInit();
    void onBtnSolve();
    void onBtnReset();
    void onHeuristicChanged(int index);
    void updateOpenTableDisplay();
    void updateClosedTableDisplay();
    void onBtnBenchmark();
    void onSetTarget();

private:
    Ui::EightPuzzleSolverClass* ui;
    AStarSolver* solver;
    std::array<int, 9> currentState;   // Current board
    std::array<int, 9> targetState;    // User-defined goal

    void updateBoardDisplay();
    void setState(const std::array<int, 9>& state);
    std::array<int, 9> generateRandomSolvableStateFromTarget(int steps = 200);
    void displayStatistics();
    bool parseTargetString(const QString& text, std::array<int, 9>& outState);
};

#endif // MAINWINDOW_H