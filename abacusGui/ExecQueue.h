#pragma once
#ifndef ABACUSUI_EXECQUEUE_H
#define ABACUSUI_EXECQUEUE_H

#include <QObject>
#include <QString>

#include <list>
#include <vector>
#include <mutex>
#include <thread>
#include <memory>
#include <condition_variable>


class ExecQueue : public QObject
{
    Q_OBJECT

public:
    ExecQueue();
    ~ExecQueue();

    //void AddTask(int taskIdx, const QString& taskStatement);
    void AddBatch(const std::vector<QString>& batch, unsigned firstTaskIdx);
    void CancelTasks(unsigned fromTaskIdx);

    ExecQueue(const ExecQueue&) = delete;
    ExecQueue& operator=(ExecQueue&) = delete;

signals:
    void AllDone();
    void BatchQueued(unsigned firstTaskIdx, unsigned tasksNumber);
    void TaskDone(unsigned taskIdx, const QString& taskResult);
    void TaskCancelled(unsigned taskIdx);

private:

    void ExecLoop();
    void CancelTasksImpl(unsigned fromTaskIdx);

    struct Task;
    typedef std::unique_ptr<Task> TaskPtr;

    std::list<TaskPtr> m_doneTasks;
    std::list<TaskPtr> m_waitingTasks;

    volatile bool m_destroying;
    std::mutex m_mtx;
    std::condition_variable m_wakeup;
    std::thread m_execThread;
};

#endif // ABACUSUI_EXECQUEUE_H
