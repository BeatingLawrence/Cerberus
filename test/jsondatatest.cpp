#include <cerberus/cerberus.h>
#include <cerberus/data/jsondata.h>
#include <gtest/gtest.h>

using namespace cerberus::data;

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
    ByteBuffer bb;
    EXPECT_TRUE(root.generate(bb).ok(true));

    logInfo("GENERATED JSON:\n%s", bb.toString().c_str());
}

TEST(jsonDataTest, parse)
{
    JsonData data;
    filesystem::File f("jsontest.json");
    ASSERT_TRUE(f.open().ok(true));
    ASSERT_TRUE(data.parse(f).ok(true));
    std::string s;
    data.toStr(s);
    logInfo("JSON DATA: %s", s.c_str());
}

TEST(jsonDataTest, copy_generate)
{
    JsonData data;
    filesystem::File f("jsontest.json");
    ASSERT_TRUE(f.open().ok(true));
    ASSERT_TRUE(data.parse(f).ok(true));
    ByteBuffer bb;
    EXPECT_TRUE(data.generate(bb).ok(true));
    logInfo("GENERATED JSON:\n%s", bb.toString().c_str());
}

TEST(jsonDataTest, search)
{
    JsonData data;
    filesystem::File f("jsontest.json");
    ASSERT_TRUE(f.open().ok(true));
    ASSERT_TRUE(data.parse(f).ok(true));
    auto found = data.deepSearch("address");

    ASSERT_NE(found, nullptr);

    std::string s;
    found->toStr(s);
    logInfo("JSON DATA: %s", s.c_str());
    //
    ByteBuffer bb;
    EXPECT_TRUE(found->generate(bb).ok(true));
    logInfo("GENERATED JSON:\n%s", bb.toString().c_str());
}

TEST(jsonDataTest, search2)
{
    JsonData data;
    filesystem::File f("../../jsontestlong.json");
    ASSERT_TRUE(f.open().ok(true));
    ASSERT_TRUE(data.parse(f).ok(true));
    auto found = data.deepSearch("result");

    ASSERT_NE(found, nullptr);
    ASSERT_TRUE(found->isArray());

    auto& element = found->get();

    std::string s;
    element.toStr(s);
    logInfo("JSON DATA: %s", s.c_str());
    //
    ByteBuffer bb;
    EXPECT_TRUE(element.generate(bb).ok(true));
    logInfo("GENERATED JSON:\n%s", bb.toString().c_str());
}
