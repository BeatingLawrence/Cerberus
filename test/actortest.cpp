#include <cerberus/cerberus.h>
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

    static cerberus::OpRes inc(void* ctx)
    {
        ((myclass*)ctx)->print();
        ((myclass*)ctx)->increment();
        ((myclass*)ctx)->print();
        return cerberus::OR_OK;
    }
};

static int actortest_standalone(cerberus_message msg, Thread* thr)
{
    logInfo("thread tick, arrived %u", msg->id());

    thr->terminate();

    if (msg->id() == CERBERUS_MESSAGE_TASKEND_ID) return 0;

    return 1;
}

TEST(actorTest, standalone)
{
    cerberus::Thread thr("standaloneTestThread");
    thr.checkIn();
    thr.provideTickCallback(actortest_standalone);
    thr.start();
    //
    auto code = Cerberus::createThread("standalonethread");
    myclass c;

    auto m = Cerberus::messageConstruct(CERBERUS_MESSAGE_TASK_ID);
    m->getSlot("task")->to<TaskSlot>()->value({myclass::inc, &c});
    m->getSlot("client")->to<UInt64Slot>()->value(thr.id());
    m->setRecipient(code);
    Cerberus::send(m);
    //
    EXPECT_EQ(thr.join().value, 0);
}

TEST(actorTest, threadPool)
{
    myclass c;

    auto m = Cerberus::messageConstruct(CERBERUS_MESSAGE_TASK_ID);
    m->getSlot("task")->to<TaskSlot>()->value({myclass::inc, &c});
    Cerberus::send(m);

    Thread::sleep(1000);
}
