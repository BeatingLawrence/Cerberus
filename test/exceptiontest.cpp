#include <gtest/gtest.h>

#include <cerberus/exception/exceptioncatalog.h>
#include <cerberus/exception/exception.h>

TEST(exceptionTest, illegalArgument)
{
    EXPECT_THROW(throw cerberusIllegalArgExc("OOPS"), cerberus::exception::Exception);
}
