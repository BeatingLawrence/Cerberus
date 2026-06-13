#include <cerberus.h>
#include <data/database/database.h>
#include <data/database/dbdata.h>
#include <data/filesystem/file.h>
#include <gtest/gtest.h>

#include <filesystem>

using namespace crb;

namespace
{
std::string testTempFilePath(const std::string& filename)
{
    return (std::filesystem::temp_directory_path() / filename).string();
}
}

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
    ASSERT_EQ(db->createTable(prototype).res, crb::OR_OK);
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

    EXPECT_EQ(db->dropTable("test").res, crb::OR_OK);
}

//=========================Filesystem backend tests===========================

class FSDatabaseTest : public ::testing::Test
{
   public:
    Database db;
    std::string path;

    FSDatabaseTest()
        : db(DBB_Filesystem) {};

   protected:
    virtual void SetUp() override
    {
        path = testTempFilePath("fsdb_unittest.cdb");
        File::remove(path);  // best-effort cleanup
    };

    virtual void TearDown() override
    {
        db.deinit();
        File::remove(path);
    };
};

TEST_F(FSDatabaseTest, createInsertQuery)
{
    ASSERT_TRUE(db.init(path).ok("init error"));

    ASSERT_TRUE(db.ready());

    DBTableProto prototype("test");
    prototype.add("id", DBDataType::DDT_Int, 0, DBF_PrimaryKey)
        .add("name", DBDataType::DDT_VarChar, 32)
        .add("active", DBDataType::DDT_Boolean)
        .add("score", DBDataType::DDT_Double);
    ASSERT_TRUE(db.createTable(prototype).ok("createTable error"));

    DBTableBlock block(prototype);
    DBRow row;
    row.append(1);
    row.append("alpha");
    row.append(true);
    row.append(1.5);
    block.append(row);
    row.clear();
    row.append(2);
    row.append("beta");
    row.append(false);
    row.append(3.0);
    block.append(row);
    row.clear();
    row.append(3);
    row.append("gamma");
    row.append(true);
    row.append(4.5);
    block.append(row);

    ASSERT_TRUE(db.insertBlock(block).ok("insert block error"));

    // Verify the saved prototype matches the table definition used at creation time.
    auto proto = db.queryPrototype("test");
    ASSERT_TRUE(proto.ok("queryproto error"));
    EXPECT_TRUE(proto.value.isEqual(prototype));

    // Verify structural integrity (row count + type/size checks) and a few representative values.
    auto table = db.querytable("test");
    ASSERT_TRUE(table.ok("querytable error"));
    EXPECT_EQ(table.value.size(), block.size());
    EXPECT_TRUE(table.value.verify(prototype));
    logDebug("table 'test' after insert:\n%s", table.value.toString().c_str());

    EXPECT_EQ(table.value[0][0].toInt(), 1);
    EXPECT_STREQ(table.value[1][1].raw().toString().c_str(), "beta");
    EXPECT_EQ(table.value[2][2].toBool(), true);
    EXPECT_DOUBLE_EQ((double)table.value[1][3].toReal(), 3.0);

    // dump raw file bytes to help troubleshoot format
    File f(path, FOM_Read);
    ASSERT_TRUE(f.open().ok("file open"));
    ByteBuffer raw;
    ASSERT_TRUE(f.read(raw).ok("file read"));
    logInfo("FSDB raw dump:\n%s", raw.toBinaryDump().c_str());
}

