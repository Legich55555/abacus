#include "ExecQueue.h"

#include <chrono>

#include "exprCalc/ExprCalc.h"

struct ExecQueue::Task
{
    bool IsDone;
    bool IsSuccessfull;
    unsigned Idx;
    QString Statement;
    QString Output;
    Abacus::State State;
};

ExecQueue::ExecQueue()
    : m_destroying(false),
      m_execThread(&ExecQueue::ExecLoop, this)
{
}

ExecQueue::~ExecQueue()
{
    m_destroying = true;
    m_execThread.join();
}

void ExecQueue::AddBatch(const std::vector<QString>& batch, unsigned firstTaskIdx)
{
    {
        std::lock_guard<std::mutex> lock(m_mtx);

        CancelTasksImpl(firstTaskIdx);

        for (unsigned i = 0; i < batch.size(); ++i)
        {
            m_waitingTasks.push_back(TaskPtr(new Task { false, false, i + firstTaskIdx, batch[i]}));
        }

    }

    m_wakeup.notify_one();

    emit BatchQueued(firstTaskIdx, batch.size());
}

void ExecQueue::CancelTasks(unsigned fromTaskIdx)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    CancelTasksImpl(fromTaskIdx);
}

void ExecQueue::ExecLoop()
{
    while (!m_destroying)
    {
        TaskPtr currTask;

        {
            std::unique_lock<std::mutex> lock(m_mtx);

            if (!m_waitingTasks.empty())
            {
                currTask = std::move(m_waitingTasks.front());
                m_waitingTasks.pop_front();
            }
            else
            {
                m_wakeup.wait_for(lock, std::chrono::milliseconds(150U));
            }
        }

        if (currTask != nullptr)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100U));

            Abacus::State state = m_doneTasks.empty() ?
                        Abacus::State() : m_doneTasks.back().get()->State;

            Task& task = *currTask.get();

            Abacus::ExecResult taskResult = Abacus::Execute(task.Statement.toStdString(), state);
            task.IsDone = true;
            task.IsSuccessfull = taskResult.Success;
            task.State.insert(state.cbegin(), state.cend());
            task.State.insert(taskResult.Variables.cbegin(), taskResult.Variables.cend());

            task.Output = task.IsSuccessfull ? "Ok" : "Error";
            for (const auto& s : taskResult.Output)
            {
                task.Output.append(s.c_str());
                task.Output.append("; ");
            }

            {
                std::lock_guard<std::mutex> lock(m_mtx);

                m_doneTasks.push_back(std::move(currTask));

                emit TaskDone(task.Idx, task.IsSuccessfull, task.Statement, task.Output);

                if (m_doneTasks.empty())
                {
                    emit AllDone();
                }
            }
        }
    }
}

void ExecQueue::CancelTasksImpl(unsigned fromTaskIdx)
{
    while (!m_waitingTasks.empty() && m_waitingTasks.back()->Idx >= fromTaskIdx)
    {
        emit TaskCancelled(m_waitingTasks.back()->Idx);
        m_waitingTasks.pop_back();
    }

    while (!m_doneTasks.empty() && m_doneTasks.back()->Idx >= fromTaskIdx)
    {
        emit TaskCancelled(m_doneTasks.back()->Idx);
        m_doneTasks.pop_back();
    }
}
