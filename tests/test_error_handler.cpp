#include <gtest/gtest.h>

extern "C" {
    #include "Controls/error_handler.h"
    #include "fakes.h"
}

#define INIT_FAKES(FAKE)            \
    FAKE(display_7seg_initialise)   \
    FAKE(xQueueCreate)              \
    FAKE(xSemaphoreCreateMutex)     \
    FAKE(xSemaphoreGive)            \
    FAKE(xTaskCreate)

#define DESTROY_FAKES(FAKE)         \
    FAKE(vTaskDelete)               \
    FAKE(display_7seg_powerDown)    \
    FAKE(vQueueDelete)              \
    FAKE(vSemaphoreDelete)             

TEST(error_handler, init) {
    FFF_RESET_HISTORY();
    error_handler_t _handler = error_handler_init();
    ASSERT_EQ(fff.call_history[0], (void*)display_7seg_initialise);
    ASSERT_EQ(fff.call_history[1], (void*)xQueueCreate);
    ASSERT_EQ(fff.call_history[2], (void*)xSemaphoreCreateMutex);
    ASSERT_EQ(fff.call_history[3], (void*)xSemaphoreGive);
    ASSERT_EQ(fff.call_history[4], (void*)xTaskCreate);
    DESTROY_FAKES(RESET_FAKE);
}

TEST(error_handler, destroy) {
    error_handler_t _handler = error_handler_init();
    INIT_FAKES(RESET_FAKE);
    FFF_RESET_HISTORY();

    error_handler_destroy(_handler);

    ASSERT_EQ(fff.call_history[0], (void*)vTaskDelete);
    ASSERT_EQ(fff.call_history[1], (void*)vQueueDelete);
    ASSERT_EQ(fff.call_history[2], (void*)vSemaphoreDelete);
    DESTROY_FAKES(RESET_FAKE);

    _handler = error_handler_init();
    _handler->display_on = pdTRUE;
    INIT_FAKES(RESET_FAKE);
    FFF_RESET_HISTORY();

    error_handler_destroy(_handler);

    ASSERT_EQ(fff.call_history[0], (void*)vTaskDelete);
    ASSERT_EQ(fff.call_history[1], (void*)display_7seg_powerDown);
    ASSERT_EQ(fff.call_history[2], (void*)vQueueDelete);
    ASSERT_EQ(fff.call_history[3], (void*)vSemaphoreDelete);
    DESTROY_FAKES(RESET_FAKE);
}

TEST(error_handler, report) {
    error_handler_t _handler = error_handler_init();
    INIT_FAKES(RESET_FAKE);

    BaseType_t seq[] = {pdTRUE, errQUEUE_FULL};
    SET_RETURN_SEQ(xQueueSend, seq, 2);

    ASSERT_EQ(error_handler_report(_handler, ERROR_TEMP), pdTRUE);
    ASSERT_EQ(error_handler_report(_handler, ERROR_TEMP), errQUEUE_FULL);

    error_handler_destroy(_handler);
    RESET_FAKE(xQueueSend);
}

TEST(error_handler, revoke) {
    error_handler_t _handler = error_handler_init();
    INIT_FAKES(RESET_FAKE);

    BaseType_t seq[] = {pdTRUE, errQUEUE_FULL};
    SET_RETURN_SEQ(xQueueSend, seq, 2);

    ASSERT_EQ(error_handler_revoke(_handler, ERROR_TEMP), pdTRUE);
    ASSERT_EQ(error_handler_revoke(_handler, ERROR_TEMP), errQUEUE_FULL);

    error_handler_destroy(_handler);
    RESET_FAKE(xQueueSend);
}

struct error_item testitem;
BaseType_t return_item;

BaseType_t xQueueRecive_custom_func(QueueHandle_t handle, void *item, TickType_t ticks) {
    struct error_item *_item = (struct error_item*) item;
    *_item = testitem;

    return return_item;
}

