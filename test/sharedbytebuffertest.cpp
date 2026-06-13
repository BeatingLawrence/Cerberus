#include <cerberus.h>
#include <data/bytebuffer.h>
#include <data/sharedbytebuffer.h>
#include <gtest/gtest.h>

using namespace crb;

TEST(SharedByteBufferTest, equalityOperator)
{
    // test the equality operator
    SharedByteBuffer b1("Hello, World!");
    SharedByteBuffer b2("Hello, World!");

    ASSERT_TRUE(b1 == b2);
    ASSERT_FALSE(b1 != b2);

    b2 = "HELLO";

    ASSERT_TRUE(b1 != b2);
    ASSERT_FALSE(b1 == b2);
}

TEST(SharedByteBufferTest, COW)
{
    // test the main feature of the ByteBuffer class

    SharedByteBuffer b1("Hello, World!");
    EXPECT_EQ(b1.instances(), 1);

    SharedByteBuffer b2(b1);  // copy construct, the buffer is the same

    EXPECT_EQ(b1.instances(), 2);
    EXPECT_EQ(b2.instances(), 2);

    EXPECT_TRUE(b1 == b2);

    b1.appendString(" HEHE");

    EXPECT_EQ(b1.instances(), 2);
    EXPECT_EQ(b2.instances(), 2);

    EXPECT_TRUE(b2 == "Hello, World! HEHE");

    b2.appendString("...");  // b2 cannot be the owner, so it deeps copy the buffer

    EXPECT_EQ(b1.instances(), 1);
    EXPECT_EQ(b2.instances(), 1);

    EXPECT_TRUE(b2 == "Hello, World! HEHE...");
}

TEST(SharedByteBufferTest, SubBuffer)
{
    SharedByteBuffer b1("Hello, World!");

    auto b2 = b1.subBuffer(7, 5);

    //
    EXPECT_TRUE(b2 == "World");
    EXPECT_EQ(b2.size(), static_cast<crb::LSIZE>(5));
    EXPECT_EQ(b1.size(), static_cast<crb::LSIZE>(13));
    // EXPECT_EQ(b1.instances(), 1);
    // EXPECT_EQ(b2.instances(), 1);
    //
    char a[20] = {};
    b1.copyTo((crb::BYTE*)&a[0]);

    char b[20] = {};
    b2.copyTo((crb::BYTE*)&b[0]);

    logInfo(CerberusUtils::strPrint("b1: %llu, b2: %llu", static_cast<unsigned long long>(b1.size()),
                                     static_cast<unsigned long long>(b2.size())));
    logInfo(b);
    logInfo(a);

    ByteBuffer bb("hello hello hello");

    char c[20] = {};
    bb.copyTo((crb::BYTE*)&c[0]);
    logInfo(CerberusUtils::strPrint("bb: %llu", static_cast<unsigned long long>(bb.size())));
    logInfo(c);

    auto bbb = bb.subBuffer(0, 5);

    char d[20] = {};
    bbb.copyTo((crb::BYTE*)&d[0]);
    logInfo(CerberusUtils::strPrint("bbb: %llu", static_cast<unsigned long long>(bbb.size())));
    logInfo(d);
}
