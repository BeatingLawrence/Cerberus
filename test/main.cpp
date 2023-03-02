#include <gtest/gtest.h>
#include <cerberus/cerberus.h>
#include <iostream>
#include <cerberus/message/slot/charslot.h>
#include <cerberus/core/cerberusfactory.h>

int main(int argc, char* argv[])
{
    auto parms = cerberus::Cerberus::cerberusDefaultParms();
    parms->logSetup.logLevel = cerberus::LL_Debug;
    cerberus::Cerberus::init(parms);
    //start testing
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    cerberus::Cerberus::deinit();
    return ret;
}
