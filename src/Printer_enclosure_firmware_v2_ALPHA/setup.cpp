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
#include "vars.h"
#include "otherFuncs.h"
#include "ISRs.h"

#include "setup.h"

void servoSetup() {
  #ifdef debug
  printf("servoSetup() called.\n");  //  print a debug message over the sellected debug port
  #endif

  //  start servos:
  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);

  //  move servos to home position (closed, or 0deg):
  servo1.write(servo1Closed);
  servo2.write(servo2Closed);
}

bool tempSensorSetup() {
  #ifdef debug
  printf("tempSensorSetup() called.\n");  //  print a debug message over the sellected debug port
  #endif

  {
    uint32_t i;
    for (i = 0; !mutex_try_enter(&I2C_mutex, &I2C_mutex_owner) && i < maxI2CWaitTime; i++) {
      delayMicroseconds(1);
    }

    if (i >= maxI2CWaitTime) {
      mode = 0;
      errorOrigin = 13;
      errorInfo = 1;
      errorInfo = errorInfo << 16;
      return false;
    }
  }

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

    #ifdef debug
    printf("Starting the heater sensor failed.\n");  //  print a debug message over the sellected debug port
    #endif

    //i2c0InUse = false;
    mutex_exit(&I2C_mutex);

    return false;
  }

  if (! inTempSensor.begin(inTempSensorAdress)) {  //  try to start the in temp sensor and check if it worked
    mode = 0;  //  set mode to error if it didn't work
    errorOrigin = 3;  //  record where the error came from
    errorInfo = 2;  //  record which sensor is at fault

    #ifdef debug
    printf("Starting the in temp sensor failed.\n");  //  print a debug message over the sellected debug port
    #endif

    //i2c0InUse = false;
    mutex_exit(&I2C_mutex);

    return false;
  }

  if (! outTempSensor.begin(outTempSensorAdress)) {  //  try to start the out temp sensor and check if it worked
    mode = 0;  //  set mode to error if it didn't work
    errorOrigin = 3;  //  record where the error came from
    errorInfo = 3;  //  record which sensor is at fault

    #ifdef debug
    printf("Starting the out temp sensor failed.\n");  //  print a debug message over the sellected debug port
    #endif

    //i2c0InUse = false;
    mutex_exit(&I2C_mutex);

    return false;
  }

  //  set I2C temp sensor resolution:
  heaterTempSensor.setResolution(i2cTempSensorRes);
  inTempSensor.setResolution(i2cTempSensorRes);
  outTempSensor.setResolution(i2cTempSensorRes);

  //i2c0InUse = false;
  mutex_exit(&I2C_mutex);

  //  check if the sensors are connected ant try to get the temperature from them
  if (areSensorsPresent() && getTemp()) {  //  if the sensors are preasant AND geting the temperature was sucessfull | these will both handel I2C availability themselves

    #ifdef debug
    printf("Sensor startup sucessful.\n");  //  print a debug message over the sellected debug port
    #endif

    return true;

  } else {
    #ifdef debug
    printf("Sensor startup failed.\n");  //  print a debug message over the sellected debug port
    #endif

    return false;
  }
}

void printerI2cSetup() {
  #ifdef debug
  printf("printerI2cSetup()called.\n");  //  print a debug message over the sellected debug port
  #endif

  //  start I2C1 (the priter's bus):
  Wire1.setSDA(I2C1_SDA);  //  set the SDA of I2C1
  Wire1.setSCL(I2C1_SCL);  //  set the SCL of I2C1
  Wire1.begin(0x08);  // Initialize the I2C1 bus with the address 0x08 (8)
  Wire1.onRequest(requestEvent);  //  set the function to call when the printer requests data via I2C
  Wire1.onReceive(I2cReceived);  //  set the function to call when the printer sends data via I2C
}

