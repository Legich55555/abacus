#include "ExecQueue.h"

#include <chrono>

#include "exprCalc/ExprCalc.h"

struct ExecQueue::Task
{
    bool IsSuccessfull;
    unsigned Idx;
    QString Statement;
    QString Preview;
    QString Output;
    Abacus::State State;
};

ExecQueue::ExecQueue()
    : m_destroying(false),
      m_cancellingCurrentTask(false),
      m_execThread(&ExecQueue::ExecLoop, this)
{
}

ExecQueue::~ExecQueue()
{
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_destroying = true;
    }

    m_execThread.join();
}

void ExecQueue::AddBatch(const std::vector<QString>& batch, unsigned firstTaskIdx)
{
    {
        std::lock_guard<std::mutex> lock(m_mtx);

        CancelTasksImpl(firstTaskIdx);

        for (unsigned i = 0; i < batch.size(); ++i)
        {
//            bool IsSuccessfull;
//            unsigned Idx;
//            QString Statement;
//            QString Preview;
//            QString Output;
//            Abacus::State State;


            m_waitingTasks.push_back(TaskPtr(new Task { false, i + firstTaskIdx, batch[i], {}, {}, {}}));
        }

    }

    m_wakeup.notify_one();

    emit BatchQueued(firstTaskIdx, static_cast<unsigned>(batch.size()));
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
                m_cancellingCurrentTask = false;
            }
            else
            {
                m_wakeup.wait_for(lock, std::chrono::milliseconds(150U));
            }
        }

        if (currTask != nullptr)
        {
            Abacus::State state = m_doneTasks.empty() ?
                        Abacus::State() : m_doneTasks.back().get()->State;

            Task& task = *currTask.get();

            auto isCancelling = std::bind(&ExecQueue::IsCancelling, this);

            Abacus::ExecResult taskResult = Abacus::Execute(task.Statement.toStdString(), state, isCancelling);
            task.IsSuccessfull = taskResult.Success;
            task.Preview = task.IsSuccessfull ? "Ok. " : "Error. ";

            // Merge state variables
            task.State.insert(state.cbegin(), state.cend());
            task.State.insert(taskResult.Variables.cbegin(), taskResult.Variables.cend());

            if (!taskResult.Output.empty())
            {
                task.Preview.append("Out: ");
                for (const auto& s : taskResult.Output)
                {
                    task.Preview.append(s.c_str());
                    task.Preview.append(", ");
                    task.Output.append(s.c_str());
                }
            }

            if (!task.State.empty())
            {
                task.Preview.append("Vars: ");
                for (const auto& v : task.State)
                {
                    QString varName(v.first.c_str());
                    QString varValue(v.second.ToString().c_str());

                    task.Preview.append(QString("%1: %2, ")
                                       .arg(varName)
                                       .arg(varValue));
                }
            }

            if (!m_cancellingCurrentTask)
            {
                std::lock_guard<std::mutex> lock(m_mtx);

                m_doneTasks.push_back(std::move(currTask));

                emit TaskDone(task.Idx, task.IsSuccessfull, task.Statement, task.Preview);

                if (m_waitingTasks.empty())
                {
                    QString allOutput;
                    for (const auto& task : m_doneTasks)
                    {
                        if (task.get()->IsSuccessfull)
                        {
                            allOutput.append(task.get()->Output);
                        }
                    }

                    emit AllDone(allOutput);
                }
            }
        }
    }
}

void ExecQueue::CancelTasksImpl(unsigned fromTaskIdx)
{
    // This function must be called under lock on m_mtx.

    while (!m_waitingTasks.empty() && m_waitingTasks.back()->Idx >= fromTaskIdx)
    {
        emit TaskCancelled(m_waitingTasks.back()->Idx);
        m_waitingTasks.pop_back();
    }

    if ((0 == fromTaskIdx) ||
        ((m_doneTasks.size() - fromTaskIdx) == 0))
    {
        qInfo("Cancelled current task %d", fromTaskIdx);
        m_cancellingCurrentTask = true;
    }

    while (!m_doneTasks.empty() && m_doneTasks.back()->Idx >= fromTaskIdx)
    {
        emit TaskCancelled(m_doneTasks.back()->Idx);
        m_doneTasks.pop_back();
    }
}

bool ExecQueue::IsCancelling() const
{
    std::lock_guard<std::mutex> lock(m_mtx);
    return m_destroying || m_cancellingCurrentTask;
}
