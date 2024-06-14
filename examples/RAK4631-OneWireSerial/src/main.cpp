/**
 * @file main.cpp
 * @author Bernd Giesecke (bernd@giesecke.tk)
 * @brief SensorHub OneWireSerial test
 * @version 0.1
 * @date 2024-06-06
 *
 * @copyright Copyright (c) 2024
 *
 */
#include "main.h"
SoftwareHalfSerial mySerial(PIN_SERIAL1_RX); // Wire pin  P0.15

uint8_t buff[0x100];
uint16_t bufflen = 0;

uint8_t found_pids[5] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t found_pid_num = 0;
volatile bool found_new_pid = false;

bool requested_data = false;

/**
 * @brief Initial setup of the application
 *
 */
void setup(void)
{
	Serial.begin(115200);
	time_t serial_timeout = millis();
	// On nRF52840 the USB serial is not available immediately
	while (!Serial)
	{
		if ((millis() - serial_timeout) < 5000)
		{
			delay(100);
			digitalWrite(LED_GREEN, !digitalRead(LED_GREEN));
		}
		else
		{
			break;
		}
	}
	digitalWrite(LED_GREEN, LOW);

	MYLOG("APP", "Setup application");

	pinMode(WB_IO2, OUTPUT);
	digitalWrite(WB_IO2, HIGH);

	mySerial.begin(9600);
	RakSNHub_Protocl_API.init(onewire_evt);

	MYLOG("APP", "Waiting for PID");
	// Check for devices for 30 seconds

	time_t start_search = millis();
	while ((millis() - start_search) < 15000)
	{
		while (mySerial.available())
		{
			char a = mySerial.read();
			buff[bufflen++] = a;
			delay(5); // continue data, timeout=5ms
		}

		if (bufflen != 0)
		{
			RakSNHub_Protocl_API.process((uint8_t *)buff, bufflen);
			bufflen = 0;
		}

		if (found_new_pid)
		{
			found_new_pid = false;
		}
		delay(500);
	}
	Serial.printf("Found %d\r\n", found_pid_num);
	digitalWrite(WB_IO2, LOW);
}

/**
 * @brief Arduino Loop
 */
void loop()
{
	delay(10000);
	digitalWrite(WB_IO2, HIGH);
	delay(1000);

	if (found_pid_num != 0)
	{
		for (uint8_t idx = 0; idx < found_pid_num; idx++)
		{
			MYLOG("APP", "Request sensor data from pid %d", found_pids[idx]);
			RakSNHub_Protocl_API.get.data(found_pids[idx]);
			requested_data = true;

			time_t start_wait = millis();
			while (requested_data)
			{
				while (mySerial.available())
				{
					char a = mySerial.read();
					buff[bufflen++] = a;
					delay(5); // continue data, timeout=5ms
				}

				if (bufflen != 0)
				{
					RakSNHub_Protocl_API.process((uint8_t *)buff, bufflen);
					bufflen = 0;
				}

				if ((millis() - start_wait) > 120000)
				{
					MYLOG("APP", "Timeout");
					requested_data = false;
				}
			}
			memset(buff, 0, 0x100);
		}
	}
	digitalWrite(WB_IO2, LOW);
}

static void onewire_evt(const uint8_t pid, const uint8_t sid, const SNHUBAPI_EVT_E eid, uint8_t *msg, uint16_t len)
{
	switch (eid)
	{
	case SNHUBAPI_EVT_RECV_REQ:
	case SNHUBAPI_EVT_RECV_RSP:
		break;

	case SNHUBAPI_EVT_QSEND:
		mySerial.write(msg, len);
		break;

	case SNHUBAPI_EVT_ADD_SID:
		Serial.printf("+ADD:SID:[%02x]\r\n", msg[0]);
		break;

	case SNHUBAPI_EVT_ADD_PID:
		Serial.printf("+ADD:PID:[%02x]\r\n", msg[0]);
		for (int idx = 0; idx < 5; idx++)
		{
			Serial.printf("Slot %d = %02X\r\n", idx, found_pids[idx]);
		}

		for (int idx = 0; idx < 5; idx++)
		{
			if (found_pids[idx] == msg[0])
			{
				Serial.println("PID already registered");
				return;
			}
		}
		for (int idx = 0; idx < 5; idx++)
		{
			if (found_pids[idx] == 0xFF)
			{
				found_pids[idx] = msg[0];
				found_pid_num++;
				Serial.printf("New PID %d registered at index %d\r\n", msg[0], idx);
				found_new_pid = true;
				return;
			}
		}
		Serial.println("No more slots for new sensors available");
		break;

	case SNHUBAPI_EVT_GET_INTV:
		break;

	case SNHUBAPI_EVT_GET_ENABLE:
		break;

	case SNHUBAPI_EVT_SDATA_REQ:
		Serial.printf("+EVT:SDATA_REQ PID[%02x],IPSO[%02x]\r\n", pid, msg[0]);
		Serial.printf("Reversed values (Sensor Node bug)\r\n");
		for (uint16_t i = 1; i < len; i++)
		{
			Serial.printf("%02x,", msg[i]);
		}
		Serial.printf("\r\nLen = %d\r\n", len);

		// Prepare buffer for parsing
		buff[0] = pid; // Sensor number
		// Fake TDS sensor
		if (msg[0] == 194)
		{
			msg[0] = 2;
		}
		buff[1] = msg[0]; // IPSO channel
		memcpy(&buff[2], &msg[1], len - 1);
		for (int idx = 0; idx < len; idx = idx + 2)
		{
			buff[idx + 3] = msg[idx + 2];
			buff[idx + 2] = msg[idx + 1];
		}
		for (uint16_t i = 0; i < len + 1; i++)
		{
			Serial.printf("%02x", buff[i]);
		}
		Serial.println();
		parse_payload(buff, len + 1);
		requested_data = false;
		break;

	case SNHUBAPI_EVT_REPORT:
		Serial.printf("+EVT:REPORT PID[%02x],IPSO[%02x]\r\n", pid, msg[0]);
		for (uint16_t i = 1; i < len; i++)
		{
			Serial.printf("%02x,", msg[i]);
		}
		Serial.printf("\r\nLen = %d\r\n", len);

		// Prepare buffer for parsing
		buff[0] = pid; // Sensor number
		// Fake TDS sensor
		if (msg[0] == 194)
		{
			msg[0] = 2;
		}
		buff[1] = msg[0]; // IPSO channel
		memcpy(&buff[2], &msg[1], len - 1);
		for (int idx = 0; idx < len; idx = idx + 2)
		{
			buff[idx + 2] = msg[idx + 2];
			buff[idx + 3] = msg[idx + 1];
		}
		for (uint16_t i = 0; i < len + 1; i++)
		{
			Serial.printf("%02x", buff[i]);
		}
		Serial.println();
		parse_payload(buff, len + 1);
		requested_data = false;
		break;

	case SNHUBAPI_EVT_CHKSUM_ERR:
		Serial.printf("+ERR:CHKSUM\r\n");
		break;

	case SNHUBAPI_EVT_SEQ_ERR:
		Serial.printf("+ERR:SEQUCE\r\n");
		break;

	default:
		break;
	}
}
