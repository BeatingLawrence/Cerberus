#include <cerberus.h>
#include <gtest/gtest.h>
#include <thread/player.h>

using namespace crb;

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

    static crb::OpRes inc(void* ctx, void* data)
    {
        (void)data;
        ((myclass*)ctx)->print();
        Thread::sleep(300);
        ((myclass*)ctx)->increment();
        ((myclass*)ctx)->print();
        return crb::OR_OK;
    }
};

static int playertest_standalone(msg_ptr msg, Thread* thr)
{
    logInfo("thread tick, received %u", msg->id());

    thr->terminate();

    if (msg->id() == CRB_MESSAGE_TASKEND_ID) return 0;

    return 1;
}

TEST(playerTest, standalone)
{
    static int counter = 0;
    std::string tname  = CerberusUtils::strPrint("standaloneTestThread_test_%d", counter++);
    Thread thr;
    thr.checkIn(tname);
    thr.provideTickCallback(playertest_standalone);
    thr.start();
    //
    Player p(false);
    p.checkIn("standalonethread");
    p.start();

    myclass c;

    auto m = Cerberus::constructMessage(CRB_MESSAGE_TASK_ID);
    m->getSlot("task")->to<TaskSlot>()->value({myclass::inc, &c});
    m->getSlot("client")->to<UInt64Slot>()->value(thr.id());
    m->setRecipient(p.id());
    Cerberus::send(m);
    //
    Thread::sleep(200);  // give time for message exchange
    thr.terminate();
    EXPECT_TRUE(thr.join(true).ok());
    p.join(true);
}

TEST(playerTest, threadPool)  // to test dynamic thread allocation, set the thread pool to 1
{
    for (int i = 0; i < 5; i++)
    {
        myclass c1, c2;

        {
            auto m = Cerberus::constructMessage(CRB_MESSAGE_TASK_ID);
            m->getSlot("task")->to<TaskSlot>()->value({myclass::inc, &c1});
            Cerberus::send(m);
        }

        {
            auto m = Cerberus::constructMessage(CRB_MESSAGE_TASK_ID);
            m->getSlot("task")->to<TaskSlot>()->value({myclass::inc, &c2});
            Cerberus::send(m);
        }

        Thread::sleep(500);
    }
}
