#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "../types.h"
#include "mutex.h"

namespace cerberus
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
        Mutex m_poolMutex;

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

        // construct the pool
        void build(SIZE size, TimeFrame maxInactiveTime = TimeFrame());

        // clears the pool
        void clear();

        void runTask(Task t);

        // get pool size
        size_t size();
    };
}  // namespace cerberus

#endif  // THREADPOOL_H
