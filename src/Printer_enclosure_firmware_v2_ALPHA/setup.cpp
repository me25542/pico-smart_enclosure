/*
 * Copyright (c) 2024 Dalen Hardy
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
*/

#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>  //  Include the library for I2C comunication
#include <Adafruit_MCP9808.h>  //  Include the library for using the temp sensors
#include <Adafruit_GFX.h>  //  Include the graphics library
#include <Adafruit_SSD1306.h>  //  Include the library for controling the screen
#include <Servo.h>  //  include the library for controling servos
#include <Bounce2.h>  //  include the debouncing library

#include "vars.hpp"
#include "otherFuncs.hpp"
#include "ISRs.hpp"

#include "setup.hpp"


void varInit() {
  for (uint16_t i = 0; i <= 255; i++) {  //  repeat for all characters in printName
    printName[i] = 0;  //  set a character in the print name to NULL (0)
  }

  mutex_init(&I2C_mutex);  //  initialize the mutex protecting I2C0
  mutex_init(&PSU_mutex);  //  initialize the mutex protecting seting the PSU
}

void servoSetup() {
  #if debug
  printf("servoSetup() called.\n");  //  print a debug message over the sellected debug port
  #endif

  //  start servos:
  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);

  //  move servos to home position (closed, or 0deg):
  servo1.write(servo1Closed);
  servo2.write(servo2Closed);

  #if debug
  printf("Exiting servoSetup().\n");
  #endif
}

bool tempSensorSetup() {
  #if debug
  printf("tempSensorSetup() called.\n");  //  print a debug message over the sellected debug port
  #endif

  useI2C(1);

  Wire.setSDA(I2C0_SDA);  //  set the SDA of I2C0
  Wire.setSCL(I2C0_SCL);  //  set the SCL of I2C0
  Wire.setClock(400000);  //  set the bus to fast mode
  Wire.begin();  //  Initialize the I2C0 bus as the master

  delay(1);  //  wait for a tiny bit

  //  start I2C temp sensors and set mode to error if it fails:
  if (! heaterTempSensor.begin(heaterTempSensorAdress)) {  //  try to start the heater temp sensor and check if it worked
    mode = 0;  //  set mode to error if it didn't work
    errorOrigin = 3;  //  record where the error came from
    errorInfo = 1;  //  record which sensor is at fault

    #if debug
    printf("Starting the heater sensor failed.\n");  //  print a debug message over the sellected debug port
    #endif

    doneWithI2C();

    return false;
  }

  if (! inTempSensor.begin(inTempSensorAdress)) {  //  try to start the in temp sensor and check if it worked
    mode = 0;  //  set mode to error if it didn't work
    errorOrigin = 3;  //  record where the error came from
    errorInfo = 2;  //  record which sensor is at fault

    #if debug
    printf("Starting the in temp sensor failed.\n");  //  print a debug message over the sellected debug port
    #endif

    doneWithI2C();

    return false;
  }

  if (! outTempSensor.begin(outTempSensorAdress)) {  //  try to start the out temp sensor and check if it worked
    mode = 0;  //  set mode to error if it didn't work
    errorOrigin = 3;  //  record where the error came from
    errorInfo = 3;  //  record which sensor is at fault

    #if debug
    printf("Starting the out temp sensor failed.\n");  //  print a debug message over the sellected debug port
    #endif

    doneWithI2C();

    return false;
  }

  //  set I2C temp sensor resolution:
  heaterTempSensor.setResolution(i2cTempSensorRes);
  inTempSensor.setResolution(i2cTempSensorRes);
  outTempSensor.setResolution(i2cTempSensorRes);

  doneWithI2C();

  //  check if the sensors are connected ant try to get the temperature from them
  if (areSensorsPresent() && getTemp()) {  //  if the sensors are preasant AND geting the temperature was sucessfull | these will both handel I2C availability themselves

    #if debug
    printf("Sensor startup sucessful.\n");  //  print a debug message over the sellected debug port
    #endif

    return true;

  } else {
    #if debug
    printf("Sensor startup failed.\n");  //  print a debug message over the sellected debug port
    #endif

    return false;
  }
}

