#include <cerberus/cerberus.h>
#include <cerberus/core/cerberusfactory.h>
#include <cerberus/message/slot/charslot.h>
#include <gtest/gtest.h>

#include <iostream>

int main(int argc, char* argv[])
{
    auto parms              = cerberus::Cerberus::cerberusDefaultParms();
    parms.logSetup.logLevel = cerberus::LL_Debug;
    parms.useCiphers        = true;
    cerberus::Cerberus::init(parms);
    // start testing
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    cerberus::Cerberus::deinit();
    return ret;
}
