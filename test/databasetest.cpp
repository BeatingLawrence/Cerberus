#include <cerberus/cerberus.h>
#include <cerberus/data/database/sqldatabase.h>
#include <gtest/gtest.h>

#include "src/data/database/sqldata.h"

using namespace cerberus::data::database;

class DatabaseTest : public ::testing::Test
{
   public:
    SQLDatabase* db;

   protected:
    virtual void SetUp() override
    {
        return;
        db = new SQLDatabase("postgresql://test:test@localhost:5432/testdb");
        // connects to a local postgresql server with:
        //   username: test
        //   password: test
        //   port: 5432
        //   database name: testdb
    };

    virtual void TearDown() override { return;delete db; };
};

TEST_F(DatabaseTest, connection)
{
    GTEST_SKIP();
    if (db->isFailed())
    {
        // logInfo(db->failureReason());
        ASSERT_FALSE(true);
    }
}

TEST_F(DatabaseTest, createTable)
{
    GTEST_SKIP();
    if (db->isFailed())
    {
        // logInfo(db->failureReason());
        ASSERT_FALSE(true);
    }

    SQLTablePrototype prototype("test");
    prototype.add("ID", SQLTablePrototype::SQLDataType::SDT_BigInt)
        .add("name", SQLTablePrototype::SQLDataType::SDT_VarChar, 255)
        .add("country", SQLTablePrototype::SQLDataType::SDT_VarChar, 255)
        .add("heigth", SQLTablePrototype::SQLDataType::SDT_Double)
        .add("drivingLicense", SQLTablePrototype::SQLDataType::SDT_Boolean);
    ASSERT_EQ(db->createTable(prototype).res, cerberus::OR_OK);
}

TEST_F(DatabaseTest, insertInto)
{
    GTEST_SKIP();
    if (db->isFailed())
    {
        // logInfo(db->failureReason());
        ASSERT_FALSE(true);
    }

    SQLTablePrototype prototype("test");
    ASSERT_EQ(db->queryPrototype(prototype).res, cerberus::OR_OK);
    SQLBlock block("test");
    block.setPrototype(prototype);
    SQLRow row;
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
    ASSERT_EQ(db->insertBlock(block).res, cerberus::OR_OK);
}

TEST_F(DatabaseTest, queryResult)
{
    GTEST_SKIP();
    if (db->isFailed())
    {
        // logInfo("test started with a failed database: ", db->failureReason().c_str());
        ASSERT_FALSE(true);
    }

    SQLBlock block;
    ASSERT_EQ(db->querytable("test", block).res, cerberus::OR_OK);
    EXPECT_TRUE(block.structured());

    for (auto&& type : block.prototype())
    {
        logInfo("col type %s", type.typeString().c_str());
    }

    for (auto&& row : block)
    {
        for (auto&& cell : row)
        {
            logInfo("%s", cell.raw().c_str());
        }

        logInfo("=========");
    }

    EXPECT_EQ(block[0][0].toInt(), 1);  // check columns
    EXPECT_EQ(block[1][0].toInt(), 2);
    EXPECT_EQ(block[2][0].toInt(), 3);
    EXPECT_EQ(block[3][0].toInt(), 4);
    //
    EXPECT_EQ(block[0][4].toBool(), true);  // check columns
    EXPECT_EQ(block[1][4].toBool(), true);
    EXPECT_EQ(block[2][4].toBool(), false);
    EXPECT_EQ(block[3][4].toBool(), true);
}

TEST_F(DatabaseTest, dropTable)
{
    GTEST_SKIP();
    if (db->isFailed())
    {
        // logInfo(db->failureReason());
        ASSERT_FALSE(true);
    }

    EXPECT_EQ(db->dropTable("test").res, cerberus::OR_OK);
}
