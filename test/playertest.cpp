#include <cerberus/cerberus.h>
#include <cerberus/thread/player.h>
#include <gtest/gtest.h>

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
        ((myclass*)ctx)->increment();
        ((myclass*)ctx)->print();
        return cerberus::OR_OK;
    }
};

static int playertest_standalone(cerberus_message msg, Thread* thr)
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

TEST(playerTest, threadPool)
{
    myclass c;

    auto m = Cerberus::constructMessage(CERBERUS_MESSAGE_TASK_ID);
    m->getSlot("task")->to<TaskSlot>()->value({myclass::inc, &c});
    Cerberus::send(m);

    Thread::sleep(1000);
}