void buttonSetup() {
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

  //  set switch pressed states
  door_switch.setPressedState(! digitalRead(doorSwitchPin));
  light_switch.setPressedState(! digitalRead(lightSwitchPin));
  coolDown_switch.setPressedState(! digitalRead(coolDownSwitchPin));
  sell_switch.setPressedState(! digitalRead(sellPin));
  up_switch.setPressedState(! digitalRead(upPin));
  down_switch.setPressedState(! digitalRead(downPin));

  //  update switches
  door_switch.update();
  light_switch.update();
  coolDown_switch.update();
  sell_switch.update();
  up_switch.update();
  down_switch.update();
}

void pinSetup() {
  #ifdef debug
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
  digitalWrite(errorLightPin, LOW);
  digitalWrite(heater1Pin, HIGH);  //  turn off heater 1
  digitalWrite(heater2Pin, HIGH);  //  turn off heater 2
  digitalWrite(fanPin, LOW);  //  turn off the fan
  digitalWrite(psPin, HIGH);  //  turn on the power supply
}

bool menuSetup() {
  #ifdef debug
  printf("menuSetup() called.\n");  //  print a debug message over the sellected debug port
  #endif
  
  {
    uint32_t i;
    for (i = 0; !mutex_try_enter(&I2C_mutex, &I2C_mutex_owner) && i < maxI2CWaitTime; i++) {
      delayMicroseconds(1);
    }

    if (i >= maxI2CWaitTime) {
      mode = 0;
      errorOrigin = 13;
      errorInfo = 9;
      errorInfo = errorInfo << 16;
      return false;
    }
  }

  uint16_t i;
  for (i = 0; (i < maxScreenStartupTries) && !display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS); i++) { delay(1); }

  if (i >= maxScreenStartupTries) {
    mode = 0;
    errorOrigin = 10;

    #ifdef debug
    printf("Starting screen failed.\n");  //  print a debug message over the sellected debug port
    #endif

    //i2c0InUse = false;
    mutex_exit(&I2C_mutex);
  
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

    //i2c0InUse = false;
    mutex_exit(&I2C_mutex);

    return true;
  }
}

void backupRecovery() {
  if (static_cast<bool>(EEPROM.read(0))) {  //  if the stored data indicates to use itself
    //  get the stored version info (the version info of the code that stored the data)
    //  we do this so that if data compatability is broken by some future update we don't break things
    uint8_t stored_majorVersion = EEPROM.read(1);
    uint8_t stored_minorVersion = EEPROM.read(2);
    uint8_t stored_bugFixVersion = EEPROM.read(3);
    uint8_t stored_buildVersion = EEPROM.read(4);

    if ((stored_majorVersion == majorVersion) && (stored_minorVersion == minorVersion) && (stored_bugFixVersion == bugFixVersion) && (stored_buildVersion == buildVersion)) {
        //  a bit long here, but basically if the version is the same as the stored one

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

            return;
          }

          case 4: {
            //  read the saved data
            uint8_t recoveredByte0 = EEPROM.read(memAdr++);
            uint8_t recoveredByte1 = EEPROM.read(memAdr++);

            int16_t recoveredData = recoveredByte0;
            recoveredData <<= 8;  //  move the first byte of the recovered data 8 bits (one byte) to the left
            recoveredData += recoveredByte1;  //  add the second byte of the recovered data

            mainMenu[i].int16ToMenuItem(recoveredData);

            return;
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

            return;
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

            return;
          }  //  case 6:
        }  //  switch (mainMenu[i].getDataType())
      }  //  for (uint8_t i = 0; i < menuLength; i++)
    }  //  if ((stored_majorVersion == majorVersion) && (stored_minorVersion == minorVersion) && (stored_bugFixVersion == bugFixVersion) && (stored_buildVersion == buildVersion))
  }  //  if (static_cast<bool>(EEPROM.read(0)))
}  //  void backupRecovery()

void serialSetup() {
  #ifdef debug
  printf("serialSetup() called.\n");  //  print a debug message over the sellected debug port
  #endif
  
  for (uint16_t serialStartupTries = 0; serialStartupTries < maxSerialStartupTries && !startSerial(); ++ serialStartupTries);  //  start serial (USB) comunication (and wait for up to one second for a computer to be connected)
  Serial.setTimeout(serialTimout);  //  set the serial timout time
}

