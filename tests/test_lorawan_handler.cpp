#include "gtest/gtest.h"

extern "C"
{
    #include "fakes.h"
    #include <taskConfig.h>
    #include "Controls/temperature.h"
    #include "Controls/humidity.h"
    #include "Controls/co2.h"
    #include "Controls/error.h"
    #include "Controls/actuation.h"
    #include "Controls/LoRaWANHandler.h"
}

class Test_lorawan_handler : public ::testing::Test
{
protected:
    void SetUp() override
    {
        RESET_FAKE(lora_driver_initialise);
        RESET_FAKE(status_leds_slowBlink);
        RESET_FAKE(lora_driver_mapReturnCodeToText);
        RESET_FAKE(lora_driver_rn2483FactoryReset);
        RESET_FAKE(lora_driver_configureToEu868);
        RESET_FAKE(lora_driver_getRn2483Hweui);
        RESET_FAKE(lora_driver_setOtaaIdentity);
        RESET_FAKE(lora_driver_saveMac);
        RESET_FAKE(lora_driver_setAdaptiveDataRate);
        RESET_FAKE(lora_driver_setReceiveDelay);
        RESET_FAKE(lora_driver_join);
        RESET_FAKE(lora_driver_resetRn2483);
        RESET_FAKE(lora_driver_flushBuffers);
        RESET_FAKE(lora_driver_sendUploadMessage);
        RESET_FAKE(status_leds_slowBlink);
        RESET_FAKE(status_leds_longPuls);
        RESET_FAKE(status_leds_ledOn);
        RESET_FAKE(status_leds_ledOff);
        RESET_FAKE(status_leds_fastBlink);
        RESET_FAKE(status_leds_shortPuls);
        RESET_FAKE(xMessageBufferReceive);
        RESET_FAKE(xMessageBufferCreate);
        RESET_FAKE(xTaskDelayUntil);
        RESET_FAKE(vTaskDelay);
        RESET_FAKE(xTaskCreate);
        RESET_FAKE(xTaskGetTickCount);
        RESET_FAKE(xSemaphoreCreateMutex);
        RESET_FAKE(xSemaphoreTake);
        RESET_FAKE(xSemaphoreGive);
        FFF_RESET_HISTORY();
    }

    void TearDown() override
    {
    }

    temperature_st makeTemp() {
        temperature_st _temp = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0,
            32,
            0,
            0,
            0,
            0,
            0,
            0,
            0
        };

        return _temp;
    }

    humidity_st makeHum() {
        humidity_st _hum = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0,
            43,
            0,
            0,
            0,
            0,
            0,
            0,
            0
        };

        return _hum;
    }

    co2_st makeCo2() {
        co2_st _co2 = {
            0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
            0,
            547,
            0,
            0,
            0
        };

        return _co2;
    }

    error_handler makeError() {
        error_handler _error = {
            0,
            0,
            2,
            0,
            0
        };

        return _error;
    }

    actuation_handler makeActuation(temperature_t temp, humidity_t hum) {
        actuation_handler _actuation = {
            temp,
            hum,
            0,
            0,
            VENT_OFF,
            AIRCON_OFF,
            ACTUATORS_OFF
        };

        return _actuation;
    }

    void copyArrayMembers(uint8_t dest[], uint8_t src[], int size)
    { 
        for(int i = 0; i < size; i++) {
            dest[i] = src[i];
        }
    }
};

TEST_F(Test_lorawan_handler, lorawan_handler_create)
{   char mBuf = 'y';
    void* mBufP= &mBuf;
    xMessageBufferCreate_fake.return_val =  mBufP;
    
    TickType_t last_messure_circle_time = 0;
    lorawan_handler_t result_lorawan_handler = lorawan_handler_create(last_messure_circle_time);
    
    EXPECT_EQ(1, lora_driver_initialise_fake.call_count);
    EXPECT_EQ(1, lora_driver_initialise_fake.arg0_val);
    ASSERT_TRUE(lora_driver_initialise_fake.arg1_val != NULL);

    EXPECT_EQ(1, xMessageBufferCreate_fake.call_count);
    EXPECT_EQ(sizeof(lora_driver_payload_t) * 2, xMessageBufferCreate_fake.arg0_val);

    EXPECT_EQ(2, xSemaphoreCreateMutex_fake.call_count);
    EXPECT_EQ(2, xSemaphoreTake_fake.call_count);

    EXPECT_EQ(1, xTaskCreate_fake.call_count);
    ASSERT_TRUE(xTaskCreate_fake.arg0_val != NULL);
    ASSERT_TRUE(xTaskCreate_fake.arg1_val != NULL);
    EXPECT_EQ(362, xTaskCreate_fake.arg2_val);
    EXPECT_EQ(result_lorawan_handler, xTaskCreate_fake.arg3_val);
    EXPECT_EQ(2, xTaskCreate_fake.arg4_val);
    ASSERT_TRUE(xTaskCreate_fake.arg5_val != NULL);
}

