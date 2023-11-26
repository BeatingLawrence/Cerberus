#include "testthread.h"

#include <cerberus/cerberus.h>

int TestThread::tick()
{
    thrLogInfo("TICK");
    return 10;
}

void TestThread::warmUp() { thrLogInfo("Warm-up"); }

void TestThread::coolDown() { thrLogInfo("Cool-down"); }

TestThread::TestThread(const char* name)
    : cerberus::thread::Thread(Thread::ThreadPeriodicity::TP_Periodic, 500, name)
{
    // noop
}
