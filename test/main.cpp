#include <cerberus.h>
#include <core/libloader.h>
#include <gtest/gtest.h>

using namespace cerberus;
using namespace cerberus::core;

int main(int argc, char* argv[])
{
    auto parms                               = Cerberus::cerberusDefaultParms();
    parms.logSetup.cerbLogLevel              = cerberus::LL_Debug;
    parms.logSetup.appLogLevel               = cerberus::LL_Debug;
    parms.coreSetup.threadPool               = 1;       // 1 thread in the threadpool
    parms.coreSetup.backupThreadMaxTime      = 2000;    // 2 seconds
    parms.logSetup.fileLogConf.logDirMaxSize = 500000;  //~ 500K
    Cerberus::init(parms);
    // cerberus::core::LibLoader::fastload("libtestobject.dylib");
    //   start testing
    logInfo("=================================");
    logInfo("  START TESTING CERBERUS %s", Cerberus::cerberusVersion().text.c_str());
    logInfo("=================================");
    //
    ::testing::InitGoogleTest(&argc, argv);
    int ret = RUN_ALL_TESTS();
    Cerberus::deinit();
    return ret;
}
