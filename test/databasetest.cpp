#include <gtest/gtest.h>
#include <cerberus/data/database/sqldatabase.h>
#include <cerberus/data/database/sqlresult.h>
#include <cerberus/data/database/sqlrow.h>
#include <cerberus/cerberus.h>

using namespace cerberus::data::database;

class DatabaseTest : public ::testing::Test
{
    public:
        SQLDatabase* db;

    protected:
        virtual void SetUp() override
        {
            db = new SQLDatabase("postgresql://test:test@localhost:5432/testdb");
            //connects to a local postgresql server with:
            //  username: test
            //  password: test
            //  port: 5432
            //  database name: testdb
        };

        virtual void TearDown() override
        {
            delete db;
        };
};

TEST_F(DatabaseTest, connection)
{
    if(db->isFailed())
    {
        logInfo(db->failureReason());
        ASSERT_FALSE(true);
    }
}

TEST_F(DatabaseTest, createTable)
{
    if(db->isFailed())
    {
        logInfo(db->failureReason());
        ASSERT_FALSE(true);
    }

    SQLTablePrototype prototype("test");
    prototype.add("ID", SQLTablePrototype::SQLDataType::SDT_BigInt)
    .add("name", SQLTablePrototype::SQLDataType::SDT_VarChar, 255)
    .add("country", SQLTablePrototype::SQLDataType::SDT_VarChar, 255);
    ASSERT_EQ(db->createTable(prototype), SQLDatabase::OperationResult::OR_OK);
}

TEST_F(DatabaseTest, insertInto)
{
    if(db->isFailed())
    {
        logInfo(db->failureReason());
        ASSERT_FALSE(true);
    }

    SQLTablePrototype prototype("test");
    prototype.add("ID", SQLTablePrototype::SQLDataType::SDT_BigInt)
    .add("name", SQLTablePrototype::SQLDataType::SDT_VarChar, 255)
    .add("country", SQLTablePrototype::SQLDataType::SDT_VarChar, 255);
    ASSERT_EQ(db->queryPrototype(prototype), SQLDatabase::OperationResult::OR_OK);
    SQLBlock block;
    SQLRow row;
    row.append("1");
    row.append("first");
    row.append("USA");
    block.append(row);
    row.clear();
    row.append("2");
    row.append("second");
    row.append("Italy");
    block.append(row);
    row.clear();
    row.append("3");
    row.append("third");
    row.append("UK");
    block.append(row);
    row.clear();
    row.append("4");
    row.append("fourth");
    row.append("Russia");
    block.append(row);
    ASSERT_EQ(db->insertBlock(prototype, block), SQLDatabase::OperationResult::OR_OK);
}

TEST_F(DatabaseTest, queryResult)
{
    if(db->isFailed())
    {
        logInfo("test started with a failed database: ", db->failureReason().c_str());
        ASSERT_FALSE(true);
    }

    SQLBlock block;
    ASSERT_EQ(db->queryBlock("SELECT * FROM test;", block), SQLDatabase::OperationResult::OR_OK);

    for(size_t i = 0; i < block.size(); i++)
    {
        logInfo("ROW %u:", i);

        for(size_t j = 0; j < block[i].size(); j++)
        {
            logInfo("Value %u: %s", j, block[i][j].c_str());
        }

        logInfo("=========");
    }
}

TEST_F(DatabaseTest, dropTable)
{
    if(db->isFailed())
    {
        logInfo(db->failureReason());
        ASSERT_FALSE(true);
    }

    EXPECT_EQ(db->command("DROP TABLE test;"), SQLDatabase::OperationResult::OR_OK);
}
