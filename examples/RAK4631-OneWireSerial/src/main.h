/**
 * @file main.h
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief Defines and includes
 * @version 0.1
 * @date 2024-06-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <Arduino.h>
#include <Adafruit_TinyUSB.h> // for Serial

#include <RAK-OneWireSerial.h>

// Debug output set to 0 to disable app debug output
#ifndef MY_DEBUG
#define MY_DEBUG 1
#endif

#if MY_DEBUG > 0
#define MYLOG(tag, ...)           \
	do                            \
	{                             \
		if (tag)                  \
			PRINTF("[%s] ", tag); \
		PRINTF(__VA_ARGS__);      \
		PRINTF("\n");             \
	} while (0)
#else
#define MYLOG(...)
#endif

static void onewire_evt(const uint8_t pid, const uint8_t sid, const SNHUBAPI_EVT_E eid, uint8_t *msg, uint16_t len);
bool parse_payload(uint8_t *data, uint16_t data_len);
#define LPP_CHANNEL_BATT_PERCENT 100 // Base Board
