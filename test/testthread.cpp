#include "testthread.h"
#include <cerberus/cerberus.h>

int TestThread::tick()
{
    cerberus::Cerberus::log("TICK");
    return 10;
}

void TestThread::warmUp()
{
    cerberus::Cerberus::log("Warm-up");
}

void TestThread::coolDown()
{
    cerberus::Cerberus::log("Cool-down");
}

TestThread::TestThread() : cerberus::thread::Thread(Thread::ThreadPeriodicity::TP_Periodic, 500)
{
    // noop
}
