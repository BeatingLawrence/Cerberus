#include "threadpool.h"

#include "../cerberus.h"
#include "../time/timer.h"
#include "mutexlocker.h"
#include "player.h"

using namespace cerberus;

//=============================================================================
void ThreadPool::taskEndCb(void* ctx, void* data, OpRes res) { ((ThreadPool*)ctx)->_taskEndCb(data, res); }
//=============================================================================
void ThreadPool::_taskEndCb(void* data, OpRes res)
{
    (void)res;

    bool wakeManager = false;
    {
        MutexLocker ml(m_poolMutex);

        if (data)
            ((Timer*)data)->start();
        else
            wakeManager = true;

        if (!m_queue.empty()) wakeManager = true;
    }

    if (wakeManager) m_manager->start();
}

//=============================================================================
void ThreadPool::timerEndCb(void* ctx) { ((ThreadPool*)ctx)->_timerEndCb(); }
//=============================================================================
void ThreadPool::_timerEndCb() { m_manager->start(); }
//=============================================================================
OpRes ThreadPool::managerCb(void* ctx, void* data)
{
    (void)data;
    return ((ThreadPool*)ctx)->_managerCb();
}
//=============================================================================
OpRes ThreadPool::_managerCb()
{
    MutexLocker ml(m_poolMutex);

    // cleanup timed-out backup threads
    bool done = false;
    while (!done)
    {
        done = true;
        for (auto it = m_pool.begin(); it != m_pool.end(); it++)
        {
            if (!it->backup) continue;
            if (!it->player->end()) continue;

            if (it->timer)
                if (it->timer->isRunning()) continue;

            done = false;
            it->player->join(true);
            delete it->player;
            delete it->timer;
            m_pool.erase(it);
            logDebug("temporary thread removed");
            break;
        }
    }

    // dispatch queued tasks on idle players
    for (;;)
    {
        if (m_queue.empty()) break;

        bool dispatched = false;

        for (auto& el : m_pool)
        {
            if (m_queue.empty()) break;
            if (!el.player->end()) continue;

            Task t = m_queue.front();
            m_queue.pop_front();

            el.player->run(t.cb, t.ctx, t.data);
            if (el.backup && el.timer) el.timer->stop();

            dispatched = true;
        }

        if (!dispatched) break;
    }

    return OR_OK;
}
//=============================================================================
ThreadPool::PoolEl ThreadPool::newPlayer(bool backup)
{
    PoolEl ret = {};
    ret.player = new Player(true);
    ret.timer  = nullptr;
    ret.backup = backup;

    if (backup && !m_maxInactiveTime.isNull())
    {
        ret.timer = new Timer(m_maxInactiveTime);
        ret.timer->provideTimeoutCallback(ThreadPool::timerEndCb, this);
    }

    ret.player->setTaskEndCB(ThreadPool::taskEndCb, this, ret.timer);

    return ret;
}
//=============================================================================
ThreadPool::ThreadPool()
    : m_manager(new Player(true)),
      m_allowBackup(true),
      m_maxQueue(0)
{
    m_manager->assign(ThreadPool::managerCb, this);
}
//=============================================================================
ThreadPool::~ThreadPool()
{
    m_manager->join(true);
    delete m_manager;
}
//=============================================================================
void ThreadPool::build(SIZE size, TimeFrame maxInactiveTime)
{
    if (size == 0 || !m_pool.empty()) return;

    m_maxInactiveTime = maxInactiveTime;

    for (SIZE i = 0; i < size; i++) m_pool.push_back(newPlayer(false));
}
//=============================================================================
void ThreadPool::clear()
{
    for (auto& el : m_pool)
    {
        el.player->join(true);
        delete el.player;
        if (el.timer) delete el.timer;
    }

    m_pool.clear();

    {
        MutexLocker ml(m_poolMutex);
        m_queue.clear();
    }
}
//=============================================================================
void ThreadPool::allowBackupThreads(bool allow)
{
    MutexLocker ml(m_poolMutex);
    m_allowBackup = allow;
}
//=============================================================================
void ThreadPool::setMaxQueue(SIZE maxQueue)
{
    MutexLocker ml(m_poolMutex);
    m_maxQueue = maxQueue;

    if (m_maxQueue && m_queue.size() > m_maxQueue)
        while (m_queue.size() > m_maxQueue) m_queue.pop_back();
}
//=============================================================================
OpRes ThreadPool::runTask(Task t)
{
    bool wakeManager = false;

    {
        MutexLocker ml(m_poolMutex);

        // search for a free player
        for (auto& el : m_pool)
            if (el.player->end())
            {
                el.player->run(t.cb, t.ctx, t.data);
                if (el.backup && el.timer) el.timer->stop();
                return OR_OK;
            }

        // no idle player
        if (m_allowBackup)
        {
            auto pl = newPlayer(true);
            pl.player->run(t.cb, t.ctx, t.data);
            m_pool.push_back(pl);
            logDebug("temporary thread created");
            return OR_OK;
        }

        // if no backup threads, we must have at least one fixed worker or the queue is useless
        if (m_pool.empty()) return OR_Failure;

        // enqueue
        if (m_maxQueue && m_queue.size() >= m_maxQueue)
        {
            logDebug("threadpool queue full, dropping task");
            return OR_Failure;
        }

        m_queue.push_back(t);
        wakeManager = true;
    }

    if (wakeManager) m_manager->start();

    return OR_OK;
}
//=============================================================================
OpRes ThreadPool::runTask(cerberus::OpRes (*cb)(void*, void*), void* ctx, void* data)
{
    Task t = {};
    t.cb   = cb;
    t.ctx  = ctx;
    t.data = data;
    return runTask(t);
}
//=============================================================================
size_t ThreadPool::size()
{
    MutexLocker ml(m_poolMutex);
    return m_pool.size();
}
//=============================================================================
