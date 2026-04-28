#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_MainWindow.h"      // 包含生成的 UI 类定义
#include "AStarSolver.h"
#include <array>

QT_BEGIN_NAMESPACE
namespace Ui { class EightPuzzleSolverClass; }   // 匹配实际生成的类名
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

private:
    Ui::EightPuzzleSolverClass* ui;   // 修改指针类型
    AStarSolver* solver;
    std::array<int, 9> currentState;
    std::array<int, 9> goalState;

    void updateBoardDisplay();
    void setState(const std::array<int, 9>& state);
    bool isSolvable(const std::array<int, 9>& state);
    std::array<int, 9> generateRandomSolvableState(int steps = 200);
    void displayStatistics();
};

#endif // MAINWINDOW_H