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

    EXPECT_TRUE(db->command("CREATE TABLE test ( ID int, name varchar(255), country varchar(255) );"));

    if(db->isFailed())
    {
        logInfo(db->failureReason());
        ASSERT_FALSE(true);
    }

    EXPECT_TRUE(db->command("INSERT INTO test VALUES (1, 'first', 'USA');"));

    if(db->isFailed())
    {
        logInfo(db->failureReason());
        ASSERT_FALSE(true);
    }

    EXPECT_TRUE(db->command("INSERT INTO test VALUES (2, 'second', 'Italy');"));
    EXPECT_TRUE(db->command("INSERT INTO test VALUES (3, 'third', 'UK');"));
    EXPECT_TRUE(db->command("INSERT INTO test VALUES (4, 'fourth', 'Russia');"));
}

TEST_F(DatabaseTest, queryResult)
{
    if(db->isFailed())
    {
        logInfo("test started with a failed database: ", db->failureReason().c_str());
        ASSERT_FALSE(true);
    }

    SQLResult result = db->query("SELECT * FROM test;");

    if(result.isFailed())
    {
        if(db->isFailed())
        {
            logInfo("database failure: %s", db->failureReason().c_str());
        }
        else
        {
            logInfo("result failure: %s", result.failureReason().c_str());
        }

        ASSERT_FALSE(true);
    }

    ASSERT_NE(result.size(), 0);

    for(size_t i = 0; i < result.size(); i++)
    {
        logInfo("ROW %u:", i);

        for(size_t j = 0; j < result[i].size(); j++)
        {
            logInfo("Value %u: %s", j, result[i][j].c_str());
        }

        logInfo("=========");
    }
}

TEST_F(DatabaseTest, querySingleRow)
{
    if(db->isFailed())
    {
        logInfo("test started with a failed database: ", db->failureReason().c_str());
        ASSERT_FALSE(true);
    }

    SQLRow row = db->querySingleRow("SELECT * FROM test WHERE ID = '2';");

    if(row.isFailed())
    {
        if(db->isFailed())
        {
            logInfo("database failure: %s", db->failureReason().c_str());
        }
        else
        {
            logInfo("row failure: %s", row.failureReason().c_str());
        }

        ASSERT_FALSE(true);
    }

    ASSERT_NE(row.size(), 0);

    for(size_t i = 0; i < row.size(); i++)
    {
        logInfo("Value %u: %s", i, row[i].c_str());
    }
}

TEST_F(DatabaseTest, dropTable)
{
    if(db->isFailed())
    {
        logInfo(db->failureReason());
        ASSERT_FALSE(true);
    }

    EXPECT_TRUE(db->command("DROP TABLE test;"));
}
