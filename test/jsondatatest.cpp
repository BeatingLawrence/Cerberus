#include <cerberus.h>
#include <data/jsondata.h>
#include <gtest/gtest.h>

using namespace crb;

static bool resources()
{
    // check for the testing resources
    return File::existsAsFile("jsontest.json").ok();
}

TEST(jsonDataTest, create_generate)
{
    JsonData root;

    root.add(JsonData("test_string", "This is a test string!"));
    root.add(JsonData("bool_flag", true));
    root.add(JsonData("number_value", 0.2122f));

    JsonData days("days_of_week");
    days.add("Sunday");
    days.add("Monday");
    days.add("Tuesday");
    days.add("Wednesday");
    days.add("Thursday");
    days.add("Friday");
    days.add("Saturday");

    root.add(days);

    std::string s;
    root.toStr(s);
    logInfo("JSON DATA: %s", s.c_str());
    //
    // generate JSON:
    auto gen = root.generate();
    EXPECT_TRUE(gen.ok());

    logInfo("GENERATED JSON:\n%s", gen.value.toString().c_str());
}

TEST(jsonDataTest, parse)
{
    if (!resources()) GTEST_SKIP();

    JsonData data;
    File f("jsontest.json");
    ASSERT_TRUE(f.open().ok());
    ASSERT_TRUE(data.parse(f).ok());
    std::string s;
    data.toStr(s);
    logInfo("JSON DATA: %s", s.c_str());
}

TEST(jsonDataTest, copy_generate)
{
    if (!resources()) GTEST_SKIP();

    JsonData data;
    File f("jsontest.json");
    ASSERT_TRUE(f.open().ok());
    ASSERT_TRUE(data.parse(f).ok());

    auto gen = data.generate();
    EXPECT_TRUE(gen.ok());
    logInfo("GENERATED JSON:\n%s", gen.value.toString().c_str());
}

TEST(jsonDataTest, search)
{
    if (!resources()) GTEST_SKIP();

    JsonData data;
    File f("jsontest.json");
    ASSERT_TRUE(f.open().ok());
    ASSERT_TRUE(data.parse(f).ok());
    auto found = data.search("street_address");

    ASSERT_TRUE(found.isValid());

    std::string s;
    found.toStr(s);
    logInfo("JSON DATA: %s", s.c_str());
    //

    auto gen = data.generate();
    EXPECT_TRUE(gen.ok());
    logInfo("GENERATED JSON:\n%s", gen.value.toString().c_str());
}
