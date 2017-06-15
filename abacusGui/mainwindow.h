#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ExecQueue.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_sourceEditor_cursorPositionChanged();
    void on_sourceEditor_textChanged();

    void on_allDone(const QString& output);
    void on_batchQueued(unsigned firstTaskIdx, unsigned tasksNumber);
    void on_taskDone(unsigned taskIdx, bool success, const QString& statement, const QString& result);
    void on_taskCancelled(unsigned taskIdx);

    void on_actionOpen_triggered();

    void on_actionSave_triggered();

private:

    void setTaskStatus(unsigned taskIdx, const QString taskResult);

    Ui::MainWindow *ui;
    int m_lastChangedBlockIdx;
    ExecQueue m_execQueue;
};

#endif // MAINWINDOW_H