void printerI2cSetup() {
  #if debug
  printf("printerI2cSetup()called.\n");  //  print a debug message over the sellected debug port
  #endif

  //  start I2C1 (the priter's bus):
  Wire1.setSDA(I2C1_SDA);  //  set the SDA of I2C1
  Wire1.setSCL(I2C1_SCL);  //  set the SCL of I2C1
  Wire1.begin(0x08);  // Initialize the I2C1 bus with the address 0x08 (8)
  Wire1.onRequest(requestEvent);  //  set the function to call when the printer requests data via I2C
  Wire1.onReceive(I2cReceived);  //  set the function to call when the printer sends data via I2C

  #if debug
  printf("Exiting printerI2cSetup().\n");
  #endif
}

void buttonSetup() {
  #if debug
  printf("buttonSetup() called.\n");
  #endif

  //  set switch pinModes
  door_switch.attach(doorSwitchPin, INPUT_PULLUP);
  light_switch.attach(lightSwitchPin, INPUT_PULLUP);
  coolDown_switch.attach(coolDownSwitchPin, INPUT_PULLUP);
  sell_switch.attach(sellPin, INPUT_PULLUP);
  up_switch.attach(upPin, INPUT_PULLUP);
  down_switch.attach(downPin, INPUT_PULLUP);

  //  set switch debounce intervals
  door_switch.interval(100);
  light_switch.interval(100);
  coolDown_switch.interval(100);
  sell_switch.interval(50);
  up_switch.interval(50);
  down_switch.interval(50);

  #if !manualPressedState
  if (findPressedState) {
    doorSw_ps = !digitalRead(doorSwitchPin);
    lightSw_ps = !digitalRead(lightSwitchPin);
    coolDownSw_ps = !digitalRead(coolDownSwitchPin);
    sellSw_ps = !digitalRead(sellPin);
    upSw_ps = !digitalRead(upPin);
    downSw_ps = !digitalRead(downPin);
  }
  #endif

  //  set switch pressed states
  door_switch.setPressedState(doorSw_ps);
  light_switch.setPressedState(lightSw_ps);
  coolDown_switch.setPressedState(coolDownSw_ps);
  sell_switch.setPressedState(sellSw_ps);
  up_switch.setPressedState(upSw_ps);
  down_switch.setPressedState(downSw_ps);


  //  update switches
  door_switch.update();
  light_switch.update();
  coolDown_switch.update();
  sell_switch.update();
  up_switch.update();
  down_switch.update();

  doorOpen = !door_switch.isPressed();  //  for startup after power loss, just going by state changes dosn't work

  #if debug
  printf("Exiting buttonSetup().\n");
  #endif
}

void pinSetup() {
  #if debug
  printf("pinSetup() called.\n");  //  print a debug message over the sellected debug port
  #endif
  
  //  Pin definitions:
  //  outputs, high pin# - low pin#
  pinMode(ledPin, OUTPUT);
  pinMode(errorLightPin, OUTPUT);
  pinMode(heater2Pin, OUTPUT);
  pinMode(heater1Pin, OUTPUT);
  pinMode(psPin, OUTPUT);
  pinMode(fanPin, OUTPUT);

  //  inputs, high pin# - low pin#
  pinMode(pokPin, INPUT_PULLDOWN);

  // set initial output pin values, high pin# - low pin#:
  digitalWrite(ledPin, LOW);  //  turn off the built-in LED
  digitalWrite(errorLightPin, !errorLightOn);  //  turn off the error light
  digitalWrite(heater1Pin, HIGH);  //  turn off heater 1
  digitalWrite(heater2Pin, HIGH);  //  turn off heater 2
  digitalWrite(fanPin, LOW);  //  turn off the fan
  digitalWrite(psPin, HIGH);  //  turn on the power supply

  #if debug
  printf("Exiting pinSetup().\n");
  #endif
}

