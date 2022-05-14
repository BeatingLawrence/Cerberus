#include <gtest/gtest.h>
#include <cerberus/cerberus.h>
#include <iostream>

int main(int argc, char* argv[])
{
    cerberus::Cerberus::provider()->cerberus::Cerberus::init(cerberus::Cerberus::cerberusDefaultParms());
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
