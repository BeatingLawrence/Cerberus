#include <cerberus/cerberus.h>
#include <cerberus/core/cerberusfactory.h>
#include <cerberus/message/slot/charslot.h>
#include <gtest/gtest.h>

int main(int argc, char* argv[])
{
    auto parms                         = cerberus::Cerberus::cerberusDefaultParms();
    parms.logSetup.cerberusLogLevel    = cerberus::LL_Error;
    parms.logSetup.applicationLogLevel = cerberus::LL_Error;
    parms.useCiphers                   = true;
    cerberus::Cerberus::init(parms);
    // start testing
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    cerberus::Cerberus::deinit();
    return ret;
}
