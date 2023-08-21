#include <cerberus/cerberus.h>
#include <cerberus/data/sharedbytebuffer.h>
#include <gtest/gtest.h>

TEST(SharedByteBufferTest, equalityOperator)
{
    // test the equality operator
    cerberus::data::SharedByteBuffer b1("Hello, World!");
    cerberus::data::SharedByteBuffer b2("Hello, World!");

    ASSERT_TRUE(b1 == b2);
    ASSERT_FALSE(b1 != b2);

    b2 = "HELLO";

    ASSERT_TRUE(b1 != b2);
    ASSERT_FALSE(b1 == b2);
}

TEST(SharedByteBufferTest, COW)
{
    // test the main feature of the ByteBuffer class

    cerberus::data::SharedByteBuffer b1("Hello, World!");
    EXPECT_EQ(b1.instances(), 1);

    cerberus::data::SharedByteBuffer b2(b1);  // copy construct, the buffer is the same

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
    cerberus::data::SharedByteBuffer b1("Hello, World!");

    auto b2 = b1.subBuffer(7, 5);

    EXPECT_TRUE(b2 == "World");
    EXPECT_EQ(b2.size(), 5);
    EXPECT_EQ(b1.size(), 13);
}
