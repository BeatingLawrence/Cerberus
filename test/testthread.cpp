#include "testthread.h"

#include <cerberus/cerberus.h>

int TestThread::tick()
{
    thrlogInfo("TICK");
    return 10;
}

void TestThread::warmUp() { thrlogInfo("Warm-up"); }

void TestThread::coolDown() { thrlogInfo("Cool-down"); }

TestThread::TestThread(const char* name)
    : cerberus::Thread(cerberus::ThreadPeriodicity::TP_Periodic, 500, name)
{
    // noop
}
