#include <cerberus/cerberus.h>
#include <cerberus/message/messagetemplate.h>
#include <gtest/gtest.h>

TEST(messageFactoryTest, registering_message)
{
    cerberus::Message msg;
    msg.addSlot(cerberus::ByteSlot::create(10));
    msg.addSlot(cerberus::ByteSlot::create(12));
    msg.addSlot(cerberus::ByteSlot::create(14));
    EXPECT_NO_THROW(cerberus::Cerberus::registerMessage(msg, "Test-Message"));
}

TEST(messageFactoryTest, probing_message)
{
    EXPECT_FALSE(cerberus::Cerberus::templateByName("UNKNOWN_MESSAGE").ok());

    auto tmplt = cerberus::Cerberus::templateByName("Test-Message");

    EXPECT_TRUE(tmplt.ok());
    EXPECT_NE(tmplt.value.id, CERBERUS_INVALID_ID);
}

TEST(messageFactoryTest, constructing_message)
{
    auto msg = cerberus::Cerberus::constructMessage("Test-Message");
    EXPECT_EQ(msg->count(), 3);
}
