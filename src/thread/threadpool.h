#ifndef CERBERUS_THREADPOOL_H
#define CERBERUS_THREADPOOL_H

#include <deque>
#include <list>

#include "../types.h"
#include "mutex.h"

namespace crb
{
    class Player;
    class Timer;

    class ThreadPool
    {
        struct PoolEl
        {
            Player* player;
            Timer* timer;
            bool backup;
        };

        Player* m_manager;

        TimeFrame m_maxInactiveTime;

        std::list<PoolEl> m_pool;
        std::deque<Task> m_queue;

        mutable Mutex m_poolMutex;

        bool m_allowBackup;
        SIZE m_maxQueue;  // 0 = unlimited

        static void taskEndCb(void* ctx, void* data, OpRes res);
        void _taskEndCb(void* data, OpRes res);

        static void timerEndCb(void* ctx);
        void _timerEndCb();

        static OpRes managerCb(void* ctx, void* data);
        OpRes _managerCb();

        PoolEl newPlayer(bool backup);

       public:
        ThreadPool();
        ~ThreadPool();

        // Construct the pool with a fixed number of workers.
        void build(SIZE size, TimeFrame maxInactiveTime = TimeFrame());

        // Stop all workers and clear any queued tasks.
        void clear();

        // Enable/disable temporary backup worker threads.
        void allowBackupThreads(bool allow);

        // Set max queued tasks (0 = unlimited). If shrinking, drops newest tasks.
        void setMaxQueue(SIZE maxQueue);

        // Returns OR_OK if accepted, OR_Failure if rejected.
        OpRes runTask(Task t);

        // Convenience overload.
        OpRes runTask(crb::OpRes (*cb)(void*, void*), void* ctx, void* data);

        // Returns current total number of workers (fixed + backup).
        size_t size() const;

        // Returns number of queued tasks waiting for an idle worker.
        SIZE queuedCount() const;

        // Returns number of workers currently running a task.
        SIZE busyCount() const;

        // Returns number of idle workers ready to accept a task.
        SIZE idleCount() const;

        // Returns number of fixed (non-backup) workers.
        SIZE fixedCount() const;

        // Returns number of backup workers currently alive.
        SIZE backupCount() const;

        // Returns max queued tasks (0 = unlimited).
        SIZE maxQueue() const;

        // Returns whether backup threads are allowed.
        bool backupAllowed() const;
    };
}  // namespace crb

#endif  // CERBERUS_THREADPOOL_H
