#ifndef CERBERUS_ACTOR_H
#define CERBERUS_ACTOR_H

#include "thread.h"

namespace cerberus
{
    class Actor final : public Thread
    {
       private:
        actorCallback m_cb;
        taskEndCallback m_endCb;
        void *m_ctx, *m_endCbCtx;
        void sendTaskEndMsg(HASH32 recipient, OpRes res);

        virtual int tick() override;

       public:
        Actor(bool manualTrigger = false, const std::string& name = "");

        Actor(const Actor& other) = delete;

        virtual ~Actor();

        // check if the actor has done
        bool end();

        // check if the Actor is busy
        bool running();

        // start task execution
        OpRes start();

        // assign a new task that will replace the current one
        OpRes assign(actorCallback cb, void* ctx = nullptr);

        // assign a new task and run it
        OpRes run(actorCallback cb, void* ctx = nullptr);

        // set a new task-end callback. This function will be called
        // when the task ends and will contain the task result
        OpRes setTaskEndCB(taskEndCallback cb, void* ctx = nullptr);
    };

}  // namespace cerberus

#endif  // CERBERUS_ACTOR_H
