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
            m_waitingTasks.push_back(TaskPtr(new Task { false, i + firstTaskIdx, batch[i], {}, {}, {}}));
        }
    }

    m_wakeup.notify_one();

    emit BatchQueued(firstTaskIdx, static_cast<unsigned>(batch.size()));
}

void ExecQueue::ExecLoop()
{
    auto isCancelling = std::bind(&ExecQueue::IsCancelling, this);

    while (!m_destroying)
    {
        TaskPtr currTask;
        Abacus::State state;

        {
            std::unique_lock<std::mutex> lock(m_mtx);

            if (!m_waitingTasks.empty())
            {
                currTask = std::move(m_waitingTasks.front());
                m_waitingTasks.pop_front();
                m_cancellingCurrentTask = false;

                state = m_doneTasks.empty() ?
                            Abacus::State() : m_doneTasks.back().get()->State;
            }
            else
            {
                m_wakeup.wait_for(lock, std::chrono::milliseconds(150U));
            }
        }

        if (currTask != nullptr)
        {
            Task& task = *currTask.get();

            Abacus::ExecResult taskResult = Abacus::Execute(task.Statement.toStdString(), state, isCancelling);
            task.IsSuccessfull = taskResult.Brief == Abacus::ResultBrief::SUCCEEDED;
            task.Preview = task.IsSuccessfull ? "Ok. " : "Error: ";

            for (const auto& err : taskResult.Errors)
            {
              const std::string& msg = err.Message;
              task.Preview.append(msg.c_str());

              const std::vector<Abacus::Position>& positions = err.Positions;
              if (!positions.empty())
              {
                task.Preview.append(QString(", where: "));
                for (const auto& pos : positions)
                {
                  // Position is converted from indecies to ordinal.
                  task.Preview.append(QString("%1, ").arg(pos.CharIdx + 1U));
                }
              }
            }

            task.State.swap(taskResult.Variables);

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
                    const QString varName(v.first.c_str());
                    const QString varValue(v.second.ToString().c_str());

                    task.Preview.append(QString("%1: %2, ")
                                       .arg(varName)
                                       .arg(varValue));
                }
            }

            {
                std::lock_guard<std::mutex> lock(m_mtx);

                if (!m_cancellingCurrentTask)
                {
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
}

void ExecQueue::CancelTasksImpl(unsigned fromTaskIdx)
{
    // This function must be called under lock on m_mtx.

    while (!m_waitingTasks.empty() && m_waitingTasks.back()->Idx >= fromTaskIdx)
    {
        emit TaskCancelled(m_waitingTasks.back()->Idx);
        m_waitingTasks.pop_back();
    }

    if (m_doneTasks.size() >= fromTaskIdx)
    {
        qInfo("Cancelled current task %u", fromTaskIdx);
        m_cancellingCurrentTask = true;
    }
    else
    {
        qInfo("Current task is kept. Done size: %u, fromIdx: %u",
              static_cast<unsigned>(m_doneTasks.size()), fromTaskIdx);
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
