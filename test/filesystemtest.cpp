#include <cerberus/cerberus.h>
#include <cerberus/data/filesystem/file.h>
#include <cerberus/data/filesystem/inidatafile.h>
#include <gtest/gtest.h>

TEST(fileTest, creation)
{
    cerberus::data::filesystem::File file("testFile.txt", cerberus::FOM_ReadWrite);
    file.open();
    file.writeLine("this is a test");
    file.close();
}

TEST(iniDataFileTest, read)  // BEFORE TEST, prepare a read.ini file containing the following data:
{
    cerberus::data::filesystem::IniDataFile file("read.ini");
    EXPECT_TRUE(file.load());
    EXPECT_STREQ(file.read_string("string_value").c_str(), "this is a string");
    EXPECT_TRUE(file.read_bool("bool_value"));
    EXPECT_EQ(file.read_integer("integer_value"), (uint64_t)1010);
}

TEST(iniDataFileTest, write)
{
    cerberus::data::filesystem::IniDataFile file("write.ini");
    //
    file.write_bool("bool_value", true);
    file.write_integer("integer_value", 1010);
    file.write_string("string_value", "this is a string");
    //
    file.load();
    EXPECT_STREQ(file.read_string("string_value").c_str(), "this is a string");
    EXPECT_TRUE(file.read_bool("bool_value"));
    EXPECT_EQ(file.read_integer("integer_value"), (uint64_t)1010);
}

TEST(iniDataFileTest, modify)
{
    cerberus::data::filesystem::IniDataFile file("read.ini");
    file.load();
    EXPECT_EQ(file.read_integer("integer_value"), (uint64_t)1010);
    file.write_integer("integer_value", (uint64_t)1200);
    EXPECT_EQ(file.read_integer("integer_value"), (uint64_t)1200);
    file.write_integer("integer_value", (uint64_t)1010);
}