TEST_F(FSDatabaseTest, reopenAndQuery)
{
    ASSERT_TRUE(db.init(path).ok("init error"));

    DBTableProto prototype("reopen");
    prototype.add("id", DBDataType::DDT_Int, 0, DBF_PrimaryKey).add("flag", DBDataType::DDT_Boolean);
    ASSERT_TRUE(db.createTable(prototype).ok("createTable error"));

    DBTableBlock block(prototype);
    DBRow row;
    for (int i = 0; i < 5; ++i)
    {
        row.clear();
        row.append(i);
        row.append((i % 2) == 0);
        block.append(row);
    }
    ASSERT_TRUE(db.insertBlock(block).ok("insert block error"));

    db.deinit();

    ASSERT_TRUE(db.init(path).ok("reinit error"));

    // Verify persisted content after reopen by checking count and per-row deterministic values.
    auto table = db.querytable("reopen");
    ASSERT_TRUE(table.ok("querytable error"));
    EXPECT_EQ(table.value.size(), block.size());
    EXPECT_TRUE(table.value.verify(prototype));
    for (size_t i = 0; i < table.value.size(); ++i)
    {
        EXPECT_EQ(table.value[i][0].toInt(), static_cast<int64_t>(i));
        EXPECT_EQ(table.value[i][1].toBool(), (i % 2) == 0);
    }
    logDebug("table 'reopen' after reopen:\n%s", table.value.toString().c_str());
}

TEST_F(FSDatabaseTest, multipleTablesSingleFile)
{
    ASSERT_TRUE(db.init(path).ok("init error"));

    // Table A
    DBTableProto protoA("alpha");
    protoA.add("id", DDT_Int, 0, DBF_PrimaryKey).add("txt", DDT_VarChar, 16);
    ASSERT_TRUE(db.createTable(protoA).ok("create alpha error"));

    DBTableBlock blockA(protoA);
    DBRow row;
    for (int i = 0; i < 3; ++i)
    {
        row.clear();
        row.append(i);
        row.append(std::string("a") + std::to_string(i));
        blockA.append(row);
    }
    ASSERT_TRUE(db.insertBlock(blockA).ok("insert alpha error"));

    // Table B
    DBTableProto protoB("beta");
    protoB.add("id", DDT_Int, 0, DBF_PrimaryKey).add("val", DDT_Double).add("flag", DDT_Boolean);
    ASSERT_TRUE(db.createTable(protoB).ok("create beta error"));

    DBTableBlock blockB(protoB);
    for (int i = 0; i < 2; ++i)
    {
        row.clear();
        row.append(i + 10);
        row.append(i * 1.25);
        row.append((i % 2) == 0);
        blockB.append(row);
    }
    ASSERT_TRUE(db.insertBlock(blockB).ok("insert beta error"));

    // Verify both tables are readable in the same physical database file.
    auto qa = db.querytable("alpha");
    ASSERT_TRUE(qa.ok("query alpha error"));
    EXPECT_EQ(qa.value.size(), blockA.size());
    EXPECT_TRUE(qa.value.verify(protoA));

    auto qb = db.querytable("beta");
    ASSERT_TRUE(qb.ok("query beta error"));
    EXPECT_EQ(qb.value.size(), blockB.size());
    EXPECT_TRUE(qb.value.verify(protoB));

    // Verify both tables survive a full deinit/init cycle with expected values.
    db.deinit();
    ASSERT_TRUE(db.init(path).ok("reinit error"));

    qa = db.querytable("alpha");
    ASSERT_TRUE(qa.ok("query alpha after reopen"));
    EXPECT_EQ(qa.value.size(), blockA.size());
    EXPECT_STREQ(qa.value[1][1].raw().toString().c_str(), "a1");
    logDebug("table 'alpha' after reopen:\n%s", qa.value.toString().c_str());

    qb = db.querytable("beta");
    ASSERT_TRUE(qb.ok("query beta after reopen"));
    EXPECT_EQ(qb.value.size(), blockB.size());
    EXPECT_DOUBLE_EQ((double)qb.value[0][1].toReal(), 0.0);
    EXPECT_TRUE(qb.value[0][2].toBool());
    logDebug("table 'beta' after reopen:\n%s", qb.value.toString().c_str());
}

