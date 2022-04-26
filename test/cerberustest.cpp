#include <gtest/gtest.h>
#include <cerberus/cerberus.h>
#include <cerberus/define.h>

TEST(cerberusTest, init)
{
    cerberus::Cerberus* provider = cerberus::Cerberus::provider();
    ASSERT_NE(provider, nullptr);
    cerberus::Cerberus::CerberusInitParms parms = {};
    parms.terminalFormattingDisabled = false;
    parms.stdout_formatting.foregroundColor = TERMINAL_FOREGROUND_GREEN;
    parms.stdout_formatting.backgroundColor = TERMINAL_BACKGROUND_WHITE;
    parms.stderr_formatting.foregroundColor = TERMINAL_FOREGROUND_WHITE;
    parms.stderr_formatting.backgroundColor = TERMINAL_BACKGROUND_RED;
    provider->init(parms);
}

TEST(cerberusTest, strPrint)
{
    EXPECT_STREQ(cerberus::Cerberus::strPrint("%s %u", "test", 20u).c_str(), "test 20");
}

TEST(cerberusTest, stdoutPrint)
{
    cerberus::Cerberus::stdoutPrint("This is a STDOUT print");
}

TEST(cerberusTest, stderrPrint)
{
    cerberus::Cerberus::stderrPrint("This is a STDERR print");
}
