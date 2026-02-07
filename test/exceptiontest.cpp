#include <exception/exception.h>
#include <exception/exceptioncatalog.h>
#include <gtest/gtest.h>

TEST(exceptionTest, illegalArgument) { EXPECT_THROW(throw cIllegalArgExc("OOPS"), crb::Exception); }
