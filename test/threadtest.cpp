#include <gtest/gtest.h>
#include "./testthread.h"
#include <cerberus/cerberus.h>
#include <thread>
#include <chrono>

TEST(threadTest, derived_thread_creation)
{
    TestThread thread1("test-Thread1");
    TestThread thread2("test-Thread2");
    thread1.start();
    thread2.start();
    std::this_thread::sleep_for(std::chrono::seconds(5));
    thread2.join();
    EXPECT_EQ(thread1.join(), 10);
}

static int testCallback(cerberus::message::cerberus_message msg)
{
    logInfo("Tick from callback");
    return 20;
}

TEST(threadTest, thread_callback)
{
    cerberus::thread::Thread thread(cerberus::thread::Thread::TP_Periodic, 100, "test-Thread3");
    thread.provideTickCallback(&testCallback);
    thread.start();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    EXPECT_EQ(thread.join(), 20);
}
