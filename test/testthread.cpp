#include "testthread.h"
#include <cerberus/cerberus.h>

int TestThread::tick()
{
    logInfo("TICK!");
    return 10;
}

void TestThread::warmUp()
{
    logInfo("Warm-up");
}

void TestThread::coolDown()
{
    logInfo("Cool-down");
}

TestThread::TestThread(const char* name) : cerberus::thread::Thread(name, Thread::ThreadPeriodicity::TP_Periodic, 500)
{
    // noop
}
