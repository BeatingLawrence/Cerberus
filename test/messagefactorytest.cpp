#include <cerberus.h>
#include <gtest/gtest.h>
#include <message/messagetemplate.h>

TEST(messageFactoryTest, registering_message)
{
    cerberus::Message msg;
    msg.addSlot(cerberus::ByteSlot::create(10));
    msg.addSlot(cerberus::ByteSlot::create(12));
    msg.addSlot(cerberus::ByteSlot::create(14));

    EXPECT_EQ(msg.count(), 3);

    EXPECT_NO_THROW(cerberus::Cerberus::registerMessage(msg, "Test-Message"));
}

TEST(messageFactoryTest, probing_message)
{
    EXPECT_FALSE(cerberus::Cerberus::templateByName("UNKNOWN_MESSAGE").ok());

    auto tmplt = cerberus::Cerberus::templateByName("Test-Message");

    EXPECT_TRUE(tmplt.ok("error"));
    EXPECT_NE(tmplt.value.id, CERBERUS_INVALID_ID);
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
    char* ch;
    float val;
};

new_slot_from_cstruct(CSTRUCT_SLOT, testStruct);

TEST(messageFactoryTest, slotFromCStruct_creation) { auto s = CSTRUCT_SLOT::create(); }
