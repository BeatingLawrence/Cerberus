#ifndef CERBERUS_PLAYER_H
#define CERBERUS_PLAYER_H

#include "thread.h"

namespace cerberus
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
        Player(bool manualTrigger = false, const std::string& name = "");

        Player(const Player& other) = delete;

        virtual ~Player();

        // check if the actor has done
        bool end();

        // check if the Actor is busy
        bool running();

        // start task execution
        OpRes start();

        // assign a new task that will replace the current one
        OpRes assign(playerCallback cb, void* ctx = nullptr, void* data = nullptr);

        // assign a new task and run it
        OpRes run(playerCallback cb, void* ctx = nullptr, void* data = nullptr);

        // set a new task-end callback. This function will be called
        // when the task ends and will contain the task result
        OpRes setTaskEndCB(taskEndCallback cb, void* ctx = nullptr, void* data = nullptr);
    };

}  // namespace cerberus

#endif  // CERBERUS_PLAYER_H