bool menuSetup() {
  #if debug
  printf("menuSetup() called.\n");  //  print a debug message over the sellected debug port
  #endif

  useI2C(9);

  uint16_t i;
  for (i = 0; (i < maxScreenStartupTries) && !display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS); i++) { delay(1); }

  if (i >= maxScreenStartupTries) {
    mode = 0;
    errorOrigin = 10;

    #if debug
    printf("Starting screen failed.\n");  //  print a debug message over the sellected debug port
    #endif

    doneWithI2C();
  
    return false;

  } else {
    display.display();

    display.clearDisplay();
    display.cp437(true);  //  use normal ASCII text encoding
    display.setCursor(0, 0);
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
    display.setTextSize(2);
    display.print("Loading...");

    display.display();

    doneWithI2C();

    return true;
  }
}

bool checkVersion() {
  //  get the stored version info (the version info of the code that stored the data)
  uint8_t stored_majorVersion = EEPROM.read(1);
  uint8_t stored_minorVersion = EEPROM.read(2);
  uint8_t stored_bugFixVersion = EEPROM.read(3);
  uint8_t stored_buildVersion = EEPROM.read(4);

  return ((stored_majorVersion == majorVersion) && (stored_minorVersion == minorVersion) && (stored_bugFixVersion == bugFixVersion) && (stored_buildVersion == buildVersion));
    //  a bit long here, but basically return true if the version is the same as the stored one, false otherwise
}

#if !manualPressedState
bool buttonRecovery() {
  bool useButtonData = !static_cast<bool>(EEPROM.read(9));  //  use the button data only if address 9 contains a 0

  if (useButtonData) {
    findPressedState = false;

    doorSw_ps = static_cast<bool>(EEPROM.read(10));
    lightSw_ps = static_cast<bool>(EEPROM.read(11));
    coolDownSw_ps = static_cast<bool>(EEPROM.read(12));
    sellSw_ps = static_cast<bool>(EEPROM.read(13));
    upSw_ps = static_cast<bool>(EEPROM.read(14));
    downSw_ps = static_cast<bool>(EEPROM.read(15));

  } else {
    findPressedState = true;
  }

  return useButtonData;
}
#endif

