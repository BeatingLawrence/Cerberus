#include <cerberus.h>
#include <gtest/gtest.h>
#include <thread/player.h>

using namespace cerberus;

class myclass
{
    int x;

    void increment() { x++; }

    void print() { logInfo("value: %i", x); }

   public:
    myclass()
        : x(0)
    {
    }

    static cerberus::OpRes inc(void* ctx, void* data)
    {
        (void)data;
        ((myclass*)ctx)->print();
        Thread::sleep(300);
        ((myclass*)ctx)->increment();
        ((myclass*)ctx)->print();
        return cerberus::OR_OK;
    }
};

static int playertest_standalone(msg_ptr msg, Thread* thr)
{
    logInfo("thread tick, received %u", msg->id());

    thr->terminate();

    if (msg->id() == CERBERUS_MESSAGE_TASKEND_ID) return 0;

    return 1;
}

TEST(playerTest, standalone)
{
    Thread thr("standaloneTestThread");
    thr.checkIn();
    thr.provideTickCallback(playertest_standalone);
    thr.start();
    //
    Player p(false, "standalonethread");
    p.checkIn();
    p.start();

    myclass c;

    auto m = Cerberus::constructMessage(CERBERUS_MESSAGE_TASK_ID);
    m->getSlot("task")->to<TaskSlot>()->value({myclass::inc, &c});
    m->getSlot("client")->to<UInt64Slot>()->value(thr.id());
    m->setRecipient(p.id());
    Cerberus::send(m);
    //
    EXPECT_EQ(thr.join().value, 0);
    p.join(true);
}

TEST(playerTest, threadPool)  // to test dynamic thread allocation, set the thread pool to 1
{
    for (int i = 0; i < 5; i++)
    {
        myclass c1, c2;

        {
            auto m = Cerberus::constructMessage(CERBERUS_MESSAGE_TASK_ID);
            m->getSlot("task")->to<TaskSlot>()->value({myclass::inc, &c1});
            Cerberus::send(m);
        }

        {
            auto m = Cerberus::constructMessage(CERBERUS_MESSAGE_TASK_ID);
            m->getSlot("task")->to<TaskSlot>()->value({myclass::inc, &c2});
            Cerberus::send(m);
        }

        Thread::sleep(1000);
    }

    Thread::sleep(3000);  // give time to the tester to see the "temporary thread removed" log
}
