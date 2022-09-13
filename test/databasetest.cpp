#include <gtest/gtest.h>
#include <cerberus/data/database/sqldatabase.h>
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
            //  port: 5433
            //  database name: testdb
        };

        virtual void TearDown() override
        {
            delete db;
        };
};

TEST_F(DatabaseTest, connection)
{
    logInfo(db->failureReason());
    ASSERT_FALSE(db->isFailed());   //checks if connection was successfully established
}

//TODO to implement other tests
