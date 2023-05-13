#include "gtest/gtest.h"
#include "fff.h"
DEFINE_FFF_GLOBALS;

extern "C" {
    #include <temperature.h>
    #include <ATMEGA_FreeRTOS.h>
    #include <semphr.h>
    #include <message_buffer.h>
    #include <task.h>
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

