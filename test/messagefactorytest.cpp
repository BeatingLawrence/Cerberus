#include <cerberus.h>
#include <gtest/gtest.h>

TEST(messageFactoryTest, registering_message)
{
    auto msg = cerberus::Message::create("Test-Message");
    msg->addSlot(cerberus::ByteSlot::create(10));
    msg->addSlot(cerberus::ByteSlot::create(12));
    msg->addSlot(cerberus::ByteSlot::create(14));

    EXPECT_EQ(msg->count(), 3);

    EXPECT_NO_THROW(cerberus::Cerberus::registerTemplate(msg));
}

TEST(messageFactoryTest, constructing_message)
{
    auto msg = cerberus::Cerberus::constructMessage("Test-Message");
    EXPECT_EQ(msg->count(), 3);

    auto def = cerberus::Cerberus::constructMessage(CERBERUS_MESSAGE_SOCKETDATA_ID);
    EXPECT_EQ(def->count(), 3);
    // def->getSlot("result")->to<cerberus::ResultSlot>();  //->value(cerberus::OR_OK);

    auto& x = def->getSlot("result");
    logInfo("%x", x->id());
}

struct testStruct
{
    uint8_t a, b, c, d;
    const char* ch;
    float val;
};

static const char* charp = "testcharp";

new_slot_from_cstruct(CSTRUCT_SLOT, testStruct);

static int facTestCallback(cerberus::msg_ptr msg, cerberus::Thread* thread)
{
    logInfo("Tick of factory test thread");

    if (msg->is("cstructmessage"))
    {
        logInfo("message is a cstructmessage");

        auto& s = msg->get<CSTRUCT_SLOT>("slot1");
        logInfo("printing slot1:\na:%u\nb:%u\nc:%u\nd:%u\nch:%s\nval:%f", s.a, s.b, s.c, s.d, s.ch, s.val);

        s = msg->get<CSTRUCT_SLOT>("slot2");
        logInfo("printing slot2:\na:%u\nb:%u\nc:%u\nd:%u\nch:%s\nval:%f", s.a, s.b, s.c, s.d, s.ch, s.val);
    }
    else
    {
        logError("message is NOT a cstructmessage");
    }

    thread->terminate();

    return 0;
}

TEST(messageFactoryTest, slotFromCStruct_creation)
{
    cerberus::Thread t;
    t.provideTickCallback(facTestCallback);
    t.start();

    auto s1    = CSTRUCT_SLOT::create({}, "slot1");
    auto& und1 = s1->to<CSTRUCT_SLOT>()->value();
    und1.a     = 5;
    und1.b     = 6;
    und1.c     = 7;
    und1.d     = 8;
    und1.ch    = charp;
    und1.val   = 6.21f;

    auto s2    = CSTRUCT_SLOT::create({}, "slot2");
    auto& und2 = s2->to<CSTRUCT_SLOT>()->value();
    und2.a     = 15;
    und2.b     = 16;
    und2.c     = 17;
    und2.d     = 18;
    und2.ch    = charp;
    und2.val   = 1.14f;

    auto msg = cerberus::Message::create("cstructmessage");
    msg->addSlot(std::move(s1)).addSlot(std::move(s2));

    // also dynamically register the message for the subsequent test
    cerberus::Cerberus::registerTemplate(msg);

    t.send(msg);

    t.join();
}

TEST(messageFactoryTest, slotFromCStruct_part2)
{
    // THIS TEST DEPENDS ON THE slotFromCStruct_creation TEST

    auto msg = cerberus::Cerberus::constructMessage("cstructmessage");

    ASSERT_TRUE(msg->is("cstructmessage"));

    // values are preserved into the template

    auto& s = msg->get<CSTRUCT_SLOT>("slot1");
    logInfo("printing slot1:\na:%u\nb:%u\nc:%u\nd:%u\nch:%s\nval:%f", s.a, s.b, s.c, s.d, s.ch, s.val);

    s = msg->get<CSTRUCT_SLOT>("slot2");
    logInfo("printing slot2:\na:%u\nb:%u\nc:%u\nd:%u\nch:%s\nval:%f", s.a, s.b, s.c, s.d, s.ch, s.val);
}
