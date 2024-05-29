#include "testthread.h"

#include <cerberus.h>

int TestThread::tick()
{
    tlogInfo("TICK");
    return 10;
}

void TestThread::warmUp() { tlogInfo("Warm-up"); }

void TestThread::coolDown() { tlogInfo("Cool-down"); }

TestThread::TestThread(const char* name)
    : cerberus::Thread(cerberus::ThreadPeriodicity::TP_Periodic, 500, name)
{
    // noop
}