TEST_F(FSDatabaseTest, modifyTest)
{
    std::string modPath = testTempFilePath("fsdb_modify.cdb");
    File::remove(modPath);

    Database ldb(DBB_Filesystem);
    ASSERT_TRUE(ldb.init(modPath).ok("init error"));

    DBTableProto proto("alltypes");
    proto.add("big", DDT_BigInt, 0, DBF_PrimaryKey)
        .add("small", DDT_SmallInt)
        .add("mid", DDT_Int)
        .add("realv", DDT_Real)
        .add("doublev", DDT_Double)
        .add("moneyv", DDT_Money)
        .add("flag", DDT_Boolean)
        .add("bits", DDT_Bit, 5)
        .add("vbits", DDT_VarBit, 10)
        .add("ch", DDT_Char, 4)
        .add("vch", DDT_VarChar, 16);
    ASSERT_TRUE(ldb.createTable(proto).ok("createTable error"));

    DBTableBlock block(proto);
    DBRow row;
    row.append(1234567890123LL);
    row.append(-123);
    row.append(42);
    row.append(1.25f);
    row.append(9.75L);
    row.append(5000);
    row.append(true);
    row.append(std::vector<bool>{1, 0, 1, 1, 0});
    row.append(std::vector<bool>{1, 0, 0, 1, 1, 0, 1, 0, 1});
    row.append("ABCD");
    row.append("hello");
    block.append(row);

    row.clear();
    row.append(-1);
    row.append(7);
    row.append(-99);
    row.append(-2.5f);
    row.append(0.125L);
    row.append(123);
    row.append(false);
    row.append(std::vector<bool>{0, 1, 1, 0, 1});
    row.append(std::vector<bool>{0, 1, 0, 1});
    row.append("WXYZ");
    row.append("world");
    block.append(row);

    /* debug verify sizes for maintenance:
       row 0 (hello) and row 1 (world) must align to proto sizes (int64=8, int16=2, int32=4, float=4,
       double=8, money(int32)=4, bool=1, bit(5)=1, varbit(<=10)=ceil(bits/8) so 9 bits -> 2 bytes,
       char(4)=4, varchar("hello"/"world")=5). Uncomment the loop below if you need to inspect raw sizes.
    */
    // for (size_t r = 0; r < block.size(); ++r)
    //     for (size_t c = 0; c < proto.size(); ++c)
    //         logDebug("row %zu col %s size %llu", r, proto[c].name().c_str(),
    //                  static_cast<unsigned long long>(block[r][c].raw().size()));
    // Verify input block is coherent with the declared schema before first write.
    EXPECT_TRUE(block.verify(proto));

    ASSERT_TRUE(ldb.insertBlock(block).ok("first insert error"));
    // Verify first insert produced readable table content before applying modifications.
    auto initialTable = ldb.querytable("alltypes");
    ASSERT_TRUE(initialTable.ok("query after first insert"));
    logDebug("table 'alltypes' after first insert:\n%s", initialTable.value.toString().c_str());

    // modify values by appending an updated block
    DBTableBlock updated(proto);
    for (auto&& r : block)
    {
        DBRow nr    = r;
        nr[0]       = DBCell(nr[0].toInt() + 10);  // big
        int32_t mid = 0;
        nr[2].raw().copyTo(&mid, sizeof(mid));
        nr[2]         = DBCell((int64_t)(mid + 1000));  // mid
        int32_t money = 0;
        nr[5].raw().copyTo(&money, sizeof(money));
        nr[5] = DBCell((int64_t)(money + 1));  // money
        nr[6] = DBCell(!nr[6].toBool());       // flag
        updated.append(nr);
    }
    // Verify modified block is still schema-valid before appending it.
    EXPECT_TRUE(updated.verify(proto));
    ASSERT_TRUE(ldb.insertBlock(updated).ok("second insert error"));

    // Verify merged table shape and key field updates after second insert.
    auto table = ldb.querytable("alltypes");
    ASSERT_TRUE(table.ok("querytable error"));
    EXPECT_EQ(table.value.size(), block.size() + updated.size());
    EXPECT_TRUE(table.value.verify(proto));

    // order after insert() is newest-first
    EXPECT_EQ(table.value[0][0].toInt(), block[0][0].toInt() + 10);
    auto toInt32 = [](const DBCell& c)
    {
        int32_t v = 0;
        c.raw().copyTo(&v, sizeof(v));
        return v;
    };

    EXPECT_EQ(toInt32(table.value[1][2]), toInt32(block[1][2]) + 1000);
    EXPECT_EQ(table.value[0][6].toBool(), !block[0][6].toBool());
    EXPECT_STREQ(table.value[1][10].raw().toString().c_str(), "world");
    EXPECT_EQ(table.value[2][0].toInt(), block[0][0].toInt());

    ldb.deinit();
    File::remove(modPath);
}