TEST_F(Test_lorawan_handler, lora_uplink_task) {
    char mBuf = 'y';
    void* mBufP= &mBuf;
    xMessageBufferCreate_fake.return_val =  mBufP;
    TickType_t last_messure_circle_time = 0;
    lora_driver_rn2483FactoryReset_fake.return_val = LORA_OK;
    lora_driver_getRn2483Hweui_fake.return_val = LORA_OK;
    lora_driver_join_fake.return_val = LORA_ACCEPTED;
    xSemaphoreTake_fake.return_val = pdTRUE;
    lorawan_handler_t lorawan_handler = lorawan_handler_create(last_messure_circle_time);

    lora_uplink_task_(1, lorawan_handler);

    EXPECT_EQ(2, lora_driver_resetRn2483_fake.call_count);
    EXPECT_EQ(1, lora_driver_resetRn2483_fake.arg0_history[0]);
    EXPECT_EQ(0, lora_driver_resetRn2483_fake.arg0_history[1]);
    EXPECT_EQ(1, lora_driver_flushBuffers_fake.call_count);
    
    EXPECT_EQ(1, status_leds_slowBlink_fake.call_count);
    EXPECT_EQ(led_ST2, status_leds_slowBlink_fake.arg0_val);
    EXPECT_EQ(1, lora_driver_rn2483FactoryReset_fake.call_count);
    EXPECT_EQ(1, lora_driver_configureToEu868_fake.call_count);
    EXPECT_EQ(1, lora_driver_getRn2483Hweui_fake.call_count);
    EXPECT_EQ(1, lora_driver_setDeviceIdentifier_fake.call_count);
    EXPECT_EQ(1, lora_driver_setOtaaIdentity_fake.call_count);
    EXPECT_EQ(1, lora_driver_saveMac_fake.call_count);
    EXPECT_EQ(1, lora_driver_setReceiveDelay_fake.call_count);
    EXPECT_EQ(1, lora_driver_join_fake.call_count);
    EXPECT_EQ(LORA_OTAA, lora_driver_join_fake.arg0_val);
    EXPECT_EQ(1, lora_driver_setAdaptiveDataRate_fake.call_count);
    EXPECT_EQ(1, lora_driver_setAdaptiveDataRate_fake.call_count);
}

TEST_F(Test_lorawan_handler, lora_upload_uplink_no_downlink) {
    char mBuf = 'y';
    void* mBufP= &mBuf;
    xMessageBufferCreate_fake.return_val =  mBufP;
    lora_driver_join_fake.return_val = LORA_ACCEPTED;
    lorawan_handler_t lorawan_handler = lorawan_handler_create(0);
    lora_driver_payload_t _uplink_payload, _downlink_payload;
    _uplink_payload.len = 10;
    _uplink_payload.portNo = 1;
    _downlink_payload.portNo = 1;
    lora_driver_sendUploadMessage_fake.return_val = LORA_MAC_TX_OK;
    xSemaphoreTake_fake.return_val = pdTRUE;
    uint8_t bytes[] = {2, 0, 32, 43, 2, 35, 0, 0, 0, 0};
    lorawan_handler_set_uplink(lorawan_handler, bytes);
    
    lora_uplink_task_(1, lorawan_handler);

    EXPECT_EQ(3, xSemaphoreTake_fake.call_count);
    EXPECT_EQ(1, lora_driver_sendUploadMessage_fake.call_count);
    EXPECT_EQ(0, xMessageBufferReceive_fake.call_count);
    EXPECT_EQ(bytes[0], ((lora_driver_payload_t *) lora_driver_sendUploadMessage_fake.arg1_val)->bytes[0]);
    EXPECT_EQ(bytes[1], ((lora_driver_payload_t *) lora_driver_sendUploadMessage_fake.arg1_val)->bytes[1]);
    EXPECT_EQ(bytes[2], ((lora_driver_payload_t *) lora_driver_sendUploadMessage_fake.arg1_val)->bytes[2]);
    EXPECT_EQ(bytes[3], ((lora_driver_payload_t *) lora_driver_sendUploadMessage_fake.arg1_val)->bytes[3]);
    EXPECT_EQ(bytes[4], ((lora_driver_payload_t *) lora_driver_sendUploadMessage_fake.arg1_val)->bytes[4]);
    EXPECT_EQ(bytes[5], ((lora_driver_payload_t *) lora_driver_sendUploadMessage_fake.arg1_val)->bytes[5]);
    EXPECT_EQ(bytes[6], ((lora_driver_payload_t *) lora_driver_sendUploadMessage_fake.arg1_val)->bytes[6]);
    EXPECT_EQ(bytes[7], ((lora_driver_payload_t *) lora_driver_sendUploadMessage_fake.arg1_val)->bytes[7]);
    EXPECT_EQ(bytes[8], ((lora_driver_payload_t *) lora_driver_sendUploadMessage_fake.arg1_val)->bytes[8]);
    EXPECT_EQ(bytes[9], ((lora_driver_payload_t *) lora_driver_sendUploadMessage_fake.arg1_val)->bytes[9]);
}

