#include <cerberus.h>
#include <data/filesystem/csvdatafile.h>
#include <data/filesystem/directory.h>
#include <data/filesystem/file.h>
#include <data/filesystem/inidatafile.h>
#include <gtest/gtest.h>

using namespace crb;

TEST(fileTest, creation)
{
    File file("testFile.txt", crb::FOM_ReadWriteTrunc);
    ASSERT_TRUE(file.open().ok());
    ASSERT_TRUE(file.writeLine("this is a test").ok());
    file.close();
}

TEST(fileTest, existanceCheck)
{
    EXPECT_TRUE(File::existsAsFile("testFile.txt").ok());
}

TEST(fileTest, readLine)
{
    {
        File file("readLineTest.txt", crb::FOM_ReadWriteTrunc);
        ASSERT_TRUE(file.open().ok());
        for (int i = 0; i < 100; i++) file.write("helloworld");  // big chunks to test line buffering
        file.writeLine("EOL_HERE");
        for (int i = 0; i < 100; i++) file.write("helloworld");
        file.writeLine("EOF_HERE");
        file.close();
    }

    {
        File file("readLineTest.txt", crb::FOM_Read);
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
    EXPECT_STREQ(file.read("string_value").value.get().c_str(), "this is a string");
    EXPECT_TRUE(file.read("bool_value").value.getBool());
    EXPECT_EQ(file.read("integer_value").value.getInt(), (uint64_t)1010);
    // EXPECT_EQ(file.read_integer("double_value").f, 0.2030f);
}

TEST(iniDataFileTest, write)
{
    File del("write.ini");
    del.remove();
    //
    IniDataFile file("write.ini");
    EXPECT_TRUE(file.write("bool_value", Opaque(true)).ok());
    EXPECT_TRUE(file.write("integer_value", Opaque((int64_t)1010)).ok());
    EXPECT_TRUE(file.write("string_value", Opaque(std::string("this is a string"))).ok());
    EXPECT_TRUE(file.write("double_value", Opaque(0.2030f)).ok());
    //
    EXPECT_TRUE(file.load().ok());
    EXPECT_STREQ(file.read("string_value").value.get().c_str(), "this is a string");
    EXPECT_TRUE(file.read("bool_value").value.getBool());
    EXPECT_EQ(file.read("integer_value").value.getInt(), (uint64_t)1010);
}

TEST(iniDataFileTest, modify)
{
    IniDataFile file("write.ini");
    EXPECT_TRUE(file.load().ok());
    EXPECT_EQ(file.read("integer_value").value.getInt(), (uint64_t)1010);
    EXPECT_TRUE(file.write("integer_value", Opaque((int64_t)1200)).ok());  // change to 1200
    //
    IniDataFile file2("write.ini");
    EXPECT_TRUE(file2.load().ok());
    EXPECT_EQ(file2.read("integer_value").value.getInt(), (uint64_t)1200);    // verify
    EXPECT_TRUE(file2.write("integer_value", Opaque((int64_t)1010)).ok());  // change back to 1010
}

TEST(iniDataFileTest, writeSections)
{
    IniDataFile file("temp.ini");
    //
    EXPECT_TRUE(file.write("bool_value", Opaque(true), "section 1").ok());
    EXPECT_TRUE(file.write("integer_value", Opaque((int64_t)1010), "section 2").ok());
    EXPECT_TRUE(file.write("string_value", Opaque(std::string("this is a string")), "section 2").ok());
    //
    EXPECT_TRUE(file.write("nosectionvalue", Opaque(std::string("this is the no section value"))).ok());
    //
    EXPECT_TRUE(file.load().ok());
}

TEST(iniDataFileTest, readSections)
{
    IniDataFile file("temp.ini");
    EXPECT_TRUE(file.load().ok());
    //
    EXPECT_TRUE(file.read("bool_value", "section 1").expect().value.getBool());
    EXPECT_EQ(file.read("integer_value", "section 2").expect().value.getInt(), 1010);
    EXPECT_EQ(file.read("string_value", "section 2").expect().value.get().compare("this is a string"), 0);
}

TEST(iniDataFileTest, modifySections)
{
    IniDataFile file("temp.ini");
    EXPECT_TRUE(file.load().ok());
    EXPECT_TRUE(file.read("bool_value", "section 1").expect("Read failure").value.getBool());
    EXPECT_TRUE(file.write("bool_value", Opaque(false), "section 1").ok());
    EXPECT_FALSE(file.read("bool_value", "section 1").expect("Read failure 2").value.getBool());
}

TEST(iniDataFileTest, readSections2)
{
    IniDataFile file("temp.ini");
    EXPECT_TRUE(file.load().ok());
    EXPECT_FALSE(file.read("bool_value", "section 1").expect().value.getBool());
}

TEST(directoryTest, get)
{
    Directory d("logs");
    d.get(true).expect();
    std::string s;
    d.toStr(s);
    logInfo("%s", s.c_str());

    logInfo("size of logs dir: %llu", static_cast<unsigned long long>(d.size()));

    if (d.files().empty()) return;
    logInfo("path of first file: %s", d.files().front().completePath().toStr().c_str());
}

TEST(fileTest, search)
{
    {
        File f("searchtest.txt");
        f.setOpenMode(crb::FOM_ReadWriteTrunc);
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
        f.setOpenMode(crb::FOM_ReadWriteTrunc);
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

TEST(csvDataFileTest, loadReadWrite)
{
    const char* fname = "csvdatafile_test.csv";

    // Prepare a CSV file with one malformed row (wrong type) that should be skipped.
    {
        File f(fname, crb::FOM_ReadWriteTrunc);
        ASSERT_TRUE(f.open().ok());
        ASSERT_TRUE(f.writeLine("id,price,flag").ok());
        ASSERT_TRUE(f.writeLine("1,2.5,true").ok());    // types: int, double, bool
        ASSERT_TRUE(f.writeLine("2,3.14,false").ok());  // valid row
        ASSERT_TRUE(f.writeLine("bad,7.2,true").ok());  // invalid: id not int -> should be discarded
        f.close();
    }

    CSVDataFile csv(fname);
    auto res = csv.load();
    EXPECT_TRUE(res.ok());
    EXPECT_TRUE(res.hasOptional(OR_Failure));  // one row discarded

    EXPECT_EQ(csv.size(), static_cast<crb::SIZE>(2));
    EXPECT_EQ(csv.columnPos("price"), 1);
    EXPECT_EQ(csv.type(0), DT_Integer);
    EXPECT_EQ(csv.type(1), DT_Double);
    EXPECT_EQ(csv.type(2), DT_Bool);

    auto v01 = csv.read(0, 1);
    EXPECT_NEAR(v01.getDouble(), 2.5, 1e-6);

    csv.write(1, 1, Opaque(9.9));
    auto v11 = csv.read(1, 1);
    EXPECT_NEAR(v11.getDouble(), 9.9, 1e-6);

    CSVDataFile::CSVRecord rec;
    rec.addValue(Opaque(int64_t(3)));
    rec.addValue(Opaque(4.2));
    rec.addValue(Opaque(true));
    csv.addRecord(rec);

    EXPECT_EQ(csv.size(), static_cast<crb::SIZE>(3));
    auto last = csv.read(2, 0);
    EXPECT_EQ(last.getInt(), 3);

    auto colId = csv.getColumn("id");
    EXPECT_EQ(colId.size(), (size_t)3);
    EXPECT_EQ(colId[0].getInt(), 1);
    EXPECT_EQ(colId[2].getInt(), 3);

    File(fname).remove();
}

TEST(csvDataFileTest, largeSequential)
{
    const char* fname = "csvdatafile_seq.csv";
    const int rows    = 1000;

    // Generate CSV with 5 columns: int,int,double,bool,double
    {
        File f(fname, crb::FOM_ReadWriteTrunc);
        ASSERT_TRUE(f.open().ok());
        ASSERT_TRUE(f.writeLine("id,count,value,flag,ratio").ok());
        for (int i = 1; i <= rows; ++i)
        {
            int id       = i;
            int count    = i * 2;
            double val   = i + 0.5;
            bool flag    = (i % 2 == 0);
            double ratio = (static_cast<double>(count) / (id + 1));
            std::string line =
                CerberusUtils::strPrint("%d,%d,%.6f,%s,%.6f", id, count, val, flag ? "true" : "false", ratio);
            ASSERT_TRUE(f.writeLine(line).ok());
        }
        f.close();
    }

    CSVDataFile csv(fname);
    auto res = csv.load();
    ASSERT_TRUE(res.ok());
    EXPECT_EQ(csv.size(), static_cast<crb::SIZE>(rows));

    // spot-check column types
    EXPECT_EQ(csv.type(0), DT_Integer);
    EXPECT_EQ(csv.type(1), DT_Integer);
    EXPECT_EQ(csv.type(2), DT_Double);
    EXPECT_EQ(csv.type(3), DT_Bool);
    EXPECT_EQ(csv.type(4), DT_Double);

    // validate sequential content
    for (int i = 0; i < rows; ++i)
    {
        int idExp       = i + 1;
        int countExp    = idExp * 2;
        double valExp   = idExp + 0.5;
        bool flagExp    = ((idExp % 2) == 0);
        double ratioExp = static_cast<double>(countExp) / (idExp + 1);

        EXPECT_EQ(csv.read(i, 0).getInt(), idExp);
        EXPECT_EQ(csv.read(i, 1).getInt(), countExp);
        EXPECT_NEAR(csv.read(i, 2).getDouble(), valExp, 1e-6);
        EXPECT_EQ(csv.read(i, 3).getBool(), flagExp);
        EXPECT_NEAR(csv.read(i, 4).getDouble(), ratioExp, 1e-6);
    }

    // column length and a couple of values
    auto colId = csv.getColumn(0);
    ASSERT_EQ(colId.size(), (size_t)rows);
    EXPECT_EQ(colId.front().getInt(), 1);
    EXPECT_EQ(colId.back().getInt(), rows);

    auto colByName = csv.getColumn("ratio");
    ASSERT_EQ(colByName.size(), (size_t)rows);
    EXPECT_NEAR(colByName.front().getDouble(), 2.0 / 2.0, 1e-6);  // count/id+1 with id=1 -> 2/2
    EXPECT_NEAR(colByName.back().getDouble(), (rows * 2) / static_cast<double>(rows + 1), 1e-6);

    File(fname).remove();
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
        f.setOpenMode(crb::FOM_ReadWriteTrunc);
        ASSERT_TRUE(f.open().ok());
        for (int i = 0; i < 100; i++) EXPECT_TRUE(f.writeLine("this file  is  temporary").ok());  // 25 b
    }

    auto f = File("insertionTest.txt");
    f.setOpenMode(crb::FOM_ReadWrite);
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

    EXPECT_EQ(rr.value.size(), static_cast<crb::LSIZE>(100));

    logInfo("readUntil returned %llu bytes: %s", static_cast<unsigned long long>(rr.value.size()),
            rr.value.toString().c_str());
    f.close();
}