TEST_F(FSDatabaseTest, updateBlock)
{
    std::string path = testTempFilePath("fsdb_update.cdb");
    File::remove(path);

    Database db(DBB_Filesystem);
    ASSERT_TRUE(db.init(path).ok("init error"));

    DBTableProto proto("alltypes");
    proto.add("big", DDT_BigInt, 0, DBF_PrimaryKey).add("mid", DDT_Int).add("flag", DDT_Boolean);
    ASSERT_TRUE(db.createTable(proto).ok("createTable error"));

    DBTableBlock block(proto);
    DBRow row;
    row.append(1);
    row.append(10);
    row.append(true);
    block.append(row);

    row.clear();
    row.append(2);
    row.append(20);
    row.append(false);
    block.append(row);

    ASSERT_TRUE(db.insertBlock(block).ok("insert block error"));
    // Verify baseline table is present before running updateBlock.
    auto afterInsert = db.querytable("alltypes");
    ASSERT_TRUE(afterInsert.ok("query after initial insert"));
    logDebug("table 'alltypes' after initial insert:\n%s", afterInsert.value.toString().c_str());

    // update existing rows (subset of columns)
    DBTableProto uproto("alltypes");
    uproto.add("big", DDT_BigInt, 0, DBF_PrimaryKey).add("mid", DDT_Int).add("flag", DDT_Boolean);
    DBTableBlock updates(uproto);

    DBRow up;
    up.append(1);
    up.append(100);
    up.append(false);
    updates.append(up);

    up.clear();
    up.append(2);
    up.append(200);
    up.append(true);
    updates.append(up);

    // Verify updateBlock applies in-place updates for existing keys.
    ASSERT_TRUE(db.updateBlock(updates).ok("updateBlock error"));

    // Verify updated rows are retrievable and contain updated payloads.
    auto table = db.querytable("alltypes");
    ASSERT_TRUE(table.ok("querytable error"));
    auto findRow = [](const DBTableBlock& t, int64_t key) -> const DBRow*
    {
        for (auto&& r : t)
            if (r[0].toInt() == key) return &r;
        return nullptr;
    };
    logDebug("table 'alltypes' after updateBlock:\n%s", table.value.toString().c_str());
    auto r1 = findRow(table.value, 1);
    auto r2 = findRow(table.value, 2);
    ASSERT_NE(r1, nullptr);
    ASSERT_NE(r2, nullptr);
    EXPECT_EQ((*r1)[1].toInt(), 100);
    EXPECT_FALSE((*r1)[2].toBool());
    EXPECT_EQ((*r2)[1].toInt(), 200);
    EXPECT_TRUE((*r2)[2].toBool());

    // Verify updateBlock inserts missing keys when full row data is provided.
    DBTableBlock newRows(proto);
    DBRow nr;
    nr.append(3);
    nr.append(300);
    nr.append(false);
    newRows.append(nr);
    ASSERT_TRUE(db.updateBlock(newRows).ok("updateBlock insert error"));

    auto table2 = db.querytable("alltypes");
    ASSERT_TRUE(table2.ok("querytable error 2"));
    EXPECT_EQ(table2.value.size(), 3u);
    auto r3 = findRow(table2.value, 3);
    ASSERT_NE(r3, nullptr);
    EXPECT_EQ((*r3)[1].toInt(), 300);
    EXPECT_FALSE((*r3)[2].toBool());
    logDebug("table 'alltypes' after insert via updateBlock:\n%s", table2.value.toString().c_str());

    db.deinit();
    File::remove(path);
}

