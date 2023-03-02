#include <gtest/gtest.h>
#include "./testthread.h"
#include <cerberus/cerberus.h>
#include <cerberus/message/slot/charslot.h>
#include <thread>
#include <chrono>
#include <cerberus/core/cerberusfactory.h>

using namespace cerberus;

TEST(threadTest, simple_thread_creation_destruction)
{
    cerberus::thread::Thread* thread = new cerberus::thread::Thread("Simple-Thread");
    delete thread;
}

TEST(threadTest, derived_thread_creation)
{
    TestThread thread1("test-Thread1");
    TestThread thread2("test-Thread2");
    thread1.start();
    thread2.start();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    thread2.join();
    EXPECT_EQ(thread1.join(), 10);
}

static int testCallback(cerberus::message::cerberus_message msg, cerberus::thread::Thread* thread)
{
    logInfo("Tick from callback");
    return 20;
}

static void testWarmUpCallback()
{
    logInfo("Warm-up Callback");
}

static void testCoolDownCallback()
{
    logInfo("Cool-down Callback");
}

TEST(threadTest, thread_callback)
{
    cerberus::thread::Thread thread("test-Thread3", cerberus::thread::Thread::TP_Periodic, 100);
    thread.provideTickCallback(&testCallback);
    thread.provideWarmUpCallback(&testWarmUpCallback);
    thread.provideCoolDownCallback(&testCoolDownCallback);
    thread.start();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    EXPECT_EQ(thread.join(), 20);
}

static int pingTestCallback(cerberus::message::cerberus_message msg, cerberus::thread::Thread* thread)
{
    static char a = 0;
    static char b = 10;
    static char c = 20;

    if(msg->isValid())
    {
        logInfo(core::CerberusUtils::strPrint("PONG! %u %u %u", msg->getSlotAt(0)->to<cerberus::message::slot::CharSlot>()->value(),
                                              msg->getSlotAt(1)->to<cerberus::message::slot::CharSlot>()->value(),
                                              msg->getSlotAt(2)->to<cerberus::message::slot::CharSlot>()->value()));
    }
    else
    {
        if(a == 10)
        {
            auto message = cerberus::core::CerberusFactory::createStandardMessage(cerberus::core::CerberusFactory::SM_ShutdownMessage);
            message->setDestinationId(cerberus::core::CerberusFactory::threadIdByName("pongThread"));
            cerberus::Cerberus::send(message);
            thread->terminate();
            return 0;
        }

        logInfo("PING!");
        //Create message using factory
        auto message = cerberus::core::CerberusFactory::messageConstruct(cerberus::core::CerberusFactory::messageIdByName("PingPongMessage"));
        message->getSlotAt(0)->to<cerberus::message::slot::CharSlot>()->setValue(a++);
        message->getSlotAt(1)->to<cerberus::message::slot::CharSlot>()->setValue(b++);
        message->getSlotAt(2)->to<cerberus::message::slot::CharSlot>()->setValue(c++);
        message->setDestinationId(cerberus::core::CerberusFactory::threadIdByName("pongThread"));
        //Send the message
        cerberus::Cerberus::send(message);
    }

    return 0;
}

static int pongTestCallback(cerberus::message::cerberus_message msg, cerberus::thread::Thread* thread)
{
    if(msg->id() == CERBERUS_MESSAGE_SHUTDOWN_ID)
    {
        thread->terminate();
        return 0;
    }

    logInfo("Sending Back..");
    //Change destination
    msg->setDestinationId(cerberus::core::CerberusFactory::threadIdByName("pingThread"));
    //Send the message
    cerberus::Cerberus::send(msg);
    return 0;
}

TEST(threadTest, thread_ping_pong)
{
    //register a generic message to test Thread messages exchange
    cerberus::message::Message msg;
    msg.addSlot(cerberus::message::slot::CharSlot::create());
    msg.addSlot(cerberus::message::slot::CharSlot::create());
    msg.addSlot(cerberus::message::slot::CharSlot::create());
    cerberus::core::CerberusFactory::registerMessage(msg, "PingPongMessage");
    //start the test
    cerberus::thread::Thread ping("pingThread", cerberus::thread::Thread::TP_PeriodicQueue, 100);
    cerberus::thread::Thread pong("pongThread");    //Non-Periodic (receiver)
    ping.provideTickCallback(&pingTestCallback);
    pong.provideTickCallback(&pongTestCallback);
    ping.start();
    pong.start();
    ping.join(false);
    pong.join(false);
}
