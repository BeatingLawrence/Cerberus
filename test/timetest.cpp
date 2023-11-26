#include <cerberus/cerberus.h>
#include <cerberus/time/datetime.h>
#include <cerberus/time/timeframe.h>
#include <gtest/gtest.h>

using namespace cerberus::time;

TEST(timeFrameTest, creation)
{
    TimeFrame hundredMilliseconds(100);
    TimeFrame tenSeconds(10, TimeFrame::U_Second);
    TimeFrame twoMonths(2, TimeFrame::U_Month);
    TimeFrame anYear(1, TimeFrame::U_Year);

    EXPECT_EQ(hundredMilliseconds.toMilliseconds(), 100);
    EXPECT_EQ(hundredMilliseconds.milliseconds(), 100);

    EXPECT_EQ(tenSeconds.toMilliseconds(), 10000);
    EXPECT_EQ(tenSeconds.milliseconds(), 0);
    EXPECT_EQ(tenSeconds.toSeconds(), 10);

    EXPECT_EQ(twoMonths.toMonths(), 2);
    EXPECT_EQ(twoMonths.toDays(), 60);

    EXPECT_EQ(anYear.toDays(), 365);
    EXPECT_EQ(anYear.days(), 5);
    EXPECT_EQ(anYear.toSeconds(), 31536000);
}

TEST(timeFrameTest, sum)
{
    TimeFrame hundredMilliseconds(100);
    TimeFrame tenSeconds(10, TimeFrame::U_Second);

    auto sum = hundredMilliseconds + tenSeconds;

    EXPECT_EQ(sum.toMilliseconds(), 10100);
    //

    EXPECT_EQ(tenSeconds.toMilliseconds(), 10000);
    EXPECT_EQ(hundredMilliseconds.toMilliseconds(), 100);

    auto diff = tenSeconds - hundredMilliseconds;

    EXPECT_EQ(diff.toMilliseconds(), 9900);

    hundredMilliseconds.setNegative();

    diff = tenSeconds + hundredMilliseconds;

    EXPECT_EQ(diff.toMilliseconds(), 9900);
}

TEST(dateTimeTest, creation)
{
    auto dt = DateTime::current();

    debug("%s", dt.toString().c_str());

    debug(dt.usingDst() ? "USING DST" : "NOT USING DST");

    dt.addMonths(6);

    debug("%s", dt.toString().c_str());

    debug(dt.usingDst() ? "USING DST" : "NOT USING DST");

    debug("timestamp %s", dt.toTimeStampString().c_str());
}
