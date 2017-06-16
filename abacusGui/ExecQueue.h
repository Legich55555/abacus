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

    void AddBatch(const std::vector<QString>& batch, unsigned firstTaskIdx);

    ExecQueue(const ExecQueue&) = delete;
    ExecQueue& operator=(ExecQueue&) = delete;

signals:
    void AllDone(const QString& output);
    void BatchQueued(unsigned firstTaskIdx, unsigned tasksNumber);
    void TaskDone(unsigned taskIdx, bool success, const QString& statement, const QString& result);
    void TaskCancelled(unsigned taskIdx);

private:

    void ExecLoop();
    void CancelTasksImpl(unsigned fromTaskIdx);
    bool IsCancelling() const;

    struct Task;
    typedef std::unique_ptr<Task> TaskPtr;

    std::list<TaskPtr> m_doneTasks;
    std::list<TaskPtr> m_waitingTasks;

    bool m_destroying;
    bool m_cancellingCurrentTask;

    mutable std::mutex m_mtx;
    std::condition_variable m_wakeup;
    std::thread m_execThread;
};

#endif // ABACUSUI_EXECQUEUE_H