bool menuRecovery() {
  bool useMenuData = static_cast<bool>(EEPROM.read(0));

  if (useMenuData) {
    uint16_t memAdr = 1024;  //  start with the adress at the start of the second kb of memory

    for (uint8_t i = 0; i < menuLength; i++) {
      switch (mainMenu[i].getDataType()) {
        case 0: {
          uint8_t recoveredByte = EEPROM.read(memAdr++);  //  get a byte from the EEPROM
          bool recoveredData = static_cast<bool>(recoveredByte);  //  convert it to the right datatype
          mainMenu[i].boolToMenuItem(recoveredData);  //  set the menu item's data to the recovered value

          break;
        }

        case 1: {
          uint8_t recoveredByte = EEPROM.read(memAdr++);  //  get a byte from the EEPROM
          mainMenu[i].uint8ToMenuItem(recoveredByte);  //  in this case we don't need to convert the data, it is already in the right format

          break;
        }

        case 2: {
          uint8_t recoveredByte = EEPROM.read(memAdr++);  //  get a byte from the EEPROM
          mainMenu[i].int8ToMenuItem(static_cast<int8_t>(recoveredByte));  //  restore the saved byte (we have to convert it from an unsigned to a signed format)

          break;
        }

        case 3: {
          //  read the saved data
          uint8_t recoveredByte0 = EEPROM.read(memAdr++);
          uint8_t recoveredByte1 = EEPROM.read(memAdr++);

          uint16_t recoveredData = recoveredByte0;
          recoveredData <<= 8;  //  move the first byte of the recovered data 8 bits (one byte) to the left
          recoveredData += recoveredByte1;  //  add the second byte of the recovered data

          mainMenu[i].uint16ToMenuItem(recoveredData);

          break;
        }

        case 4: {
          //  read the saved data
          uint8_t recoveredByte0 = EEPROM.read(memAdr++);
          uint8_t recoveredByte1 = EEPROM.read(memAdr++);

          int16_t recoveredData = recoveredByte0;
          recoveredData <<= 8;  //  move the first byte of the recovered data 8 bits (one byte) to the left
          recoveredData += recoveredByte1;  //  add the second byte of the recovered data

          mainMenu[i].int16ToMenuItem(recoveredData);

          break;
        }

        case 5: {
          //  read the saved data
          uint8_t recoveredByte0 = EEPROM.read(memAdr++);
          uint8_t recoveredByte1 = EEPROM.read(memAdr++);
          uint8_t recoveredByte2 = EEPROM.read(memAdr++);
          uint8_t recoveredByte3 = EEPROM.read(memAdr++);

          uint32_t recoveredData = recoveredByte0;  //  put the first byte (8 bits) of the recovered data at the very right
          recoveredData <<= 8;  //  move the first byte of the recovered data 8 bits (one byte) to the left
          recoveredData += recoveredByte1;  //  put the second byte (8 bits) of the recovered data at the very right
          recoveredData <<= 8;  //  move the second byte of the recovered data 8 bits (one byte) to the left
          recoveredData += recoveredByte2;  //  put the third byte (8 bits) of the recovered data at the very right
          recoveredData <<= 8;  //  move the third byte of the recovered data 8 bits (one byte) to the left
          recoveredData += recoveredByte3;  //  put the fourth byte (8 bits) of the recovered data at the very right

          mainMenu[i].uint32ToMenuItem(recoveredData);

          break;
        }

        case 6: {
          //  read the saved data
          uint8_t recoveredByte0 = EEPROM.read(memAdr++);
          uint8_t recoveredByte1 = EEPROM.read(memAdr++);
          uint8_t recoveredByte2 = EEPROM.read(memAdr++);
          uint8_t recoveredByte3 = EEPROM.read(memAdr++);

          int32_t recoveredData = recoveredByte0;  //  put the first byte (8 bits) of the recovered data at the very right
          recoveredData <<= 8;  //  move the first byte of the recovered data 8 bits (one byte) to the left
          recoveredData += recoveredByte1;  //  put the second byte (8 bits) of the recovered data at the very right
          recoveredData <<= 8;  //  move the second byte of the recovered data 8 bits (one byte) to the left
          recoveredData += recoveredByte2;  //  put the third byte (8 bits) of the recovered data at the very right
          recoveredData <<= 8;  //  move the third byte of the recovered data 8 bits (one byte) to the left
          recoveredData += recoveredByte3;  //  put the fourth byte (8 bits) of the recovered data at the very right

          mainMenu[i].int32ToMenuItem(recoveredData);

          break;
        }  //  case 6:
      }  //  switch (mainMenu[i].getDataType())
    }  //  for (uint8_t i = 0; i < menuLength; i++)
  }  //  if (useMenuData)

  return useMenuData;
}  //  menuRecovery()

void backupRecovery() {
  #if debug
  printf("backupRecovery() called.\n");
  #endif

  EEPROM.begin(2048);  //  2048 because we have 1kb (1024b) for data, and I wanted a nice number of bytes to be used (we use like 10 of the remaining 1024 for version info and if to use the data)

  if (checkVersion()) {  //  if the version is the same as the stored one. We do this so that if data compatability is broken by some future update we don't break things
    #if !manualPressedState
    buttonRecovery();
    #endif

    menuRecovery();
  }  //  if (checkVersion)
}  //  void backupRecovery()

void serialSetup() {
  #if debug
  printf("serialSetup() called.\n");  //  print a debug message over the sellected debug port
  #endif
  
  for (uint16_t serialStartupTries = 0; serialStartupTries < maxSerialStartupTries && !startSerial(); ++ serialStartupTries);  //  start serial (USB) comunication (and wait for up to one second for a computer to be connected)
  Serial.setTimeout(serialTimout);  //  set the serial timout time

  #if debug
  printf("Exiting serialSetup().\n");
  #endif
}

