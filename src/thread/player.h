#ifndef CERBERUS_PLAYER_H
#define CERBERUS_PLAYER_H

#include "thread.h"

namespace crb
{
    class Player final : public Thread
    {
       private:
        playerCallback m_cb;
        taskEndCallback m_endCb;
        void *m_ctx, *m_endCbCtx, *m_data, *m_endCbData;
        void sendTaskEndMsg(HASH32 recipient, OpRes res);

        virtual int tick() override;

       public:
        CERBERUS_EXPORT Player(bool manualTrigger = false, LSIZE stackSize = 0);

        Player(const Player& other) = delete;

        CERBERUS_EXPORT virtual ~Player();

        // check if the player has done
        CERBERUS_EXPORT bool end();

        // check if the player is busy
        CERBERUS_EXPORT bool running();

        // start task execution
        CERBERUS_EXPORT OpRes start();

        // assign a new task that will replace the current one
        CERBERUS_EXPORT OpRes assign(playerCallback cb, void* ctx = nullptr, void* data = nullptr);

        // assign a new task and run it
        CERBERUS_EXPORT OpRes run(playerCallback cb, void* ctx = nullptr, void* data = nullptr);

        // set a new task-end callback. This function will be called
        // when the task ends and will contain the task result
        CERBERUS_EXPORT OpRes setTaskEndCB(taskEndCallback cb, void* ctx = nullptr, void* data = nullptr);
    };

}  // namespace crb

#endif  // CERBERUS_PLAYER_H