TEST_F(FSDatabaseTest, queryBlockAndDropStress)
{
    const int ROWS = 1000;
    std::string p  = testTempFilePath("fsdb_stress.cdb");
    File::remove(p);

    ASSERT_TRUE(db.init(p).ok("init error"));

    DBTableProto proto("stress");
    proto.add("id", DDT_Int, 0, DBF_PrimaryKey)
        .add("name", DDT_VarChar, 32)
        .add("score", DDT_Double)
        .add("flag", DDT_Boolean);
    ASSERT_TRUE(db.createTable(proto).ok("createTable error"));

    DBTableBlock block(proto);
    DBRow row;
    for (int i = 0; i < ROWS; ++i)
    {
        row.clear();
        row.append(i);
        row.append("name_" + std::to_string(i));
        row.append(i * 0.5);
        row.append((i % 2) == 0);
        block.append(row);
    }
    ASSERT_TRUE(db.insertBlock(block).ok("insert block error"));

    // Verify numeric range filtering and projected columns.
    auto qb = db.queryBlock("stress:[100:199]id:[id,name]");
    ASSERT_TRUE(qb.ok("queryBlock range"));
    EXPECT_EQ(qb.value.size(), 100u);
    if (!qb.value.empty())
    {
        EXPECT_EQ(qb.value[0][0].toInt(), 100);
        EXPECT_STREQ(qb.value[0][1].raw().toString().c_str(), "name_100");
    }

    // Verify regex filtering on string columns.
    auto qb2 = db.queryBlock("stress:{name_9[0-9]}name:[name]");
    ASSERT_TRUE(qb2.ok("queryBlock regex"));
    EXPECT_GE(qb2.value.size(), 10u);

    // Verify exact-match filtering on boolean values.
    auto qb3 = db.queryBlock("stress:[true]flag:[flag]");
    ASSERT_TRUE(qb3.ok("queryBlock bool"));
    EXPECT_EQ(qb3.value.size(), (size_t)((ROWS + 1) / 2));

    // Verify dropTable removes metadata and makes prototype lookup fail.
    ASSERT_TRUE(db.dropTable("stress").ok("dropTable error"));
    auto protoMissing = db.queryPrototype("stress");
    EXPECT_TRUE(protoMissing.res == OR_NotFound);

    File::remove(p);
}

