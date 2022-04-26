#include <gtest/gtest.h>
#include "./testthread.h"
#include <thread>
#include <chrono>

TEST(threadTest, thread_creation)
{
    TestThread thread;
    thread.start();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    EXPECT_EQ(thread.join(), 10);
}
