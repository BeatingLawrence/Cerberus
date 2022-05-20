#include <gtest/gtest.h>
#include <cerberus/core/cerberuslog.h>
#include <cerberus/core/cerberusutils.h>

using namespace cerberus;

TEST(cerberusTest, strPrint)
{
    EXPECT_STREQ(core::CerberusUtils::strPrint("%s %u", "test", 20u).c_str(), "test 20");
}

TEST(cerberusTest, logTest)
{
    logInfo("Info Log");
    logWarning("Warning Log");
    logError("Error Log");
    debug("Debug Log");
}
