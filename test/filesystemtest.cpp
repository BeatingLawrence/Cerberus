#include <cerberus.h>
#include <data/filesystem/directory.h>
#include <data/filesystem/file.h>
#include <data/filesystem/inidatafile.h>
#include <gtest/gtest.h>

using namespace cerberus;

TEST(fileTest, creation)
{
    File file("testFile.txt", cerberus::FOM_ReadWriteTrunc);
    ASSERT_TRUE(file.open().ok());
    ASSERT_TRUE(file.writeLine("this is a test").ok());
    file.close();
}

TEST(fileTest, readLine)
{
    {
        File file("readLineTest.txt", cerberus::FOM_ReadWriteTrunc);
        ASSERT_TRUE(file.open().ok());
        for (int i = 0; i < 100; i++) file.write("helloworld");  // big chunks to test line buffering
        file.writeLine("EOL_HERE");
        for (int i = 0; i < 100; i++) file.write("helloworld");
        file.writeLine("EOF_HERE");
        file.close();
    }

    {
        File file("readLineTest.txt", cerberus::FOM_Read);
        ASSERT_TRUE(file.open().ok());
        while (true)
        {
            auto x = file.readLine();
            if (x.ok())
            {
                logInfo("OK");
                logInfo("is eof:%s, read:%s", x.hasOptional(OR_EOF) ? "yes" : "no", x.value.c_str());
            }
            else
            {
                logInfo("NOT OK");
                EXPECT_EQ(x.res, OR_EOF);
                break;
            }
        }
        file.close();
        // file.remove();
    }
}

TEST(iniDataFileTest, read)  // BEFORE TEST, prepare a read.ini file containing the following data:
{
    if (File::existsAsFile("read.ini").fail()) GTEST_SKIP();

    IniDataFile file("read.ini");
    EXPECT_TRUE(file.load().ok());
    EXPECT_STREQ(file.read_string("string_value").value.c_str(), "this is a string");
    EXPECT_TRUE(file.read_bool("bool_value").value);
    EXPECT_EQ(file.read_integer("integer_value").value, (uint64_t)1010);
    // EXPECT_EQ(file.read_integer("double_value").f, 0.2030f);
}

TEST(iniDataFileTest, write)
{
    File del("write.ini");
    del.remove();
    //
    IniDataFile file("write.ini");
    EXPECT_TRUE(file.write_bool("bool_value", true).ok());
    EXPECT_TRUE(file.write_integer("integer_value", 1010).ok());
    EXPECT_TRUE(file.write_string("string_value", "this is a string").ok());
    EXPECT_TRUE(file.write_double("double_value", 0.2030f).ok());
    //
    EXPECT_TRUE(file.load().ok());
    EXPECT_STREQ(file.read_string("string_value").value.c_str(), "this is a string");
    EXPECT_TRUE(file.read_bool("bool_value").value);
    EXPECT_EQ(file.read_integer("integer_value").value, (uint64_t)1010);
}

TEST(iniDataFileTest, modify)
{
    IniDataFile file("write.ini");
    EXPECT_TRUE(file.load().ok());
    EXPECT_EQ(file.read_integer("integer_value").value, (uint64_t)1010);
    EXPECT_TRUE(file.write_integer("integer_value", (uint64_t)1200).ok());  // change to 1200
    //
    IniDataFile file2("write.ini");
    EXPECT_TRUE(file2.load().ok());
    EXPECT_EQ(file2.read_integer("integer_value").value, (uint64_t)1200);    // verify
    EXPECT_TRUE(file2.write_integer("integer_value", (uint64_t)1010).ok());  // change back to 1010
}

TEST(iniDataFileTest, writeSections)
{
    IniDataFile file("temp.ini");
    //
    EXPECT_TRUE(file.write_bool("bool_value", true, "section 1").ok());
    EXPECT_TRUE(file.write_integer("integer_value", 1010, "section 2").ok());
    EXPECT_TRUE(file.write_string("string_value", "this is a string", "section 2").ok());
    //
    EXPECT_TRUE(file.write_string("nosectionvalue", "this is the no section value").ok());
    //
    EXPECT_TRUE(file.load().ok());
}

TEST(iniDataFileTest, readSections)
{
    IniDataFile file("temp.ini");
    EXPECT_TRUE(file.load().ok());
    //
    EXPECT_TRUE(file.read_bool("bool_value", "section 1").expect().value);
    EXPECT_EQ(file.read_integer("integer_value", "section 2").expect().value, 1010);
    EXPECT_EQ(file.read_string("string_value", "section 2").expect().value.compare("this is a string"), 0);
}

