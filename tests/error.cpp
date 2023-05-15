#include <gtest/gtest.h>

extern "C" {
    #include <display_7seg.h>
    #include <task.h>
    #include <queue.h>

    #include <error.h>
    #include <private/p_error.h>

    #include <fakes.h>
}

#define INIT_FAKES(FAKE)        \
    FAKE(display_7seg_display)  \
    FAKE(xQueueCreate)          \
    FAKE(xTaskCreate)

