#include <cerberus/cerberus.h>
#include <cerberus/thread/thread.h>
#include <cerberus/time/timer.h>
#include <gtest/gtest.h>

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
