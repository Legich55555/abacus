#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QTextBlock>
#include <QTextCursor>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    m_lastChangedBlockIdx(0)
{
    ui->setupUi(this);

    connect(&m_execQueue, &ExecQueue::AllDone, this, &MainWindow::on_allDone);
    connect(&m_execQueue, &ExecQueue::BatchQueued, this, &MainWindow::on_batchQueued);
    connect(&m_execQueue, &ExecQueue::TaskDone, this, &MainWindow::on_taskDone);
    connect(&m_execQueue, &ExecQueue::TaskCancelled, this, &MainWindow::on_taskCancelled);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_executeButton_clicked(bool /*checked*/)
{

}

void MainWindow::on_sourceEditor_cursorPositionChanged()
{
}

void MainWindow::on_sourceEditor_textChanged()
{
    const int changedBlockIdx = ui->sourceEditor->textCursor().blockNumber();
    const int blockCount = ui->sourceEditor->blockCount();

    int fromTaskIdx = std::min(changedBlockIdx, m_lastChangedBlockIdx);

    std::vector<QString> statements;
    statements.reserve(blockCount - fromTaskIdx);

    for (int i = fromTaskIdx; i < blockCount; ++i)
    {
        QTextBlock block = ui->sourceEditor->document()->findBlockByNumber(i);
        Q_ASSERT(block.isValid());

        if (block.isValid())
        {
            QString blockText = block.text();
            statements.push_back(blockText);
            //m_execQueue.AddTask(i, blockText);
        }
        else
        {
            // It is unexpected behavior.
            break;
        }
    }

    m_execQueue.AddBatch(statements, fromTaskIdx);

    m_lastChangedBlockIdx = blockCount - 1;
}

void MainWindow::on_allDone()
{
    qInfo("on_allDone");
}

void MainWindow::on_batchQueued(unsigned firstTaskIdx, unsigned tasksNumber)
{
    qInfo("on_batchQueued");

    for (unsigned i = 0; i < tasksNumber; ++i)
    {
        setTaskStatus(i + firstTaskIdx, "Queued");
    }
}

void MainWindow::on_taskDone(unsigned taskIdx, bool success, const QString& statement, const QString& result)
{
    qInfo("on_taskDone");

    setTaskStatus(taskIdx, result);
}

void MainWindow::on_taskCancelled(unsigned taskIdx)
{
    qInfo("on_taskCancelled");

    if (static_cast<unsigned>(ui->sourceEditor->blockCount()) <= taskIdx &&
        static_cast<unsigned>(ui->resultViewer->blockCount()) > taskIdx)
    {
        QTextCursor cursor = ui->resultViewer->textCursor();

        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, taskIdx);
        cursor.select(QTextCursor::BlockUnderCursor);
        cursor.removeSelectedText();
    }
    else
    {
        setTaskStatus(taskIdx, "Cancelled");
    }
}

void MainWindow::setTaskStatus(unsigned taskIdx, const QString taskResult)
{
    if (taskIdx >= static_cast<unsigned>(ui->sourceEditor->blockCount()))
    {
        // There is no more source code for this task
        return;
    }

    const unsigned blockCount = static_cast<unsigned>(ui->resultViewer->blockCount());

    QTextCursor cursor = ui->resultViewer->textCursor();

    if (taskIdx < blockCount)
    {
        cursor.movePosition(QTextCursor::Start);
        cursor.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, taskIdx);
        cursor.select(QTextCursor::LineUnderCursor);
        cursor.removeSelectedText();
    }
    else
    {
        cursor.movePosition(QTextCursor::End);

        for (unsigned i = 0; i < (taskIdx - blockCount + 1); ++i)
        {
            cursor.insertBlock();
        }
    }

    cursor.insertText(taskResult);
}
