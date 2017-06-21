#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
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

    void closeEvent (QCloseEvent *event);

private slots:
    void on_allDone(const QString& output);
    void on_batchQueued(unsigned firstTaskIdx, unsigned tasksNumber);
    void on_taskDone(unsigned taskIdx, bool success, const QString& statement, const QString& result);
    void on_taskCancelled(unsigned taskIdx);

    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();

    void on_sourceEditor_cursorPositionChanged();
    void on_sourceEditor_textChanged();

private:

    void setTaskStatus(unsigned taskIdx, const QString taskResult);
    void updateTextPosLabel();
    void save(const QString filename);
    void saveAs();
    bool checkAskAndSave();

    Ui::MainWindow *ui;

    int m_lastChangedBlockIdx;

    QString m_documentName;
    bool m_isDirty;

    ExecQueue m_execQueue;
};

#endif // MAINWINDOW_H
