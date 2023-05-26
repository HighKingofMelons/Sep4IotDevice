#pragma once
#include <stdbool.h>
#include <ATMEGA_FreeRTOS.h>
#include <message_buffer.h>

#include <serial.h>

#define LORA_MAX_PAYLOAD_LENGTH	20 /* bytes - Must newer be changed!!!*/

typedef struct lora_driver_payload {
	uint8_t portNo; /**< Port_no the data is received on, or to transmit to [1..223]*/
	uint8_t len; /**< Length of the payload (no of bytes) - MAX 20 bytes is allowed in this implementation! */
	uint8_t bytes[LORA_MAX_PAYLOAD_LENGTH]; /**< Array to hold the payload to be send, or that has been received */
} lora_driver_payload_t;

typedef enum Lora_driver_returnCodes {
	LORA_OK	/**< Everything went well */
	, LORA_ERROR /**< An error occurred - the reason is not explained any further  */
	, LORA_KEYS_NOT_INIT /**< The necessary keys are not initialized */
	, LORA_NO_FREE_CH	/**< All channels are buzy */
	, LORA_SILENT /**< The module is in a Silent Immediately state */
	, LORA_BUSY /**< The MAC state of the module is not in an idle state */
	, LORA_MAC_PAUSED /**< The MAC is in PAUSED state and needs to be resumed back*/
	, LORA_DENIED /**< The join procedure was unsuccessful (the module attempted to join the
	network, but was rejected) */
	, LORA_ACCEPTED  /**< The join procedure was successful */
	, LORA_INVALID_PARAM  /**< One of the parameters given is wrong */
	, LORA_NOT_JOINED /**< The network is not joined */
	, LORA_INVALID_DATA_LEN /**< If application payload length is greater than the maximum
	application payload length corresponding to the current data rate */
	, LORA_FRAME_COUNTER_ERR_REJOIN_NEEDED /**< If the frame counter rolled over - a rejoin is needed */
	, LORA_MAC_TX_OK /**< If up link transmission was successful and no down link data was
	received back from the server */
	, LORA_MAC_RX /**< If there is a downlink message is received on an uplink transmission */
	, LORA_MAC_ERROR /**< If transmission was unsuccessful, ACK not received back from the
	server */
	, LORA_UNKNOWN /**< An unknown error occurred that is not identified by this driver */
} lora_driver_returnCode_t;

typedef enum lora_driver_joinModes {
	LORA_OTAA = 0  /**< Join the LoRaWAN network with Over The Air Activation (OTAA) */
	,LORA_ABP /**< Join the LoRaWAN network Activation By Personalization (ABP) */
} lora_driver_joinMode_t;

typedef enum lora_driver_adaptiveDataRateModes {
	LORA_OFF = 0 /**< Set ADR to ON */
	,LORA_ON /**< Set ADR to OFF */
} lora_driver_adaptiveDataRate_t;

typedef enum lora_driver_automaticReplyModes {
	LORA_AR_ON /**< Set AR to ON */
	,LORA_AR_OFF  /**< Set AR to OFF */
} lora_driver_automaticReplyMode_t;

void lora_driver_initialise(serial_comPort_t comPort, MessageBufferHandle_t downlinkMessageBuffer);

lora_driver_returnCode_t lora_driver_setOtaaIdentity(char appEUI[17], char appKEY[33], char devEUI[17]);

lora_driver_returnCode_t lora_driver_configureToEu868(void);

lora_driver_returnCode_t lora_driver_join(lora_driver_joinMode_t mode);

lora_driver_returnCode_t lora_driver_sendUploadMessage(bool confirmed, lora_driver_payload_t * payload);

lora_driver_returnCode_t lora_driver_setDeviceIdentifier(const char devEUI[17]);

lora_driver_returnCode_t lora_driver_setAdaptiveDataRate(lora_driver_adaptiveDataRate_t state);

//lora_driver_returnCode_t lora_driver_getAdaptiveDataRate(lora_driver_adaptiveDataRate_t * state);

lora_driver_returnCode_t lora_driver_setReceiveDelay(uint16_t rxDelay1);

//lora_driver_returnCode_t lora_driver_setAutomaticReply(lora_driver_automaticReplyMode_t ar);

//lora_driver_returnCode_t lora_driver_getAutomaticReply(lora_driver_automaticReplyMode_t * ar);

lora_driver_returnCode_t lora_driver_setLinkCheckInterval(uint16_t sec); // [0..65535]

void lora_driver_resetRn2483(uint8_t state);

void lora_driver_flushBuffers(void);

lora_driver_returnCode_t lora_driver_getRn2483Hweui(char hwDevEUI[17]);

lora_driver_returnCode_t lora_driver_rn2483FactoryReset(void);