TEST(iniDataFileTest, modifySections)
{
    IniDataFile file("temp.ini");
    EXPECT_TRUE(file.load().ok());
    EXPECT_TRUE(file.read_bool("bool_value", "section 1").expect("Read failure").value);
    EXPECT_TRUE(file.write_bool("bool_value", false, "section 1").ok());
    EXPECT_FALSE(file.read_bool("bool_value", "section 1").expect("Read failure 2").value);
}

TEST(iniDataFileTest, readSections2)
{
    IniDataFile file("temp.ini");
    EXPECT_TRUE(file.load().ok());
    EXPECT_FALSE(file.read_bool("bool_value", "section 1").expect().value);
}

TEST(directoryTest, get)
{
    Directory d("logs");
    d.get(true).expect();
    std::string s;
    d.toStr(s);
    logInfo("%s", s.c_str());

    logInfo("size of logs dir: %u", d.size());

    if (d.files().empty()) return;
    logInfo("path of first file: %s", d.files().front().completePath().toStr().c_str());
}

TEST(fileTest, search)
{
    {
        File f("searchtest.txt");
        f.setOpenMode(cerberus::FOM_ReadWriteTrunc);
        ASSERT_TRUE(f.open().ok());

        for (int i = 0; i < 20; i++) ASSERT_TRUE(f.writeLine("garbage").ok());

        ASSERT_TRUE(f.write("/chewchew").ok());

        for (int i = 0; i < 10; i++) ASSERT_TRUE(f.writeLine("garbage").ok());

        f.close();
    }

    File f("searchtest.txt");
    ASSERT_TRUE(f.open().ok());
    auto pos = f.search("chewchew");

    EXPECT_TRUE(pos.ok());

    logInfo("position of chewchew: %llu", pos.value);

    f.close();

    EXPECT_EQ(pos.value, 161);
}

TEST(fileTest, writeBehindEOF)
{
    {
        File f("writeBehindEOFTest.txt");
        f.setOpenMode(cerberus::FOM_ReadWriteTrunc);
        ASSERT_TRUE(f.open().ok());

        for (int i = 0; i < 20; i++) ASSERT_TRUE(f.writeLine("Hello!").ok());

        f.close();
    }

    File f("writeBehindEOFTest.txt");  // size 140

    ASSERT_TRUE(f.open().ok());
    EXPECT_TRUE(f.writeExpand("one more line\n").ok("write fail"));

    EXPECT_EQ(f.size().value, 154);  // this uses cursor

    f.close();

    EXPECT_EQ(f.size().value, 154);  // this uses stat()
}

TEST(fileTest, tempFile)
{
    {
        auto f = File::tmpFile();  // generate a temp file

        EXPECT_TRUE(f.writeLine("this file is temporary").ok());
    }  // automatically closed (and deleted) on stack unwind

    auto f = File::tmpFile();

    EXPECT_TRUE(f.writeLine("this file was temporary but now its not").ok());

    f.move("nontempfile.txt").expect();

    f.close();
}

TEST(fileTest, insertion_readUntil)
{
    {
        auto f = File("insertionTest.txt");
        f.setOpenMode(cerberus::FOM_ReadWriteTrunc);
        ASSERT_TRUE(f.open().ok());
        for (int i = 0; i < 100; i++) EXPECT_TRUE(f.writeLine("this file  is  temporary").ok());  // 25 b
    }

    auto f = File("insertionTest.txt");
    f.setOpenMode(cerberus::FOM_ReadWrite);
    ASSERT_TRUE(f.open().ok());

    ASSERT_TRUE(f.seek(100).ok());
    ASSERT_TRUE(f.insert("THIS_IS_INSERTED").ok());

    f.close();

    ASSERT_TRUE(f.open().ok());
    auto pos = f.search("THIS_IS_INSERTED");

    EXPECT_TRUE(pos.ok());

    logInfo("position of token: %llu", pos.value);

    f.close();

    EXPECT_EQ(pos.value, 100);

    // readUntil test
    ASSERT_TRUE(f.open().ok());
    auto rr = f.readUntil("THIS_IS_INSERTED");
    ASSERT_TRUE(rr.ok());

    EXPECT_EQ(rr.value.size(), 100);

    logInfo("readUntil returned %u bytes: %s", rr.value.size(), rr.value.toString().c_str());
    f.close();
}
