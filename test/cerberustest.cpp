#include <gtest/gtest.h>
#include <cerberus/core/cerberuslog.h>
#include <cerberus/core/cerberusutils.h>
#include <cerberus/cerberus.h>

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

TEST(cerberusTest, environmentVariable)
{
    logInfo(core::CerberusUtils::environmentVariable("APPDATA").c_str());   //works only on windows
}

TEST(cerberusTest, cerberusVersion)
{
    logInfo(Cerberus::cerberusVersion());
}

TEST(cerberusTest, hostTest)
{
    cerberus::Host h;
    EXPECT_TRUE(h.fromString("192.168.3.67:9000"));
    EXPECT_EQ(h.octect[0], 192);
    EXPECT_EQ(h.octect[1], 168);
    EXPECT_EQ(h.octect[2], 3);
    EXPECT_EQ(h.octect[3], 67);
    EXPECT_EQ(h.port, 9000);
    //
    EXPECT_FALSE(h.fromString("192.168..67:9000"));
    EXPECT_FALSE(h.fromString("192.168.5000.67:9000"));
    EXPECT_FALSE(h.fromString("192.168.3.256:9000"));
    //
    EXPECT_TRUE(h.fromString("172.16.1.43"));
    EXPECT_EQ(h.octect[0], 172);
    EXPECT_EQ(h.octect[1], 16);
    EXPECT_EQ(h.octect[2], 1);
    EXPECT_EQ(h.octect[3], 43);
}
