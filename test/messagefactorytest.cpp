#include <cerberus/cerberus.h>
#include <cerberus/core/cerberusfactory.h>
#include <cerberus/core/cerberusregister.h>
#include <cerberus/message/messagetemplate.h>
#include <cerberus/message/slot/charslot.h>
#include <gtest/gtest.h>

TEST(messageFactoryTest, registering_message)
{
    cerberus::message::Message msg;
    msg.addSlot(cerberus::message::slot::CharSlot::create(10));
    msg.addSlot(cerberus::message::slot::CharSlot::create(12));
    msg.addSlot(cerberus::message::slot::CharSlot::create(14));
    EXPECT_NO_THROW(cerberus::core::CerberusFactory::registerMessage(msg, "Test-Message"));
}

TEST(messageFactoryTest, probing_message)
{
    auto tmplt = cerberus::core::CerberusRegister::msgTemplateByName("UNKNOWN_MESSAGE");
    EXPECT_EQ(tmplt.id(), CERBERUS_INVALID_ID);
    EXPECT_FALSE(tmplt.isObjValid());
    tmplt = cerberus::core::CerberusRegister::msgTemplateByName("Test-Message");
    EXPECT_NE(tmplt.id(), CERBERUS_INVALID_ID);
    EXPECT_TRUE(tmplt.isObjValid());
}

TEST(messageFactoryTest, constructing_message)
{
    auto msg = cerberus::core::CerberusFactory::messageConstruct("Test-Message");
    EXPECT_EQ(msg->count(), 3);
}
