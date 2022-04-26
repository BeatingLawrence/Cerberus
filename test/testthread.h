#ifndef TESTTHREAD_H
#define TESTTHREAD_H

#include <cerberus/thread/thread.h>

class TestThread : public cerberus::thread::Thread
{
    private:
        virtual int tick() override;
        virtual void warmUp() override;
        virtual void coolDown() override;

    public:
        TestThread();

};

#endif // TESTTHREAD_H
