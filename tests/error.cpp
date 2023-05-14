#include <gtest/gtest.h>

extern "C" {
    #include <display_7seg.h>
    #include <task.h>
    #include <queue.h>
    #include <error.h>

    #include <fakes.h>
}

TEST(Error, QueueError) {
    error_handler_init();

    ASSERT_EQ(fff.call_history[0], (void *)display_7seg_initialise);
    ASSERT_EQ(fff.call_history[1], (void *)xQueueCreate);
    ASSERT_EQ(fff.call_history[2], (void *)xTaskCreate);

    FFF_RESET_HISTORY();
};