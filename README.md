| <img src="./assets/RAK-Whirls.png" alt="RAKWireless" width=300%> | <img src="./assets/WisBlock.png" alt="WisBlock"> | <img src="./assets/rakstar.jpg" alt="RAKstar"> |    
| :-: | :-: | :-: |     

# ⚠️ BETA! ⚠️
This library is still in testing phase!

# Library for RAK-OneWireSerial 

This library provides functions to communicate over a one wire serial interface with slave modules.     
This protocol is used by [RAK2560 SensorHub](https://docs.rakwireless.com/Product-Categories/WisNode/RAK2560/Overview) sensor probes and IO probes. It is as well used to communicate with the [RAK9154 Solar Battery Lite](https://docs.rakwireless.com/Product-Categories/Accessories/RAK9154/Overview/).    

⚠️ This library implements a software half serial communication. It is designed to work with the [WisBlock RAK4631 (Arduino)](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK4631/Overview/) only. ⚠️    

----

# Required hardware
The simpliest hardware configuration to connect to SensorHub Sensor Probes and IO Probes that do not require 12V supply (check the data sheets)    
- RAK19007 WisBlock Base Board
- RAK4631 WisBlock Core Module
- RAK13002 WisBlock IO module
- RAK19002 WisBlock Boost module (if 12V supply is required)

In this configuration, the Sensor Probe is connected with 3.3V, GND and RXD1 only from the RAK13002 module only.    
<img src="./assets/connection.png" alt="RAKWireless" width=300%>


For Sensor and IO probes that require a 12V supply voltage, a 12V supply is required, e.g. the [RAK19002 Boost Module](https://docs.rakwireless.com/Product-Categories/WisBlock/RAK19002/Overview/).    
<img src="./assets/connection-12v.png" alt="RAKWireless" width=300%>

----

# Examples

* **[RakOnewireProtocol](./examples/RakOnewireProtocol.in0)** - Using the library to communicate with a SensorHub Sensor or IO Probe    
* **[RAK4631-OneWireSerial](./examples/RAK4631-OneWireSerial)** - PlatformIO example with parsing of the received sensor data    
* **[SoftwareHalfSerialExample](./examples/SoftwareHalfSerialExample.ino)** - Example for the software half serial interface.     

----

# Installation

In Arduino IDE manual installation is required as the library is not submitted to the Library Manager.      
Download the library from [RAK13015](https://github.com/beegee-tokyo/RAK13015) as [ZIP file](https://github.com/beegee-tokyo/RAK13015/archive/refs/heads/main.zip). Unzip it and place the RUI3-Arduino-Library folder into the library directory.
In Arduino IDE this is usually <arduinosketchfolder>/libraries/

In PlatformIO install the library project dependencies by adding

```log
lib_deps =
  https://github.com/beegee-tokyo/RAK13015.git
```

into **`platformio.ini`**

----

# SERIAL assignments
The GPIO's and Serial assignments depend on the used WisBlock Base Board and the Sensor Slot used. Some Sensor Slots are not suitable, because they either have no Serial connection or there is a conflict with the used IO's.    

**RAK19007**    
Slot A ==> NG (WB_IO2 not available)    
Slot B ==> NG (WB_IO2 not available)    
Slot C ==> NG    
Slot D ==> Serial 1 ALERT WB_IO6 TCON WB_IO5    

**RAK19003**    
Slot C ==> Serial 1 ALERT WB_IO4 TCON WB_IO3    
Slot D ==> Serial 1 ALERT WB_IO6 TCON WB_IO5     

**RAK19001**    
Slot A ==> NG (WB_IO2 not available)     
Slot B ==> NG (WB_IO2 not available)     
Slot C ==> NG    
Slot D ==> Serial 2 ALERT WB_IO6 TCON WB_IO5     
Slot E ==> Serial 2 ALERT WB_IO4 TCON WB_IO3    
Slot F ==> Serial 1 ALERT WB_IO5 TCON WB_IO6     

# Functions

## Construct a new RAK13015 object
Control pins and alert pins are setup depending on the provided BaseBoard and Slot parameters
    
```cpp
	RAK13015(uint8_t slot, uint8_t base_board);
```

### Parameters
@param slot WisBlock Base Board Slot. Possible SLOT_A ... SLOT_F     
@param base_board WisBlock Base Board used. Possible values RAK19007 RAK19003 RAK19001
    
### Usage     
```cpp    
RAK13015 rak_in(SLOT_D, RAK19007); // Use Slot D on RAk19007     
```

## Initialize RAK13015.
Initializes Analog inputs, 4-20mA inputs and RS485 as simple ModBus RTU master
    
```cpp
	bool initRAK13015(float analog_resolution = SGM58031_FS_4_096, uint16_t baud = 9600);
```

### Parameters
@param analog_resolution ADC resolution. Possible values SGM58031_FS_6_144 SGM58031_FS_4_096 SGM58031_FS_2_048 SGM58031_FS_1_024 SGM58031_FS_0_512 SGM58031_FS_0_256     
@param baud Baudrate for RS485     
@return true if initialization was successfull     
@return false if slot/base board selection is invalid or initialization failed
    
### Usage     
```cpp    
bool has_rak13015 = rak_in.initRAK13015(SGM58031_FS_4_096, 9600);     
if (has_rak13015)     
{     
	Serial.println("RAK13015 initialized");     
}     
else     
{     
	Serial.println("No RAK13015 found");     
}     
```

## Initialize Analog inputs and 4-20mA inputs only.
Use when RS485 is setup with other RS485/Modbus drivers
    
```cpp
	bool initAnalog_4_20(float resolution = SGM58031_FS_4_096);
```

### Parameters
@param resolution  ADC resolution. Possible values SGM58031_FS_6_144 SGM58031_FS_4_096 SGM58031_FS_2_048 SGM58031_FS_1_024 SGM58031_FS_0_512 SGM58031_FS_0_256     
@return true if initialization was successfull     
@return false if slot/base board selection is invalid or initialization failed
    
### Usage     
```cpp    
bool has_rak13015 = rak_in.initAnalog_4_20(SGM58031_FS_4_096);     
if (has_rak13015)     
{     
	Serial.println("RAK13015 initialized");     
}     
else     
{     
	Serial.println("No RAK13015 found");     
}     
```

## Read analog port.
Reads selected analog port
    
```cpp
	float readAnalog(uint16_t port);
```

### Parameters
@param port Analog port to be read. Possible values ANA_CH_0 ANA_CH_1     
@return float Measured voltage in volt
    
### Usage     
```cpp    
bool has_rak13015 = rak_in.initRAK13015(SGM58031_FS_4_096, 9600);     
if (has_rak13015)     
{     
	Serial.println("RAK13015 initialized");     
}     
else     
{     
	Serial.println("No RAK13015 found");     
}     
float a_4_20[5];     
// Read analog interfaces     
Serial.println("===============================================================");     
Serial.println("Read analog interfaces");     
Serial.printf("ANA_CH_0: %.2f V\r\n",rak_in.readAnalog(ANA_CH_0));     
Serial.printf("ANA_CH_1: %.2f V\r\n",rak_in.readAnalog(ANA_CH_1));     
```

## Read 4-20mA port.
Reads selected 4-20mA port
    
```cpp
	float read4_20ma(uint16_t port);
```

### Parameters
@param port 4-20mA port to be read. Possible values I_4_20_CH_0 I_4_20_CH_1 I_4_20_CH_2     
@return float Measured current in mA
    
### Usage     
```cpp    
bool has_rak13015 = rak_in.initRAK13015(SGM58031_FS_4_096, 9600);     
if (has_rak13015)     
{     
	Serial.println("RAK13015 initialized");     
}     
else     
{     
	Serial.println("No RAK13015 found");     
}     
float a_4_20[5];     
// Read 4-20mA interfaces     
Serial.println("===============================================================");     
Serial.println("Read 4-20mA interfaces");     
Serial.printf("4-20mA CH_0: %.2f mA\r\n", rak_in.read4_20ma(I_4_20_CH_0));     
Serial.printf("4-20mA CH_1: %.2f mA\r\n", rak_in.read4_20ma(I_4_20_CH_1));     
Serial.printf("4-20mA CH_2: %.2f mA\r\n", rak_in.read4_20ma(I_4_20_CH_2));     
```

## Initialize the RS485 interface as simple Modbus RTU master device
    
```cpp
	bool initModbus(uint16_t baud);
```

### Parameters
@param baud RS485 baud rate     
@return true if initialization was successfull     
@return false if slot/base board selection is invalid or initialization failed
    
### Usage     
```cpp    
bool has_rak13015 = rak_in.initModbus(9600);     
if (has_rak13015)     
{     
	Serial.println("RAK13015 initialized");     
}     
else     
{     
	Serial.println("No RAK13015 found");     
}     
```

## Request data from slave device on Modbus
    
```cpp
	bool requestModBus(uint8_t slave_addr, uint16_t address, uint16_t num_coils, uint16_t *coils_regs, time_t timeout);
```

### Parameters
@param slave_addr Slave address     
@param address Register/Coil start address for reading     
@param num_coils Number of registers/coils to be read     
@param coils_regs Buffer to save returned data     
@param timeout Timeout in ms to wait for failed return     
@return true Data was received     
@return false No Data was received
    
### Usage     
```cpp    
bool has_rak13015 = rak_in.initRAK13015(SGM58031_FS_4_096, 9600);     
if (has_rak13015)     
{     
	Serial.println("RAK13015 initialized");     
}     
else     
{     
	Serial.println("No RAK13015 found");     
}     
uint16_t coils_n_regs[8];     
// Read first 5 registers from ModBus device 1     
Serial.println("===============================================================");     
Serial.println("Read first 5 registers from Modbus slave device #1");     
rak_in.requestModBus(1, 0, 5, coils_n_regs, 5000);
    
Serial.println("===============================================================");     
Serial.println("Modbus device 1 first 5 registers:");     
Serial.printf("HEX 0: %04X 1: %04X 2: %04X 3: %04X 4: %04X\r\n", coils_n_regs[0], coils_n_regs[1], coils_n_regs[2], coils_n_regs[3], coils_n_regs[4]);     
Serial.printf("DEC 0: %d 1: %d 2: %d 3: %d 4: %d\r\n", coils_n_regs[0], coils_n_regs[1], coils_n_regs[2], coils_n_regs[3], coils_n_regs[4]);     
```

## Send data over Modbus to a slave device
    
```cpp
	bool writeModBus(uint8_t slave_addr, uint16_t address, uint16_t num_coils, uint16_t *coils_regs, time_t timeout);
```

### Parameters
@param slave_addr Slave address     
@param address Register/Coil start address for writing     
@param num_coils Number of registers/coils to be written     
@param coils_regs Buffer with data to write     
@param timeout Timeout in ms to wait for failed response     
@return true Data was sent     
@return false Failure to send data     

### Usage     
```cpp    
bool has_rak13015 = rak_in.initRAK13015(SGM58031_FS_4_096, 9600);     
if (has_rak13015)     
{     
	Serial.println("RAK13015 initialized");     
}     
else     
{     
	Serial.println("No RAK13015 found");     
}     
uint16_t coils_n_regs[8];     
coils_n_regs[0] = 0x00;     
coils_n_regs[1] = 0x01;     
coils_n_regs[2] = 0x02;     
coils_n_regs[3] = 0x03;     
coils_n_regs[4] = 0x04;     
// Write to first 5 registers of ModBus device 1     
Serial.println("===============================================================");     
Serial.println("Write to first 5 registers of Modbus slave device #1");     
if (rak_in.writeModBus(1, 0, 5, coils_n_regs, 5000))     
{     
	Serial.println("Modbus write success");     
}     
else     
{     
	Serial.println("Modbus write failed");     
}     
```

