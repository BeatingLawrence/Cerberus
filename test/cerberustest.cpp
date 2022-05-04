#include <gtest/gtest.h>
#include <cerberus/cerberus.h>
#include <cerberus/define.h>

TEST(cerberusTest, init)
{
    cerberus::Cerberus* provider = cerberus::Cerberus::provider();
    ASSERT_NE(provider, nullptr);
    cerberus::CerberusInitParms parms = cerberus::Cerberus::cerberusDefaultParms();
    provider->init(parms);
}

TEST(cerberusTest, strPrint)
{
    EXPECT_STREQ(cerberus::Cerberus::strPrint("%s %u", "test", 20u).c_str(), "test 20");
}

TEST(cerberusTest, logTest)
{
    cerberus::Cerberus::log("Info log");
    cerberus::Cerberus::log("Warning log", cerberus::Cerberus::LL_Warning);
    cerberus::Cerberus::log("Error log", cerberus::Cerberus::LL_Error);
    cerberus::Cerberus::log("Debug log", cerberus::Cerberus::LL_Debug);
}
