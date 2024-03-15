#include <cerberus/cerberus.h>
#include <cerberus/message/messagetemplate.h>
#include <gtest/gtest.h>

TEST(messageFactoryTest, registering_message)
{
    cerberus::message::Message msg;
    msg.addSlot(cerberus::ByteSlot::create(10));
    msg.addSlot(cerberus::ByteSlot::create(12));
    msg.addSlot(cerberus::ByteSlot::create(14));
    EXPECT_NO_THROW(cerberus::Cerberus::registerMessage(msg, "Test-Message"));
}

TEST(messageFactoryTest, probing_message)
{
    EXPECT_THROW(cerberus::Cerberus::msgTemplateByName("UNKNOWN_MESSAGE"), std::exception);

    auto tmplt = cerberus::Cerberus::msgTemplateByName("Test-Message");
    EXPECT_NE(tmplt.id(), CERBERUS_INVALID_ID);
    EXPECT_TRUE(tmplt.isObjValid());
}

TEST(messageFactoryTest, constructing_message)
{
    auto msg = cerberus::Cerberus::messageConstruct("Test-Message");
    EXPECT_EQ(msg->count(), 3);
}
