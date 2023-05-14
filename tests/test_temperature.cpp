#include "gtest/gtest.h"

extern "C" {
    #include "fakes.h"
}

class Test_production : public ::testing::Test
{
    protected:
        void SetUp() override
        {

        }

        void TearDown() override 
        {

        }
};

TEST_F(Test_production, temperature_create)
{
    EXPECT_EQ(1, 1);
}

