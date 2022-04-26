#include "testthread.h"
#include <cerberus/cerberus.h>

int TestThread::tick()
{
    cerberus::Cerberus::stdoutPrint("TICK");
    return 10;
}

void TestThread::warmUp()
{
    cerberus::Cerberus::stdoutPrint("warm-up");
}

void TestThread::coolDown()
{
    cerberus::Cerberus::stdoutPrint("cool-down");
}

TestThread::TestThread() : cerberus::thread::Thread(Thread::ThreadPeriodicity::TP_Periodic, 500)
{
    // noop
}
