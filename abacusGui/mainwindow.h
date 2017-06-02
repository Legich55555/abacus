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
    void on_executeButton_clicked(bool checked);
    void on_sourceEditor_cursorPositionChanged();
    void on_sourceEditor_textChanged();

    void on_allDone();
    void on_batchQueued(unsigned firstTaskIdx, unsigned tasksNumber);
    void on_taskDone(unsigned taskIdx, const QString& taskResult);
    void on_taskCancelled(unsigned taskIdx);

private:

    void setTaskStatus(unsigned taskIdx, const QString taskResult);

    Ui::MainWindow *ui;
    int m_lastChangedBlockIdx;
    ExecQueue m_execQueue;
};

#endif // MAINWINDOW_H
