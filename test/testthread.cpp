#include "testthread.h"
#include <iostream>

int TestThread::tick()
{
    std::cout << "TICK" << std::endl;
    return 10;
}

void TestThread::warmUp()
{
    std::cout << "warm-up" << std::endl;
}

void TestThread::coolDown()
{
    std::cout << "cool-down" << std::endl;
}

TestThread::TestThread() : cerberus::thread::Thread(Thread::ThreadPeriodicity::TP_Periodic, 500)
{
    // noop
}
