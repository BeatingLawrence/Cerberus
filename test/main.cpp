#include <gtest/gtest.h>
#include <cerberus/cerberus.h>
#include <iostream>
#include <cerberus/message/slot/charslot.h>
#include <cerberus/core/cerberusfactory.h>

int main(int argc, char* argv[])
{
    cerberus::Cerberus::init(cerberus::Cerberus::cerberusDefaultParms());
    //register a generic message to test Thread messages exchange
    cerberus::message::Message msg;
    msg.addSlot(cerberus::message::slot::CharSlot::create());
    msg.addSlot(cerberus::message::slot::CharSlot::create());
    msg.addSlot(cerberus::message::slot::CharSlot::create());
    cerberus::core::CerberusFactory::registerMessage(msg, "PingPongMessage");
    //start testing
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    cerberus::Cerberus::deinit();
    return ret;
}
