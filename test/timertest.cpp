#include <cerberus/cerberus.h>
#include <cerberus/thread/thread.h>
#include <cerberus/time/timer.h>
#include <gtest/gtest.h>

TEST(timerTest, timer)
{
    auto timer = cerberus::time::Timer(cerberus::time::TimeFrame(500, cerberus::time::TimeFrame::U_MilliSecond));
    timer.start();
    logDebug("timer started");

    while (timer.isRunning())
    {
        cerberus::thread::Thread::sleep(cerberus::time::TimeFrame(1, cerberus::time::TimeFrame::U_MilliSecond));
    }
    logDebug("timer ended");

    //
}
