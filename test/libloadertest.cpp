#include <cerberus/cerberus.h>
#include <cerberus/core/libloader.h>
#include <gtest/gtest.h>

using namespace cerberus::core;

TEST(libLoaderTest, standalone_libloading)
{
    GTEST_SKIP();
    LibLoader loader;

    ASSERT_TRUE(loader.load("libtestobject.dylib", true).ok());

    int (*func)(int, int) = (int (*)(int, int))loader.get("add").func;

    ASSERT_NE(func, nullptr);

    int a = 20;
    int b = 34;

    ASSERT_EQ(func(20, 34), (20 + 34));

    func = (int (*)(int, int))loader.get("multiply").func;

    ASSERT_NE(func, nullptr);

    ASSERT_EQ(func(20, 34), (20 * 34));

    loader.unload();
}

TEST(libLoaderTest, shared_libloading)
{
    GTEST_SKIP();
    LibLoader loader;

    ASSERT_TRUE(loader.load("libtestobject.dylib").ok());

    ASSERT_TRUE(loader.isLoaded());

    {
        auto f = loader.get("add");

        ASSERT_TRUE(f.isValid());

        int (*func)(int, int) = (int (*)(int, int))f.func;

        int a = 20;
        int b = 34;

        ASSERT_EQ(func(20, 34), (20 + 34));
    }

    {
        auto f = loader.get("multiply");

        ASSERT_TRUE(f.isValid());

        int (*func)(int, int) = (int (*)(int, int))f.func;

        int a = 20;
        int b = 34;

        ASSERT_EQ(func(20, 34), (20 * 34));
    }
}
