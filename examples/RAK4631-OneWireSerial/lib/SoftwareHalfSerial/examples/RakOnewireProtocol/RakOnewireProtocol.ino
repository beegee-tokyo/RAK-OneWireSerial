#include <Arduino.h>
#include <Adafruit_TinyUSB.h> // for Serial

#include <SoftwareHalfSerial.h>

#include "onewire_master_api.h"
#include "onewire_master_protocol.h"


SoftwareHalfSerial mySerial(15); //Wire pin  P0.15

uint8_t buff[0x100];
uint16_t bufflen = 0;

static void onewire_evt ( const uint8_t pid, const uint8_t sid, const SNHUBAPI_EVT_E eid, uint8_t * msg, uint16_t len )
{
    switch (eid)
    {
        case SNHUBAPI_EVT_RECV_REQ:
        case SNHUBAPI_EVT_RECV_RSP:
            break;

        case SNHUBAPI_EVT_QSEND:
            mySerial.write(msg,len);
            break;

        case SNHUBAPI_EVT_ADD_SID:
            Serial.printf("+ADD:SID:[%02x]\r\n", msg[0]);
            break;

        case SNHUBAPI_EVT_ADD_PID:
            Serial.printf("+ADD:PID:[%02x]\r\n", msg[0]);
            break;
        
        case SNHUBAPI_EVT_GET_INTV:
            break;
            
        case SNHUBAPI_EVT_GET_ENABLE:
            break;
            
        case SNHUBAPI_EVT_SDATA_REQ:
        case SNHUBAPI_EVT_REPORT:
            Serial.printf("+EVT:PID[%02x],IPSO[%02x]\r\n",pid,msg[0]);
            for( uint16_t i=1; i<len; i++)
            {
                Serial.printf("%02x,", msg[i]);
            }
            Serial.println();
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

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(115200);
//  while ( !Serial ) delay(10);   // for nrf52840 with native usb

  Serial.println("Demo Rakwireless Onewire Protocol...");
 
  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
  RakSNHub_Protocl_API.init(onewire_evt);
  
}
  uint16_t count = 0;
  
void loop() // run over and over//
{

  while(mySerial.available())
  {
      char a = mySerial.read();
      buff[bufflen++]=a;
      delay(5); //continue data, timeout=5ms
  }

  if( bufflen != 0)
  {
      RakSNHub_Protocl_API.process((uint8_t *)buff,bufflen);
      bufflen = 0;
  }
}