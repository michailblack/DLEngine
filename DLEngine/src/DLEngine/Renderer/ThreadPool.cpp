#include "dlpch.h"
#include "ThreadPool.h"

void ThreadPool::Create(uint32_t threadCount)
{
    writeLock _ { m_ObjectLock };

    if (m_IsInitialized || m_IsTerminated)
        return;

    for (uint32_t i = 0; i < threadCount; ++i)
        m_Workers.emplace_back(&ThreadPool::Routine, this);

    m_IsInitialized = m_Workers.size() == threadCount;
    m_IsTerminated = !m_IsInitialized;
}

void ThreadPool::Stop()
{
    {
        writeLock _ { m_ObjectLock };

        if (!IsWorkingUnsafe())
            return;

        m_IsInitialized = false;
        m_IsTerminated = true;
    }

    m_TaskWaiter.notify_all();
    m_TaskPusher.notify_all();

    for (auto& worker : m_Workers)
        worker.join();

    m_IsTerminated = false;

    m_Workers.clear();
    m_Task.reset();

    m_BusyWorkers.store(0);
}

void ThreadPool::Wait()
{
    uint32_t expectedBusyWorkers { 0 };
    do
    {
        expectedBusyWorkers = 0;
    } while (!m_BusyWorkers.compare_exchange_weak(expectedBusyWorkers, expectedBusyWorkers));
}

void ThreadPool::Routine()
{
    while (true)
    {
        Task task;
        bool taskAcquired { false };

        {
            writeLock _ { m_ObjectLock };

            m_TaskWaiter.wait(_, [this, &task, &taskAcquired] {
                taskAcquired = m_Task != nullptr;
                if (taskAcquired)
                    task = std::move(*m_Task);
                return m_IsTerminated || taskAcquired;
            });

            m_Task.reset();
            m_TaskPusher.notify_one();
        }

        if (!taskAcquired)
            return;

        m_BusyWorkers.fetch_add(1);

        task();

        m_BusyWorkers.fetch_add(-1);
    }
}
