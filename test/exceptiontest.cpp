#include <gtest/gtest.h>

#include <cerberus/exception/exceptioncatalog.h>
#include <cerberus/exception/exception.h>

TEST(exceptionTest, illegalArgument)
{
    EXPECT_THROW(throw cerberusIllegalArgumentExc("OOPS"), cerberus::exception::Exception);
}
