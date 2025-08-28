#include <cerberus.h>
#include <core/cerberusregister.h>
#include <gtest/gtest.h>
#include <thread/thread.h>

using namespace cerberus;

class TestThread : public cerberus::Thread
{
   private:
    virtual int tick() override
    {
        tlogInfo("TICK");
        return 10;
    }

    virtual void warmUp() override { tlogInfo("Warm-up"); }
    virtual void coolDown() override { tlogInfo("Cool-down"); }

   public:
    TestThread(const char* name)
        : cerberus::Thread(cerberus::ThreadPeriodicity::TP_Periodic, 500, name)
    {
    }
};

TEST(threadTest, simple_thread_creation_destruction)
{
    Thread thread("Simple-Thread");
    thread.checkIn();
    thread.join(true);
}

TEST(threadTest, derived_thread_creation)
{
    TestThread thread1("test-Thread1");
    TestThread thread2("test-Thread2");
    thread1.checkIn();
    thread2.checkIn();
    thread1.start();
    thread2.start();
    Thread::sleep(2000);
    thread2.join(true);
    logDebug("t1 joined");
    EXPECT_EQ(thread1.join(true).expect().value, 10);
}

static int testCallback(msg_ptr msg, Thread* thread)
{
    logInfo("Tick from callback");
    return 20;
}

static void testWarmUpCallback(Thread* thread) { logInfo("Warm-up Callback"); }

static void testCoolDownCallback(Thread* thread) { logInfo("Cool-down Callback"); }

TEST(threadTest, thread_callback)
{
    Thread thread3(cerberus::TP_Periodic, 100, "test-Thread3");
    thread3.checkIn();
    thread3.provideTickCallback(&testCallback);
    thread3.provideWarmUpCallback(&testWarmUpCallback);
    thread3.provideCoolDownCallback(&testCoolDownCallback);
    thread3.start();
    // debug("2");
    Thread::sleep(1000);
    EXPECT_EQ(thread3.join(true).expect().value, 20);
}

static int pingTestCallback(msg_ptr msg, Thread* thread)
{
    static char a = 0;
    static char b = 10;
    static char c = 20;

    if (msg)
    {
        logInfo(CerberusUtils::strPrint("PONG! %u %u %u",
                                        msg->getSlotAt(0)->to<cerberus::ByteSlot>()->value(),
                                        msg->getSlotAt(1)->to<cerberus::ByteSlot>()->value(),
                                        msg->getSlotAt(2)->to<cerberus::ByteSlot>()->value()));
    }
    else  // tick
    {
        if (a == 10)
        {
            auto message = Cerberus::constructMessage(CERBERUS_MESSAGE_TERM_ID);
            Cerberus::send(message, "pongThread");
            thread->terminate();
            return 0;
        }

        logInfo("PING!");
        // Create message using factory
        auto message = Cerberus::constructMessage("PingPongMessage");
        message->getSlotAt(0)->to<cerberus::ByteSlot>()->value(a++);
        message->getSlotAt(1)->to<cerberus::ByteSlot>()->value(b++);
        message->getSlotAt(2)->to<cerberus::ByteSlot>()->value(c++);
        // Send the message
        Cerberus::send(message, "pongThread");
    }

    return 0;
}

static int pongTestCallback(msg_ptr msg, Thread* thread)
{
    if (msg->id() == CERBERUS_MESSAGE_TERM_ID)
    {
        thread->terminate();
        return 0;
    }

    logInfo("Sending Back..");
    // change destination and send
    cerberus::Cerberus::send(msg, "pingThread");
    return 0;
}

TEST(threadTest, thread_ping_pong)
{
    // register a generic message to test Thread message exchange
    MessageTemplate tmplt("PingPongMessage");
    tmplt.addSlotType<ByteSlot>().addSlotType<ByteSlot>().addSlotType<ByteSlot>();
    Cerberus::registerTemplate(tmplt);

    // start the test
    Thread ping(cerberus::TP_PeriodicMessage, 100, "pingThread");
    Thread pong("pongThread");  // Non-Periodic (receiver)

    ping.checkIn();
    pong.checkIn();

    ping.provideTickCallback(&pingTestCallback);
    pong.provideTickCallback(&pongTestCallback);

    pong.start();
    ping.start();

    ping.join();
    pong.join();
}
