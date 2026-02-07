#include <cerberus.h>
#include <core/cerberusregister.h>
#include <gtest/gtest.h>
#include <thread/thread.h>

using namespace crb;

class TestThread : public crb::Thread
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
    explicit TestThread(const char* name = "TestThread")
        : crb::Thread(crb::ThreadPeriodicity::TP_Periodic, crb::TimeFrame(500))
    {
        checkIn(name);
    }
};

TEST(threadTest, simple_thread_creation_destruction)
{
    Thread thread;
    thread.checkIn("Simple-Thread");
    thread.join(true);
}

TEST(threadTest, derived_thread_creation)
{
    TestThread thread1("test-Thread1");
    TestThread thread2("test-Thread2");
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
    Thread thread3(crb::TP_Periodic, crb::TimeFrame(100));
    thread3.checkIn("test-Thread3");
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
        if (!msg->is("PingPongMessage")) logError("message is not a PingPongMessage");

        logInfo(CerberusUtils::strPrint("PONG! %u %u %u",
                                        msg->getSlotAt(0)->to<crb::ByteSlot>()->value(),
                                        msg->getSlotAt(1)->to<crb::ByteSlot>()->value(),
                                        msg->getSlotAt(2)->to<crb::ByteSlot>()->value()));
    }
    else  // tick
    {
        if (a == 10)
        {
            auto message = Cerberus::constructMessage(CRB_MESSAGE_TERM_ID);
            Cerberus::send(message, "pongThread");
            thread->terminate();
            return 0;
        }

        logInfo("PING!");
        // Create message using factory
        auto message = Cerberus::constructMessage("PingPongMessage");
        message->getSlotAt(0)->to<crb::ByteSlot>()->value(a++);
        message->getSlotAt(1)->to<crb::ByteSlot>()->value(b++);
        message->getSlotAt(2)->to<crb::ByteSlot>()->value(c++);
        // Send the message
        Cerberus::send(message, "pongThread");
    }

    return 0;
}

static int pongTestCallback(msg_ptr msg, Thread* thread)
{
    if (msg->id() == CRB_MESSAGE_TERM_ID)
    {
        thread->terminate();
        return 0;
    }

    logInfo("Sending Back..");
    // change destination and send
    crb::Cerberus::send(msg, "pingThread");
    return 0;
}

TEST(threadTest, thread_ping_pong)
{
    // register a generic message to test Thread message exchange
    auto tmplt = Message::create("PingPongMessage");
    tmplt->addSlotType<ByteSlot>().addSlotType<ByteSlot>().addSlotType<ByteSlot>();
    Cerberus::registerTemplate(tmplt);

    // start the test
    Thread ping(crb::TP_PeriodicMessage, crb::TimeFrame(100));
    ping.checkIn("pingThread");
    Thread pong;
    pong.checkIn("pongThread");  // Non-Periodic (receiver)


    ping.provideTickCallback(&pingTestCallback);
    pong.provideTickCallback(&pongTestCallback);

    pong.start();
    ping.start();

    ping.join();
    pong.join();
}
