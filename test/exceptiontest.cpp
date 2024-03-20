#include <cerberus/exception/exception.h>
#include <cerberus/exception/exceptioncatalog.h>
#include <gtest/gtest.h>

TEST(exceptionTest, illegalArgument)
{
    EXPECT_THROW(throw cerberusIllegalArgExc("OOPS"), cerberus::Exception);
}
