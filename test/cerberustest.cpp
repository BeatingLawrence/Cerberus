#include <gtest/gtest.h>

#include <cerberus/cerberus.h>

//#include "../cerberus.h"

TEST(cerberusTest, strPrint)
{
    EXPECT_STREQ(cerberus::Cerberus::strPrint("%s %u", "test", 20u).c_str(), "test 20");
}
