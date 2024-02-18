#include <cerberus/cerberus.h>
#include <cerberus/core/cerberusutils.h>
#include <gtest/gtest.h>

using namespace cerberus;

TEST(cerberusTest, strPrint) { EXPECT_STREQ(core::CerberusUtils::strPrint("%s %u", "test", 20u).c_str(), "test 20"); }

TEST(cerberusTest, logTest)
{
    logInfo("Info Log");
    logWarning("Warning Log");
    logError("Error Log");
    logDebug("Debug Log");
}

TEST(cerberusTest, environmentVariable)
{
    logInfo(core::CerberusUtils::environmentVariable("APPDATA").value.c_str());  // works only on windows
}

TEST(cerberusTest, cerberusVersion)
{
    auto v = Cerberus::cerberusVersion();
    logInfo("Version test:");
    logInfo("%s", v.text.c_str());

    const char* t = "";

    switch (v.type)
    {
        case CerbVersion::Alpha:
            t = "ALPHA";
            break;
        case CerbVersion::Beta:
            t = "BETA";
            break;
        case CerbVersion::Release:
            t = "RELEASE";
            break;
    }

    logInfo("Components: %u %u %u %s", v.major, v.minor, v.patch, t);
}

TEST(cerberusTest, hostTest)
{
    {
        cerberus::Host h;
        EXPECT_FALSE(h.isValidRemote());
        h.fromString("192.168.3.67:9000");
        EXPECT_EQ(h.octect[0], 192);
        EXPECT_EQ(h.octect[1], 168);
        EXPECT_EQ(h.octect[2], 3);
        EXPECT_EQ(h.octect[3], 67);
        EXPECT_EQ(h.port, 9000);
        //
        h.fromString("192.168..67:9000");
        h.fromString("192.168.5000.67:9000");
        h.fromString("192.168.3.256:9000");
        h.fromString("192.168.3.256:65536");
        h.fromString("192..3.256:65536");
        h.fromString("192.0.3.255:6552");
        h.fromString("0.0.0.0:6552");
        //
        h.fromString("172.16.1.43");
        EXPECT_EQ(h.octect[0], 172);
        EXPECT_EQ(h.octect[1], 16);
        EXPECT_EQ(h.octect[2], 1);
        EXPECT_EQ(h.octect[3], 43);
    }
    //
    {
        cerberus::Host h("192.168.100.1:4444");
        EXPECT_TRUE(h.isValidRemote());
        EXPECT_EQ(h.octect[0], 192);
        EXPECT_EQ(h.octect[1], 168);
        EXPECT_EQ(h.octect[2], 100);
        EXPECT_EQ(h.octect[3], 1);
        EXPECT_EQ(h.port, 4444);
    }
    //
    {
        cerberus::Host h("google.com:80");
        EXPECT_TRUE(h.isValidRemote());
        EXPECT_EQ(h.octect[0], 0);
        EXPECT_EQ(h.octect[1], 0);
        EXPECT_EQ(h.octect[2], 0);
        EXPECT_EQ(h.octect[3], 0);
        EXPECT_EQ(h.port, 80);
    }
    //
    {
        cerberus::Host h("0.0.0.400:4444");
        EXPECT_FALSE(h.isValidRemote());
        EXPECT_EQ(h.octect[0], 0);
        EXPECT_EQ(h.octect[1], 0);
        EXPECT_EQ(h.octect[2], 0);
        EXPECT_EQ(h.octect[3], 0);
        EXPECT_EQ(h.port, 0);
    }
}
