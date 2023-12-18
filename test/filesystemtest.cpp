#include <cerberus/cerberus.h>
#include <cerberus/data/filesystem/file.h>
#include <cerberus/data/filesystem/inidatafile.h>
#include <gtest/gtest.h>

TEST(fileTest, creation)
{
    cerberus::data::filesystem::File file("testFile.txt", cerberus::FOM_ReadWriteTrunc);
    ASSERT_TRUE(file.open().ok(true));
    ASSERT_TRUE(file.writeLine("this is a test").ok());
    file.close();
}

TEST(iniDataFileTest, read)  // BEFORE TEST, prepare a read.ini file containing the following data:
{
    cerberus::data::filesystem::IniDataFile file("read.ini");
    EXPECT_TRUE(file.load().ok(true));
    EXPECT_STREQ(file.read_string("string_value").str.c_str(), "this is a string");
    EXPECT_TRUE(file.read_bool("bool_value").isTrue());
    EXPECT_EQ(file.read_integer("integer_value").i, (uint64_t)1010);
    // EXPECT_EQ(file.read_integer("double_value").f, 0.2030f);
}

TEST(iniDataFileTest, write)
{
    cerberus::data::filesystem::File del("write.ini");
    del.deleteFromDisk();
    //
    cerberus::data::filesystem::IniDataFile file("write.ini");
    EXPECT_TRUE(file.write_bool("bool_value", true).ok());
    EXPECT_TRUE(file.write_integer("integer_value", 1010).ok());
    EXPECT_TRUE(file.write_string("string_value", "this is a string").ok());
    EXPECT_TRUE(file.write_double("double_value", 0.2030f).ok());
    //
    EXPECT_TRUE(file.load().ok());
    EXPECT_STREQ(file.read_string("string_value").str.c_str(), "this is a string");
    EXPECT_TRUE(file.read_bool("bool_value").isTrue());
    EXPECT_EQ(file.read_integer("integer_value").i, (uint64_t)1010);
}

TEST(iniDataFileTest, modify)
{
    cerberus::data::filesystem::IniDataFile file("write.ini");
    EXPECT_TRUE(file.load().ok(true));
    EXPECT_EQ(file.read_integer("integer_value").i, (uint64_t)1010);
    EXPECT_TRUE(file.write_integer("integer_value", (uint64_t)1200).ok());  // change to 1200
    //
    cerberus::data::filesystem::IniDataFile file2("write.ini");
    EXPECT_TRUE(file2.load().ok(true));
    EXPECT_EQ(file2.read_integer("integer_value").i, (uint64_t)1200);        // verify
    EXPECT_TRUE(file2.write_integer("integer_value", (uint64_t)1010).ok());  // change back to 1010
}

TEST(iniDataFileTest, writeSections)
{
    cerberus::data::filesystem::IniDataFile file("temp.ini");
    //
    EXPECT_TRUE(file.write_bool("bool_value", true, "section 1").ok());
    EXPECT_TRUE(file.write_integer("integer_value", 1010, "section 2").ok());
    EXPECT_TRUE(file.write_string("string_value", "this is a string", "section 2").ok());
    //
    EXPECT_TRUE(file.write_string("nosectionvalue", "this is the no section value").ok());
    //
    EXPECT_TRUE(file.load().ok(true));
}

TEST(iniDataFileTest, readSections)
{
    cerberus::data::filesystem::IniDataFile file("temp.ini");
    EXPECT_TRUE(file.load().ok(true));
    //
    EXPECT_TRUE(file.read_bool("bool_value", "section 1").expect().isTrue());
    EXPECT_EQ(file.read_integer("integer_value", "section 2").expect().i, 1010);
    EXPECT_EQ(file.read_string("string_value", "section 2").expect().str.compare("this is a string"), 0);
}

TEST(iniDataFileTest, modifySections)
{
    cerberus::data::filesystem::IniDataFile file("temp.ini");
    EXPECT_TRUE(file.load().ok(true));
    file.printDebug();
    EXPECT_TRUE(file.read_bool("bool_value", "section 1").expect("Read failure").isTrue());
    EXPECT_TRUE(file.write_bool("bool_value", false, "section 1").ok(true));
    file.printDebug();
    EXPECT_FALSE(file.read_bool("bool_value", "section 1").expect("Read failure 2").isTrue());
}

TEST(iniDataFileTest, readSections2)
{
    cerberus::data::filesystem::IniDataFile file("temp.ini");
    file.printDebug();
    EXPECT_TRUE(file.load().ok(true));
    file.printDebug();
    EXPECT_FALSE(file.read_bool("bool_value", "section 1").expect().isTrue());
}
