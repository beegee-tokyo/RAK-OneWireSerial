#include <Arduino.h>
#include <Adafruit_TinyUSB.h> // for Serial
#include <RAK-OneWireSerial.h>


SoftwareHalfSerial mySerial(15); //Wire pin  P0.15


void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while ( !Serial ) delay(10);   // for nrf52840 with native usb

  Serial.println("Goodnight moon!");
 
  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
  mySerial.println("Hello, world?");
}

void loop() // run over and over//
{
  if (mySerial.available()) 
    Serial.write(mySerial.read());

  if (Serial.available())
    mySerial.write(Serial.read());
}
