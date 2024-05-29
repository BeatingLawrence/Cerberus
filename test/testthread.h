#ifndef TESTTHREAD_H
#define TESTTHREAD_H

#include <thread/thread.h>

class TestThread : public cerberus::Thread
{
   private:
    virtual int tick() override;
    virtual void warmUp() override;
    virtual void coolDown() override;

   public:
    TestThread(const char* name);
};

#endif  // TESTTHREAD_H