TEST_F(FSDatabaseTest, typedQueryPoliciesDefaultsAndRename)
{
    std::string p = testTempFilePath("fsdb_api.cdb");
    File::remove(p);

    Database ldb(DBB_Filesystem);
    ASSERT_TRUE(ldb.init(p).ok("init error"));

    DBTableProto proto("api");
    proto.add("id", DDT_Int, 0, DBF_PrimaryKey)
        .add("name", DDT_VarChar, 16, DBF_None, DBCell("unset"))
        .add("score", DDT_Int, 0, DBF_None, DBCell((int64_t)7))
        .add("flag", DDT_Boolean, 0, DBF_None, DBCell(true))
        .add("bits", DDT_Bit, 4);
    ASSERT_TRUE(ldb.createTable(proto).ok("createTable error"));

    DBTableBlock seed(proto);
    DBRow row;
    row.append(1);
    row.append("alpha");
    ASSERT_TRUE(seed.append(row).ok("append row1"));

    row.clear();
    row.append(2);
    ASSERT_TRUE(seed.append(row).ok("append row2"));

    ASSERT_TRUE(ldb.insertBlock(seed).ok("insert seed"));

    auto initial = ldb.querytable("api");
    ASSERT_TRUE(initial.ok("query initial"));
    ASSERT_EQ(initial.value.size(), 2u);
    logDebug("table 'api' after seed insert:\n%s", initial.value.toString().c_str());

    bool found1 = false;
    bool found2 = false;
    for (size_t i = 0; i < initial.value.size(); ++i)
    {
        const DBRow& r = initial.value[i];
        if (r["id"].toInt() == 1)
        {
            found1 = true;
            EXPECT_STREQ(r["name"].raw().toString().c_str(), "alpha");
            EXPECT_EQ(r["score"].toInt(), 7);
            EXPECT_TRUE(r["flag"].toBool());
        }
        if (r["id"].toInt() == 2)
        {
            found2 = true;
            EXPECT_STREQ(r["name"].raw().toString().c_str(), "unset");
            EXPECT_EQ(r["score"].toInt(), 7);
            EXPECT_TRUE(r["flag"].toBool());
        }
    }
    EXPECT_TRUE(found1);
    EXPECT_TRUE(found2);

    DBQuery typed("api");
    typed.condition().type   = DBQC_Range;
    typed.condition().column = "id";
    typed.condition().min    = 2;
    typed.condition().max    = 1;
    typed.condition().invertOrder = true;
    typed.addColumn("id");

    auto typedRes = ldb.queryBlock(typed);
    ASSERT_TRUE(typedRes.ok("typed query failed"));
    ASSERT_EQ(typedRes.value.size(), 2u);
    EXPECT_EQ(typedRes.value[0][0].toInt(), 2);
    EXPECT_EQ(typedRes.value[1][0].toInt(), 1);

    DBTableProto up("api");
    up.add("id", DDT_Int, 0, DBF_PrimaryKey).add("score", DDT_Int);
    DBTableBlock updates(up);

    DBRow ur;
    ur.append(1);
    ur.append(100);
    ASSERT_TRUE(updates.append(ur).ok("append update row1"));

    ur.clear();
    ur.append(3);
    ur.append(300);
    ASSERT_TRUE(updates.append(ur).ok("append update row2"));

    auto upOnly = ldb.updateBlock(updates, UP_UpdateOnly);
    EXPECT_EQ(upOnly.res, OR_NotFound);

    auto inOnly = ldb.updateBlock(updates, UP_InsertOnly);
    EXPECT_EQ(inOnly.res, OR_DuplicateKey);

    ASSERT_TRUE(ldb.updateBlock(updates, UP_UpdateInsert).ok("update+insert failed"));

    auto after = ldb.querytable("api");
    ASSERT_TRUE(after.ok("query after update+insert"));
    ASSERT_EQ(after.value.size(), 3u);
    logDebug("table 'api' after update+insert:\n%s", after.value.toString().c_str());

    bool found3 = false;
    for (size_t i = 0; i < after.value.size(); ++i)
    {
        const DBRow& r = after.value[i];
        int64_t id     = r["id"].toInt();
        if (id == 1)
        {
            EXPECT_EQ(r["score"].toInt(), 100);
        }
        else if (id == 3)
        {
            found3 = true;
            EXPECT_EQ(r["score"].toInt(), 300);
            EXPECT_STREQ(r["name"].raw().toString().c_str(), "unset");
            EXPECT_TRUE(r["flag"].toBool());
            EXPECT_EQ(r["bits"].raw().size(), 1u);
        }
    }
    EXPECT_TRUE(found3);

    ASSERT_TRUE(ldb.renameColumn("api", "name", "label").ok("rename column"));
    auto p2 = ldb.queryPrototype("api");
    ASSERT_TRUE(p2.ok("query prototype after rename"));
    EXPECT_GE(p2.value.columnIndex("label"), 0);
    EXPECT_EQ(p2.value.columnIndex("name"), -1);

    auto renamed = ldb.querytable("api");
    ASSERT_TRUE(renamed.ok("query renamed table"));
    logDebug("table 'api' after rename:\n%s", renamed.value.toString().c_str());
    for (size_t i = 0; i < renamed.value.size(); ++i)
    {
        if (renamed.value[i]["id"].toInt() == 3)
        {
            EXPECT_STREQ(renamed.value[i]["label"].raw().toString().c_str(), "unset");
        }
    }

    ldb.deinit();
    File::remove(p);
}

