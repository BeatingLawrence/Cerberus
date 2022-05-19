#include <gtest/gtest.h>
#include <cerberus/cerberus.h>
#include <cerberus/message/slot/charslot.h>
#include <cerberus/message/messagetemplate.h>

TEST(messageFactoryTest, registering_message)
{
    cerberus::message::Message msg;
    msg.addSlot(cerberus::message::slot::CharSlot::create(10));
    msg.addSlot(cerberus::message::slot::CharSlot::create(12));
    msg.addSlot(cerberus::message::slot::CharSlot::create(14));
    EXPECT_NO_THROW(cerberus::Cerberus::registerMessage(msg, "Test-Message"));
}

TEST(messageFactoryTest, probing_message)
{
    uint32_t typeID = cerberus::Cerberus::messageIdByName("UNKNOWN_MESSAGE");
    EXPECT_EQ(typeID, CERBERUS_INVALID_ID);
    typeID = cerberus::Cerberus::messageIdByName("Test-Message");
    EXPECT_NE(typeID, CERBERUS_INVALID_ID);
}

TEST(messageFactoryTest, constructing_message)
{
    cerberus::message::cerberus_message msg =
        cerberus::Cerberus::messageConstruct(cerberus::Cerberus::messageIdByName("Test-Message"));
    EXPECT_NE(msg, nullptr);
    EXPECT_EQ(msg->count(), 3);
}
