#include <cerberus.h>
#include <gtest/gtest.h>
#include <thread/thread.h>
#include <time/timer.h>

using namespace cerberus;

TEST(timerTest, timer)
{
    auto timer = Timer(TimeFrame(500, TimeFrame::U_MilliSecond));
    timer.start();
    logDebug("timer started");

    while (timer.isRunning())
    {
        Thread::sleep(TimeFrame(1, TimeFrame::U_MilliSecond));
    }

    logDebug("timer ended");

    //
}