TEST(error_handler, update_flags) {
    INIT_FAKES(RESET_FAKE);
    FFF_RESET_HISTORY();

    struct error_handler _handler = {
        xQueueCreate(10, sizeof(struct error_item)),
        xSemaphoreCreateMutex(),
        0,
        pdFALSE,
        0
    };

    BaseType_t xSemaphoreTake_seq[] = {pdFALSE, pdTRUE, pdTRUE};
    SET_RETURN_SEQ(xSemaphoreTake, xSemaphoreTake_seq, 3);

    xQueueReceive_fake.custom_fake = xQueueRecive_custom_func;

    testitem = {
        ERROR_CO2,
        ERROR_ENABLE
    };

    return_item = pdFALSE;
    update_flags(&_handler);
    ASSERT_EQ(_handler.flags, (error_flags_t) 0);

    return_item = pdTRUE;
    update_flags(&_handler);
    ASSERT_EQ(_handler.flags, (error_flags_t) ERROR_CO2);

    update_flags(&_handler);
    ASSERT_EQ(_handler.flags, (error_flags_t) ERROR_CO2);

    testitem = {
        ERROR_HUMI,
        ERROR_ENABLE
    };

    update_flags(&_handler);
    ASSERT_EQ(_handler.flags, (error_flags_t) ERROR_CO2 + ERROR_HUMI);

    testitem = {
        ERROR_CO2,
        ERROR_DISABLE
    };

    update_flags(&_handler);
    ASSERT_EQ(_handler.flags, (error_flags_t) ERROR_HUMI);

    update_flags(&_handler);
    ASSERT_EQ(_handler.flags, (error_flags_t) ERROR_HUMI);

    RESET_FAKE(xQueueReceive);
    RESET_FAKE(xSemaphoreGive);
    RESET_FAKE(xSemaphoreTake);
}

TEST(error_handler, get_flags) {
    INIT_FAKES(RESET_FAKE);
    FFF_RESET_HISTORY();

    struct error_handler _handler = {
        xQueueCreate(10, sizeof(struct error_item)),
        xSemaphoreCreateMutex(),
        ERROR_CO2,
        pdFALSE,
        0
    };
    
    RESET_FAKE(xQueueReceive);
    RESET_FAKE(xSemaphoreCreateMutex);

    BaseType_t xSemaphoreTake_seq[] = {pdFALSE, pdTRUE};
    SET_RETURN_SEQ(xSemaphoreTake, xSemaphoreTake_seq, 2);

    error_flags_t flags = error_handler_get_flags(&_handler);
    ASSERT_EQ(flags, ERROR_CO2);
    ASSERT_EQ(xSemaphoreTake_fake.call_count, 2);
    ASSERT_EQ(xSemaphoreGive_fake.call_count, 1);

    RESET_FAKE(xSemaphoreGive);
    RESET_FAKE(xSemaphoreTake);
}

TEST(error_handler, update_display) {
    INIT_FAKES(RESET_FAKE);
    FFF_RESET_HISTORY();

    struct error_handler _handler = {
        xQueueCreate(10, sizeof(struct error_item)),
        xSemaphoreCreateMutex(),
        0,
        pdFALSE,
        0
    };
    
    RESET_FAKE(xQueueReceive);
    RESET_FAKE(xSemaphoreCreateMutex);

    xSemaphoreTake_fake.return_val = pdFALSE;

    update_display(&_handler);
    ASSERT_EQ(xSemaphoreTake_fake.call_count, 1);
    ASSERT_EQ(xSemaphoreGive_fake.call_count, 0);

    xSemaphoreTake_fake.return_val = pdTRUE;

    update_display(&_handler);
    ASSERT_EQ(_handler.current_display, 0);
    ASSERT_EQ(_handler.display_on, pdFALSE);

    _handler.display_on = pdTRUE;

    update_display(&_handler);
    ASSERT_EQ(_handler.current_display, 0);
    ASSERT_EQ(_handler.display_on, pdFALSE);

    _handler.flags = 0b00000001;

    update_display(&_handler);
    ASSERT_EQ(_handler.current_display, 1);
    ASSERT_EQ(_handler.display_on, pdTRUE);
    ASSERT_EQ(display_7seg_display_fake.arg0_val, 1);

    update_display(&_handler);
    ASSERT_EQ(_handler.current_display, 1);
    ASSERT_EQ(_handler.display_on, pdTRUE);
    ASSERT_EQ(display_7seg_display_fake.arg0_val, 1);

    _handler.flags = 0b00001001;

    update_display(&_handler);
    ASSERT_EQ(_handler.current_display, 4);
    ASSERT_EQ(_handler.display_on, pdTRUE);
    ASSERT_EQ(display_7seg_display_fake.arg0_val, 8);

    RESET_FAKE(xSemaphoreGive);
    RESET_FAKE(xSemaphoreTake);
}