size_t mock_message_buffer_receive(MessageBufferHandle_t handle, void* destination, size_t size, TickType_t time)
{ 
    memcpy(destination, handle, size);
    lora_driver_payload_t* hp = (lora_driver_payload_t*) handle;
    lora_driver_payload_t* dp = (lora_driver_payload_t*) destination;
    printf("Handle: %p\n", handle);
    return size;
}

TEST_F(Test_lorawan_handler, lora_upload_uplink_receive_downlink) {
    lora_driver_join_fake.return_val = LORA_ACCEPTED;
    lora_driver_payload_t  _expected_downlink_payload;
    _expected_downlink_payload.len = 7;
    _expected_downlink_payload.portNo = 2;
    uint8_t bytes[7] = {128, 32, 15, 64, 21, 2, 211};
    copyArrayMembers(_expected_downlink_payload.bytes, bytes, 7);
    void* _expected_dowlink_void = &_expected_downlink_payload;
    printf("In test Handle: %p\n", _expected_dowlink_void);
    xMessageBufferCreate_fake.return_val =  _expected_dowlink_void;
    xMessageBufferReceive_fake.custom_fake = mock_message_buffer_receive;
    lorawan_handler_t lorawan_handler = lorawan_handler_create(0);
    lora_driver_payload_t _uplink_payload;
    _uplink_payload.len = 10;
    _uplink_payload.portNo = 1;
    lora_driver_sendUploadMessage_fake.return_val = LORA_MAC_RX;
    xSemaphoreTake_fake.return_val = pdTRUE;
    uint8_t up_bytes[] = {2, 0, 32, 43, 2, 35, 0, 0, 0, 0};
    lorawan_handler_set_uplink(lorawan_handler, up_bytes);
    lora_uplink_task_(1, lorawan_handler);

    uint8_t result_bytes[7];
    lorawan_handler_get_downlink(lorawan_handler, result_bytes);

    EXPECT_EQ(2, xSemaphoreGive_fake.call_count);
    EXPECT_EQ(1, xMessageBufferReceive_fake.call_count);
    EXPECT_EQ(_expected_dowlink_void, xMessageBufferReceive_fake.arg0_val);
    EXPECT_EQ(128, result_bytes[0]);
    EXPECT_EQ(32, result_bytes[1]);
    EXPECT_EQ(15, result_bytes[2]);
    EXPECT_EQ(64, result_bytes[3]);
    EXPECT_EQ(21, result_bytes[4]);
    EXPECT_EQ(2, result_bytes[5]);
    EXPECT_EQ(211, result_bytes[6]);
}

BaseType_t mock_xTaskCreate(TaskFunction_t task, const char *const name, const uint16_t stack_size, void *const param, UBaseType_t priority, TaskHandle_t *const task_handle)
{ 
    char ch = 'y';
    void* pointer = &ch;
    memcpy(task_handle, pointer, 3);
    return 1;
}

TEST_F(Test_lorawan_handler, lorawan_handler_destroy)
{
    xTaskCreate_fake.custom_fake = mock_xTaskCreate;
    TickType_t last_messure_circle_time = 0;
    lorawan_handler_t lorawan_handler = lorawan_handler_create(last_messure_circle_time);
    
    lorawan_handler_destroy(lorawan_handler);

    EXPECT_EQ(1, vTaskDelete_fake.call_count);
}