TEST_F(FSDatabaseTest, fsDatabaseBenchmark)
{
    GTEST_SKIP();  // comment to run

    const int ROWS  = 100000;
    const int CHUNK = 5000;
    std::string p   = testTempFilePath("fsdb_bench.cdb");
    File::remove(p);

    ASSERT_TRUE(db.init(p).ok("init error"));

    DBTableProto proto("bench");
    proto.add("id", DDT_Int, 0, DBF_PrimaryKey)
        .add("small", DDT_SmallInt)
        .add("big", DDT_BigInt)
        .add("realv", DDT_Real)
        .add("doublev", DDT_Double)
        .add("moneyv", DDT_Money)
        .add("flag", DDT_Boolean)
        .add("bits", DDT_Bit, 8)
        .add("vbits", DDT_VarBit, 16)
        .add("ch", DDT_Char, 8)
        .add("vch", DDT_VarChar, 32);
    ASSERT_TRUE(db.createTable(proto).ok("createTable error"));

    logDebug("benchmark: creation start %d rows", ROWS);

    DBRow row;
    // Insert in chunks: generate synthetic rows, basic verify, insert chunk
    for (int base = 0; base < ROWS; base += CHUNK)
    {
        DBTableBlock block(proto);
        int limit = std::min(base + CHUNK, ROWS);
        for (int i = base; i < limit; ++i)
        {
            row.clear();
            row.append(i);              // id
            row.append(i % 32000);      // small
            row.append(1000000LL * i);  // big
            row.append(i * 0.1f);       // real
            row.append(i * 0.01);       // double
            row.append(10LL * i);       // money
            row.append((i % 2) == 0);   // flag
            row.append(std::vector<bool>{(bool)(i & 1), (bool)(i & 2), (bool)(i & 4), (bool)(i & 8),
                                         (bool)(i & 16), (bool)(i & 32), (bool)(i & 64),
                                         (bool)(i & 128)});                     // bits(8)
            row.append(std::vector<bool>{1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0});  // 12 bits
            std::string ch = CerberusUtils::strPrint("CH%05d", i % 100000);
            if (ch.size() < 8) ch.append(8 - ch.size(), '0');
            row.append(ch.substr(0, 8));              // char(8)
            row.append("name_" + std::to_string(i));  // varchar
            block.append(row);
        }

        // Verify chunk is not empty before expensive validation and insert.
        if (block.size() == 0) FAIL() << "empty block at base " << base;
        // Verify first row shape quickly to print detailed diagnostics on mismatch.
        if (!block[0].verify(proto))
        {
            for (crb::SIZE c = 0; c < proto.size(); ++c)
            {
                const DBCell& cell = block[0][c];
                crb::LSIZE sz      = cell.raw().size();
                DBDataType t       = proto[c].type();
                DBMOD mod          = proto[c].mod();
                bool ok            = true;
                switch (t)
                {
                    case DDT_BigInt:
                    case DDT_Double:
                        ok = sz == 8;
                        break;
                    case DDT_Int:
                    case DDT_Real:
                    case DDT_Money:
                        ok = sz == 4;
                        break;
                    case DDT_SmallInt:
                        ok = sz == 2;
                        break;
                    case DDT_Boolean:
                        ok = sz == 1;
                        break;
                    case DDT_Bit:
                        ok = (mod == 0) || (sz == CerberusUtils::qceil(mod, 8));
                        break;
                    case DDT_VarBit:
                        ok = (mod == 0) || (sz <= CerberusUtils::qceil(mod, 8));
                        break;
                    case DDT_Char:
                        ok = (mod == 0) || (sz == mod);
                        break;
                    case DDT_VarChar:
                        ok = (mod == 0) || (sz <= mod);
                        break;
                    default:
                        ok = false;
                        break;
                }
                logDebug("first row fail base %d col %s size %llu type %d mod %u cond %s rawhex %s", base,
                         proto[c].name().c_str(), static_cast<unsigned long long>(sz), t, proto[c].mod(),
                         ok ? "ok" : "FAIL", cell.raw().toHex().c_str());
            }
            FAIL() << "first row verify failed at base " << base;
        }
        // Verify full chunk consistency against the schema before writing it.
        if (!block.verify(proto))
        {
            size_t badRow = 0;
            for (; badRow < block.size(); ++badRow)
                if (!block[badRow].verify(proto)) break;
            for (crb::SIZE c = 0; c < proto.size(); ++c)
            {
                const DBCell& cell = block[badRow][c];
                crb::LSIZE sz      = cell.raw().size();
                DBDataType t       = proto[c].type();
                DBMOD mod          = proto[c].mod();
                bool ok            = true;
                switch (t)
                {
                    case DDT_BigInt:
                    case DDT_Double:
                        ok = sz == 8;
                        break;
                    case DDT_Int:
                    case DDT_Real:
                    case DDT_Money:
                        ok = sz == 4;
                        break;
                    case DDT_SmallInt:
                        ok = sz == 2;
                        break;
                    case DDT_Boolean:
                        ok = sz == 1;
                        break;
                    case DDT_Bit:
                        ok = (mod == 0) || (sz == CerberusUtils::qceil(mod, 8));
                        break;
                    case DDT_VarBit:
                        ok = (mod == 0) || (sz <= CerberusUtils::qceil(mod, 8));
                        break;
                    case DDT_Char:
                        ok = (mod == 0) || (sz == mod);
                        break;
                    case DDT_VarChar:
                        ok = (mod == 0) || (sz <= mod);
                        break;
                    default:
                        ok = false;
                        break;
                }
                logDebug("verify fail base %d row %zu col %s size %llu type %d mod %u cond %s", base, badRow,
                         proto[c].name().c_str(), static_cast<unsigned long long>(sz), t, proto[c].mod(),
                         ok ? "ok" : "FAIL");
            }
            FAIL() << "block verify failed at base " << base;
        }
        // Verify chunk insert succeeds before moving to the next chunk.
        ASSERT_TRUE(db.insertBlock(block).ok("insert block error"));
    }

    // Verify file metadata is readable after bulk creation to report resulting size.
    auto stat = File::stat(p);
    ASSERT_TRUE(stat.ok("stat file"));
    logDebug("benchmark: creation complete, file size: %llu bytes", (unsigned long long)stat.value.size);

    // Prepare update progress thresholds
    int nextLog = ROWS / 4;  // 25%

    logDebug("benchmark: update start");
    // Update in chunks: rebuild rows with new values, run updateBlock, log progress
    for (int base = 0; base < ROWS; base += CHUNK)
    {
        DBTableBlock upd(proto);
        int limit = std::min(base + CHUNK, ROWS);
        for (int i = base; i < limit; ++i)
        {
            row.clear();
            row.append(i);                // id key
            row.append((i % 16000) * 2);  // small *2 (fits in int16)
            row.append(2000000LL * i);    // big *2
            row.append(-i * 0.1f);        // real negate
            row.append(i * 0.02);         // double *2
            row.append(10LL * i + 1);     // money +1
            row.append((i % 2) == 1);     // flag flip
            row.append(std::vector<bool>{!(bool)(i & 1), !(bool)(i & 2), !(bool)(i & 4), !(bool)(i & 8),
                                         !(bool)(i & 16), !(bool)(i & 32), !(bool)(i & 64),
                                         !(bool)(i & 128)});                    // bits inverted
            row.append(std::vector<bool>{0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1});  // 12 bits alt (same length)
            std::string ch = CerberusUtils::strPrint("UP%05d", i % 100000);
            if (ch.size() < 8) ch.append(8 - ch.size(), 'X');
            row.append(ch.substr(0, 8));              // char(8)
            row.append("name_" + std::to_string(i));  // varchar (same length to avoid rewrite)
            upd.append(row);
        }
        // Verify each update chunk is accepted before logging progress markers.
        ASSERT_TRUE(db.updateBlock(upd).ok("update block error"));

        int done = std::min(limit, ROWS);
        if (done >= nextLog)
        {
            int pct = (done * 100) / ROWS;
            logDebug("benchmark: updated %d%% of database", pct);
            nextLog += ROWS / 4;
        }
    }
    logDebug("benchmark: update complete");

    db.deinit();
    File::remove(p);
}
