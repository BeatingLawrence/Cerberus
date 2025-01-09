#include <cerberus.h>
#include <data/database/database.h>
#include <data/database/dbdata.h>
#include <gtest/gtest.h>

using namespace cerberus;

//==========================Postgres backend tests============================

class PGDatabaseTest : public ::testing::Test
{
   public:
    Database* db;

   protected:
    virtual void SetUp() override
    {
        return;
        db = new Database(DBB_PostgreSQL);
        db->init("postgresql://test:test@localhost:5432/testdb");
        // connects to a local postgresql server with:
        //   username: test
        //   password: test
        //   port: 5432
        //   database name: testdb
    };

    virtual void TearDown() override
    {
        return;
        delete db;
    };
};

TEST_F(PGDatabaseTest, connection)
{
    GTEST_SKIP();

    ASSERT_TRUE(db->ready());
}

TEST_F(PGDatabaseTest, createTable)
{
    GTEST_SKIP();

    ASSERT_TRUE(db->ready());

    DBTableProto prototype("test");
    prototype.add("ID", DBDataType::DDT_BigInt)
        .add("name", DBDataType::DDT_VarChar, 255)
        .add("country", DBDataType::DDT_VarChar, 255)
        .add("heigth", DBDataType::DDT_Double)
        .add("drivingLicense", DBDataType::DDT_Boolean);
    ASSERT_EQ(db->createTable(prototype).res, cerberus::OR_OK);
}

TEST_F(PGDatabaseTest, insertInto)
{
    GTEST_SKIP();

    ASSERT_TRUE(db->ready());

    auto proto = db->queryPrototype("test");
    ASSERT_TRUE(proto.ok());

    DBTableBlock block("test");
    block.setPrototype(proto.value);
    DBRow row;
    row.append(1);
    row.append("Josh");
    row.append("USA");
    row.append(1.6f);
    row.append(true);
    block.append(row);
    row.clear();
    row.append(2);
    row.append("Jason");
    row.append("Italy");
    row.append(1.9f);
    row.append(true);
    block.append(row);
    row.clear();
    row.append(3);
    row.append("Joshua");
    row.append("UK");
    row.append(1.75f);
    row.append(false);
    block.append(row);
    row.clear();
    row.append(4);
    row.append("Jerry");
    row.append("Russia");
    row.append(1.88f);
    row.append(true);
    block.append(row);
    ASSERT_TRUE(db->insertBlock(block).ok());
}

TEST_F(PGDatabaseTest, queryResult)
{
    GTEST_SKIP();

    ASSERT_TRUE(db->ready());

    auto table = db->querytable("test");
    ASSERT_TRUE(table.ok());
    EXPECT_TRUE(table.value.structured());

    for (auto&& type : table.value.prototype())
    {
        logInfo("col type %s", type.typeString().c_str());
    }

    for (auto&& row : table.value)
    {
        for (auto&& cell : row)
        {
            logInfo("%s", cell.raw().toString().c_str());
        }

        logInfo("=========");
    }

    EXPECT_EQ(table.value[0][0].toInt(), 1);  // check columns
    EXPECT_EQ(table.value[1][0].toInt(), 2);
    EXPECT_EQ(table.value[2][0].toInt(), 3);
    EXPECT_EQ(table.value[3][0].toInt(), 4);
    //
    EXPECT_EQ(table.value[0][4].toBool(), true);  // check columns
    EXPECT_EQ(table.value[1][4].toBool(), true);
    EXPECT_EQ(table.value[2][4].toBool(), false);
    EXPECT_EQ(table.value[3][4].toBool(), true);
}

TEST_F(PGDatabaseTest, dropTable)
{
    GTEST_SKIP();

    ASSERT_TRUE(db->ready());

    EXPECT_EQ(db->dropTable("test").res, cerberus::OR_OK);
}

//=========================Filesystem backend tests===========================

class FSDatabaseTest : public ::testing::Test
{
   public:
    Database* db;

   protected:
    virtual void SetUp() override
    {
        db = new Database(DBB_Filesystem);
        db->init("testdb.cdb");
    };

    virtual void TearDown() override { delete db; };
};

TEST_F(FSDatabaseTest, createTable)
{
    ASSERT_TRUE(db->ready());

    // table creation

    if (false)
    {
        DBTableProto prototype("test");
        prototype.add("ID", DBDataType::DDT_BigInt)
            .add("name", DBDataType::DDT_VarChar, 255)
            .add("country", DBDataType::DDT_VarChar, 255)
            .add("heigth", DBDataType::DDT_Double)
            .add("drivingLicense", DBDataType::DDT_Boolean);
        EXPECT_TRUE(db->createTable(prototype).ok("createTable error"));
    }

    // data addition

    if (false)
    {
        auto proto = db->queryPrototype("test");
        EXPECT_TRUE(proto.ok("queryproto error"));

        DBTableBlock block(proto.value);
        DBRow row;
        row.append(1);
        row.append("Josh");
        row.append("USA");
        row.append(1.6f);
        row.append(true);
        block.append(row);
        EXPECT_TRUE(db->insertBlock(block).ok("insert block error"));
    }

    // query section

    if (true)
    {
        auto r = db->queryPrototype("test");
        EXPECT_TRUE(r.ok("queryprototype error"));

        logInfo("Table prototype:");
        for (auto&& el : r.value)
        {
            logInfo("COL \"%s\", %s(%u)", el.name().c_str(), el.typeString().c_str(), el.mod());
        }

        logInfo("Content:");

        auto block = db->querytable("test");
        EXPECT_TRUE(block.ok("querytable error"));

        for (auto&& row : block.value)
        {
            std::string rr;
            for (auto&& cell : row)
            {
                rr.append(cell.raw().toHex());
                rr += " ";
            }
            logInfo("%s", rr.c_str());
        }
    }
}
