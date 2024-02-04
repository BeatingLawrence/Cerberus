#include <cerberus/cerberus.h>
#include <cerberus/core/libloader.h>
#include <gtest/gtest.h>

int main(int argc, char* argv[])
{
    auto parms                         = cerberus::Cerberus::cerberusDefaultParms();
    parms.logSetup.cerberusLogLevel    = cerberus::LL_Debug;
    parms.logSetup.applicationLogLevel = cerberus::LL_Debug;
    parms.useCiphers                   = true;
    cerberus::Cerberus::init(parms);
    // cerberus::core::LibLoader::fastload("libtestobject.dylib");
    //   start testing
    logInfo("=================================");
    logInfo("  START TESTING CERBERUS %s", cerberus::Cerberus::cerberusVersion().text.c_str());
    logInfo("=================================");
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    cerberus::Cerberus::deinit();
    return ret;
}
