#include <cerberus.h>
#include <core/cerberusutils.h>
#include <gtest/gtest.h>

using namespace cerberus;

TEST(utilityTest, misc)
{
    logInfo("%s", CerberusUtils::toLower("Hello").c_str());
    logInfo("%s", CerberusUtils::toUpper("Hello").c_str());

    EXPECT_TRUE(CerberusUtils::areEqual("Hello", "Hello", cerberus::WM_CaseSensitive));
    EXPECT_FALSE(CerberusUtils::areEqual("Hello", "hello", cerberus::WM_CaseSensitive));
    EXPECT_TRUE(CerberusUtils::areEqual("Hello", "hello", cerberus::WM_CaseInsensitive));
    EXPECT_TRUE(CerberusUtils::areEqual("hello", "hello", cerberus::WM_CaseInsensitive));

    EXPECT_FALSE(CerberusUtils::areEqual("hello", "hey", cerberus::WM_CaseInsensitive));
    EXPECT_FALSE(CerberusUtils::areEqual("hello", "hey", cerberus::WM_CaseSensitive));
}

TEST(utilityTest, regex)
{
    std::string s("hello, this is the Cerberus test suite");
    EXPECT_TRUE(CerberusUtils::patternMatch(s, ".*suite").ok("regex error"));
    EXPECT_TRUE(CerberusUtils::patternMatch(s, ".*suito").fail());
    EXPECT_STREQ("hello", CerberusUtils::substrUntil_regex(s, "(\\,\\ )").expect().value.c_str());
    EXPECT_STREQ("suite", CerberusUtils::substrFrom_regex(s, "(t\\ )").expect().value.c_str());
}

TEST(utilityTest, buffer_replace)
{
    ByteBuffer b("hello, this is a buffer and the words have to be replaced in this buffer");
    auto res = b.replace("buffer", "book");
    EXPECT_TRUE(res.ok());
    logInfo(b.toString());
}
