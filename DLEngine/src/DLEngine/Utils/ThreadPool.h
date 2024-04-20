#pragma once
#include <condition_variable>
#include <shared_mutex>
#include <vector>
#include <thread>

#include "DLEngine/Core/Base.h"

class ThreadPool
{
    using Task = std::function<void()>;

    using readWriteLock = std::shared_mutex;
    using readLock = std::shared_lock<readWriteLock>;
    using writeLock = std::unique_lock<readWriteLock>;
public:
    ThreadPool() = default;
    ~ThreadPool() { Stop(); }

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;
    
    void Create(uint32_t threadCount);
    void Stop();

    void Wait();

    template <typename Task, class ...Args>
    void AddTask(Task&& task, Args&&... args)
    {
        {
            readLock _ { m_ObjectLock };
            if (!IsWorkingUnsafe())
                return;
        }

        {
            writeLock _ { m_ObjectLock };

            if (m_Task)
            {
                m_TaskWaiter.notify_one();
                m_TaskPusher.wait(_, [this] { return m_IsTerminated || !m_Task; });
            }

            if (!IsWorkingUnsafe())
                return;

            m_Task = CreateScope<ThreadPool::Task>(std::bind(std::forward<Task>(task), std::forward<Args>(args)...));
            m_TaskWaiter.notify_one();
        }
    }

    uint32_t GetWorkersCountUnsafe() const { return static_cast<uint32_t>(m_Workers.size()); }

    bool IsWorking() const { readLock _ { m_ObjectLock }; return IsWorkingUnsafe(); }
    bool IsWorkingUnsafe() const { return m_IsInitialized && !m_IsTerminated; }

private:
    void Routine();

private:
    mutable readWriteLock m_ObjectLock;
    mutable std::condition_variable_any m_TaskWaiter;
    mutable std::condition_variable_any m_TaskPusher;

    Scope<Task> m_Task;

    std::vector<std::thread> m_Workers;

    bool m_IsInitialized { false };
    bool m_IsTerminated { false };

    std::atomic<uint32_t> m_BusyWorkers { 0 };
};
