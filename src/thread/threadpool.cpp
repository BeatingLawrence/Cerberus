#include "threadpool.h"

#include "../cerberus.h"
#include "../mutex/mutexlocker.h"
#include "../time/timer.h"
#include "player.h"

using namespace cerberus;

//=============================================================================
void ThreadPool::taskEndCb(void *ctx, void *data, OpRes res) { ((ThreadPool *)ctx)->_taskEndCb(data, res); }
//=============================================================================
void ThreadPool::_taskEndCb(void *data, OpRes res)
{
    // run when a player ends its task

    MutexLocker ml(m_poolMutex);

    if (data)
        ((Timer *)data)->start();
    else
        m_manager->start();
}
//=============================================================================
void ThreadPool::timerEndCb(void *ctx) { ((ThreadPool *)ctx)->_timerEndCb(); }
//=============================================================================
void ThreadPool::_timerEndCb()
{
    // run when a timer ends
    m_manager->start();
}
//=============================================================================
OpRes ThreadPool::managerCb(void *ctx, void *data)
{
    (void)data;
    return ((ThreadPool *)ctx)->_managerCb();
}
//=============================================================================
OpRes ThreadPool::_managerCb()
{
    // run when the manager wakes up (there is at least one timer expired)

    MutexLocker ml(m_poolMutex);

    bool done = false;
    while (!done)
    {
        done = true;
        for (auto it = m_pool.begin(); it != m_pool.end(); it++)
        {
            if (!it->backup) continue;

            if (!it->player->end()) continue;

            if (it->timer)
            {
                if (it->timer->isRunning()) continue;
            }

            // timeout
            done = false;
            it->player->join(true);
            delete it->player;
            delete it->timer;
            m_pool.erase(it);
            logDebug("temporary thread removed");
            break;
        }
    }

    return OR_OK;
}
//=============================================================================
ThreadPool::PoolEl ThreadPool::newPlayer(bool backup)
{
    PoolEl ret = {};
    ret.player = new Player(true);
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
    : m_manager(new Player(true))
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
    for (auto &el : m_pool)
    {
        el.player->join(true);
        delete el.player;
        if (el.timer) delete el.timer;
    }

    m_pool.clear();
}
//=============================================================================
void ThreadPool::runTask(Task t)
{
    MutexLocker ml(m_poolMutex);

    // search for a free player
    for (auto &el : m_pool)
        if (el.player->end())
        {
            el.player->run(t.cb, t.ctx, t.data);
            if (el.backup && el.timer) el.timer->stop();

            return;
        }

    // no idle player found, creating a backup one
    auto pl = newPlayer(true);
    pl.player->run(t.cb, t.ctx, t.data);
    m_pool.push_back(pl);

    logDebug("temporary thread created");
}
//=============================================================================
