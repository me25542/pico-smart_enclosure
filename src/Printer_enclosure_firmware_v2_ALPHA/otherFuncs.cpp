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
#include <pico/stdlib.h>
#include <Wire.h>
#include <Bounce2.h>  //  include the debouncing library
#include "vars.h"
#include "otherFuncs.h"

//  a simple function to blink the built-in LED
void blinkLED(uint32_t blinkTime) {
  #ifdef debug
  uint8_t core = rp2040.cpuid();
  printf("blinkLED(%u) called from core%u.\n", blinkTime, core);  //  print a debug message over the sellected debug port
  #endif

  digitalWrite(ledPin, HIGH);  //  turn on the built-in LED
  delay(blinkTime);  //  wait 1 second
  digitalWrite(ledPin, LOW);  //  turn off the built-in LED
}

//  the function that checks if a given I2C device is connected
bool isI2CDeviceConnected(uint8_t address) {
  #ifdef debug
  uint8_t core = rp2040.cpuid();
  printf("isI2CDeiceConnected(%u) called from core%u.\n", address, core);  //  print a debug message over the sellected debug port
  #endif

  uint16_t i;
  bool isNotError = false;

  /*
  //  start I2C0 (the sensor bus):
  {
    uint32_t i;
    for (i = 0; i2c0InUse && i < maxI2CWaitTime; i++) {
      delayMicroseconds(1);
    }
    
    if (i >= maxI2CWaitTime) {
      mode = 0;
      errorOrigin = 13;
      errorInfo = 2;
      errorInfo = errorInfo << 16;
      errorInfo += address;
      printf("wait for I2C resource timeout.\n");
      return false;
    }
  }

  i2c0InUse = true;
  */
  //mutex_enter_blocking(&I2C_mutex);  //  wait for I2C resources to be available
  {
    uint32_t i;
    for (i = 0; !mutex_try_enter(&I2C_mutex, &I2C_mutex_owner) && i < maxI2CWaitTime; i++) {
      delayMicroseconds(1);
    }

    if (i >= maxI2CWaitTime) {
      mode = 0;
      errorOrigin = 13;
      errorInfo = 2;
      errorInfo = errorInfo << 16;
      errorInfo += address;
      return false;
    }
  }

  for (i = 0; !isNotError && i < 1000; i++) {
    Wire.beginTransmission(address);  //  starts a transmition to the specified device
    uint8_t I2CError = Wire.endTransmission(true);  //  record the response from ending the transmition and releasing the bus
    isNotError = I2CError == 0;
  }

  //i2c0InUse = false;
  mutex_exit(&I2C_mutex);

  #ifdef debug
  printf("isI2CDeviceConnected returned: %d\n", isNotError);  //  print a debug message over the sellected debug port
  #endif

  return isNotError;  //  this will make the function return true if there were no errors, false otherwise
}

//  the function that checks if all sensors are connected
bool areSensorsPresent() {
  #ifdef debug
  uint8_t core = rp2040.cpuid();
  printf("areSensorsPreasent() called from core%u.\n", core);  //  print a debug message over the sellected debug port
  #endif

  bool sensorNotConnected = false;
  
  {
    uint8_t i;

    for (i = 0; !isI2CDeviceConnected(heaterTempSensorAdress) && i < 255; i++) {  //  test up to 255 times if the sensor is conneceted
      delayMicroseconds(10);  //  wait a tiny bit between each test
    }

    if (i >= 255) {
      mode = 0;
      errorOrigin = 6;
      errorInfo = 1;
      sensorNotConnected = true;
    }
  }

  {
    uint8_t i;

    for (i = 0; !isI2CDeviceConnected(inTempSensorAdress) && i < 255; i++) {  //  test up to 255 times if the sensor is conneceted
      delayMicroseconds(10);  //  wait a tiny bit between each test
    }

    if (i >= 255) {
      mode = 0;
      errorOrigin = 6;
      errorInfo = 2;
      sensorNotConnected = true;
    }
  }

  {
    uint8_t i;

    for (i = 0; !isI2CDeviceConnected(outTempSensorAdress) && i < 255; i++) {  //  test up to 255 times if the sensor is conneceted
      delayMicroseconds(10);  //  wait a tiny bit between each test
    }

    if (i >= 255) {
      mode = 0;
      errorOrigin = 6;
      errorInfo = 3;
      sensorNotConnected = true;
    }
  }

  bool allSensorsConnected = ! sensorNotConnected;

  #ifdef debug
  printf("areSensorsPreasent() returned: %d\n", allSensorsConnected);  //  print a debug message over the sellected debug port
  #endif

  return allSensorsConnected;
}

//  checks if the screen is connected
bool isScreenConnected() {
  #ifdef debug
  uint8_t core = rp2040.cpuid();
  printf("isScreenConnected() called from core%u.\n", core);  //  print a debug message over the sellected debug port
  #endif

  {
    uint8_t i;

    for (i = 0; !isI2CDeviceConnected(SCREEN_ADDRESS) && i < 255; i++) {  //  test up to 255 times if the screen is conneceted
      delayMicroseconds(10);  //  wait a tiny bit between each test
    }

    if (i >= 255) {
      mode = 0;
      errorOrigin = 12;
      errorInfo = SCREEN_ADDRESS;
      return false;

    } else {
      return true;
    }
  }
  /*
  if (isI2CDeviceConnected(SCREEN_ADDRESS)) {
    return true;
  } else {
    mode = 0;
    errorOrigin = 12;
    errorInfo = SCREEN_ADDRESS;
    return false;
  }
  */
}


//  the function to set the state of the PSU (on or off) | returns true on sucess, false on failure
bool setPSU(bool state) {
  #ifdef debug
  uint8_t core = rp2040.cpuid();
  printf("setPSU(%d) called from core%u.\n", state, core);  //  print a debug message over the sellected debug port
  #endif

  digitalWrite(psPin, state);  //  set the power state of the PSU

  if (state) {  //  if we are turning the PSU on
    uint16_t i;
    for (i = 0; ((digitalRead(pokPin) != state) && (i < maxPSUOnTime)); i++) {  //  wait for the PSU to report the power state is the same as requested
      delay(1);  //  wait for one milisecond
    }

    if (i < maxPSUOnTime) {
      PSUIsOn = state;
      return true;

    } else {
      mode = 0;  //  set mode to error
      errorOrigin = 9;  //  record what caused the error
      errorInfo = state;  //  record some info about the error
      return false;
    }

  } else {
    return true;
  }
}

//  the function to set the position of the servos
void setServos(uint8_t s1_Pos, uint8_t s2_Pos) {
  #ifdef debug
  uint8_t core = rp2040.cpuid();
  printf("setServos(%u, %u) called from core%u.\n", s1_Pos, s2_Pos, core);  //  print a debug message over the sellected debug port
  #endif

  servo1.write(s1_Pos);  //  set the position of servo1
  servo2.write(s2_Pos);  //  set the position of servo2
  if ((s1_Pos != oldS1_Pos) || (s2_Pos != oldS2_Pos)) {  //  if the servos are actually being set to something they wern't before:
    delay(500);  //  wait half a second for the servos to actually move
  }
  oldS1_Pos = s1_Pos;  //  record the state of the servos
  oldS2_Pos = s2_Pos;  //  record the state of the servos
}

//  the function to set the state of booth heaters, and the fan (the temp-related stuff)
void setHeaters(bool h1_On, bool h2_On, uint8_t fanVal) {
  #ifdef debug
  uint8_t core = rp2040.cpuid();
  printf("setHeaters(%d, %d, %u) called from core%u.\n", h1_On, h2_On, fanVal, core);  //  print a debug message over the sellected debug port
  #endif

  if (doorOpen) {
    #ifdef debug
    printf("setHeaters() will turn everything off; the door is open.\n");  //  print a debug message over the sellected debug port
    #endif

    h1_On = false;
    h2_On = false;
    fanVal = 0;
  } else {
    #ifdef debug
    printf("setHeaters() will do as requested; the door is closed.\n");  //  print a debug message over the sellected debug port
    #endif
  }

  digitalWrite(heater1Pin, ! h1_On);  //  set the value of heater1
  digitalWrite(heater2Pin, ! h2_On);  //  set the value of heater2
  if (fanVal != 0) {  //  if the fan is being set to something other than 0:
    setServos(servo1Open, servo2Open);  //  move the servos to open
    digitalWrite(fanPin, HIGH);  //  turn the fan on at 100%
    delay(fanKickstartTime);  //  and wait for a bit to let it spin up
    analogWrite(fanPin, min(fanVal, maxFanSpeed));  //  then set the fan speed to its target value or the maximum fan speed, whichever is less

  } else {  //  if the fan is being set to 0:
    setServos(servo1Closed, servo2Closed);  //  move the servos to closed
    digitalWrite(fanPin, LOW);  //  turn the fan off
  }
}

//  the function that starts serial (USB) comunication (takes ~10ms) | returns 1 if a computer is connected, 0 if not
bool startSerial() {
  Serial.begin(serialSpeed);  //  start serial (over USB) with a baud rate of 115200
  return Serial;  //  send back to the calling function if serial is connected
}

// the function that gets all temp sensor data | returns 0 on failure, 1 on sucess
bool getTemp() {
  #ifdef debug
  uint8_t core = rp2040.cpuid();
  printf("getTemp() called from core%u.\n", core);  //  print a debug message over the sellected debug port
  #endif
  
  
  {
    uint32_t i;
    for (i = 0; !mutex_try_enter(&I2C_mutex, &I2C_mutex_owner) && i < maxI2CWaitTime; i++) {
      delayMicroseconds(1);
    }

    if (i >= maxI2CWaitTime) {
      mode = 0;
      errorOrigin = 13;
      errorInfo = 4;
      errorInfo = errorInfo << 16;
      return false;
    }
  }
  
  //  wake up I2C temp sensors:
  heaterTempSensor.wake();
  inTempSensor.wake();
  outTempSensor.wake();

  mutex_exit(&I2C_mutex);

  //  make some temporary variables to store the sum of all mesured temps (for avereging)
  int16_t tempHeaterTemp = 0;
  int16_t tempInTemp = 0;
  int16_t tempOutTemp = 0;

  for (uint8_t i = 0; i < sensorReads; i++) {  //  do the stuff in this loop the number of times set by sensorReads
    {
      uint32_t i;
      for (i = 0; !mutex_try_enter(&I2C_mutex, &I2C_mutex_owner) && i < maxI2CWaitTime; i++) {
        delayMicroseconds(1);
      }

      if (i >= maxI2CWaitTime) {
        mode = 0;
        errorOrigin = 13;
        errorInfo = 4;
        errorInfo = errorInfo << 16;
        return false;
      }
    }

    int16_t tHeaterTemp = static_cast<int16_t>(heaterTempSensor.readTempC());  //  get the temperature from the sensor

    mutex_exit(&I2C_mutex);

    if (tHeaterTemp <= minTemp || tHeaterTemp >= maxHeaterTemp) {  //  if it is not within the acceptable range
      mode = 0;  //  set the mode to error
      errorOrigin = 1;  //  record what caused the error

      //  record both the sensor that caused the error, and the value it read, all in one variable
      errorInfo = 1;  //  record the sensor that triggered the error
      errorInfo <<= 16;  //  move that info 8 bits to the left (errorInfo is a 32-bit signed integer)
      errorInfo += static_cast<uint8_t>(tHeaterTemp);  //  add an aproxamation of the read value to errorInfo

      return false;

    } else {  //  if it is withing the acceptable range
      tempHeaterTemp += tHeaterTemp;  //  add the reading to a avriable so it can be averaged
    }

    {
      uint32_t i;
      for (i = 0; !mutex_try_enter(&I2C_mutex, &I2C_mutex_owner) && i < maxI2CWaitTime; i++) {
        delayMicroseconds(1);
      }

      if (i >= maxI2CWaitTime) {
        mode = 0;
        errorOrigin = 13;
        errorInfo = 4;
        errorInfo = errorInfo << 16;
        return false;
      }
    }

    int16_t tInTemp = static_cast<int16_t>(inTempSensor.readTempC());  //  get the temperature from the sensor

    mutex_exit(&I2C_mutex);

    if (tInTemp <= minTemp || tInTemp >= maxInOutTemp) {  //  if it is not within the acceptable range
      mode = 0;  //  set the mode to error
      errorOrigin = 1;  //  record what caused the error

      //  record both the sensor that caused the error, and the value it read, all in one variable
      errorInfo = 2;  //  record the sensor that triggered the error
      errorInfo <<= 16;  //  move that info 8 bits to the left (errorInfo is a 32-bit signed integer)
      errorInfo += static_cast<uint8_t>(tInTemp);  //  add an aproxamation of the read value to errorInfo

      return false;

    } else {
      tempInTemp += tInTemp;  //  add the reading to a avriable so it can be averaged
    }

    {
      uint32_t i;
      for (i = 0; !mutex_try_enter(&I2C_mutex, &I2C_mutex_owner) && i < maxI2CWaitTime; i++) {
        delayMicroseconds(1);
      }

      if (i >= maxI2CWaitTime) {
        mode = 0;
        errorOrigin = 13;
        errorInfo = 4;
        errorInfo = errorInfo << 16;
        return false;
      }
    }

    int16_t tOutTemp = static_cast<int16_t>(outTempSensor.readTempC());  //  get the temperature from the sensor

    mutex_exit(&I2C_mutex);

    if (tOutTemp <= minTemp || tOutTemp >= maxInOutTemp) {  //  if it is not within the acceptable range
      mode = 0;  //  set the mode to error
      errorOrigin = 1;  //  record what caused the error

      //  record both the sensor that caused the error, and the value it read, all in one variable
      errorInfo = 4;  //  record the sensor that triggered the error
      errorInfo <<= 16;  //  move that info 8 bits to the left (errorInfo is a 32-bit signed integer)
      errorInfo += static_cast<uint8_t>(tOutTemp);  //  add an aproxamation of the read value to errorInfo

      return false;

    } else {
      tempOutTemp += tOutTemp;  //  add the reading to a variable so it can be averaged
    }
  }

  {
    uint32_t i;
    for (i = 0; !mutex_try_enter(&I2C_mutex, &I2C_mutex_owner) && i < maxI2CWaitTime; i++) {
      delayMicroseconds(1);
    }

    if (i >= maxI2CWaitTime) {
      mode = 0;
      errorOrigin = 13;
      errorInfo = 4;
      errorInfo = errorInfo << 16;
      return false;
    }
  }
  
  //  put the I2C temp sensors to sleep:
  heaterTempSensor.shutdown();
  inTempSensor.shutdown();
  outTempSensor.shutdown();
  
  mutex_exit(&I2C_mutex);

  heaterTemp = tempHeaterTemp / sensorReads;  //  set the temperature to the temporary variable devided by the number of times the temperature was read
  inTemp = tempInTemp / sensorReads;  //  set the temperature to the temporary variable devided by the number of times the temperature was read
  outTemp = tempOutTemp / sensorReads;  //  set the temperature to the temporary variable devided by the number of times the temperature was read

  #ifdef debug
  printf("Heater temp: %d. | In temp: %d. | Out temp: %d.\n", heaterTemp, inTemp, outTemp);  //  print a debug message over the sellected debug port
  #endif

  return true;
}

void lightswitchPressed() {
  #ifdef debug
  uint8_t core = rp2040.cpuid();
  printf("lightswitchPressed() called from core%u.\n", core);  //  print a debug message over the sellected debug port
  #endif

  if (lights_On_On_Door_Open && doorOpen) {
    lightswPressedSenceDoorOpen = true;
  }

  mainLight.changeState();
}

//  the functoin called when a manual cooldown is triggered (the cooldown button is released):
void manualCooldown() {
  #ifdef debug
  uint8_t core = rp2040.cpuid();
  printf("manualCooldown() called from core%u.\n", core);  //  print a debug message over the sellected debug port
  #endif

  mode = 2;  //  set the mode to cooldown
}

//  the function called when the door opens (the door switch is released):
void doorOpening() {
  #ifdef debug
  uint8_t core = rp2040.cpuid();
  printf("doorOpening() called from core%u.\n", core);  //  print a debug message over the sellected debug port
  #endif

  doorOpen = true;  //  remember the door is open
  printDone = false;  //  the print might have been removed; remember that the print is not done

  if (lights_On_On_Door_Open) {  //  if the lights should change on door open
    lightStateBeforeDoorOpen = lightState;  //  record the lights state
    lightswPressedSenceDoorOpen = false;  //  the lightswitch hsen't been pressed sence the door was opened
    mainLight.setState(true);  //  turn on the lights
  }

  if (mode != 3 && printName[0] != 0) {  //  if the mode is not printing and the print name isn't empty (well, technically if the first character of the print name isn'y NULL)
    #ifdef debug
    printf("clearing the print name.\n");  //  print a debug message over the sellected debug port
    #endif

    for (uint16_t i = 0; i < 256; i++) {  //  go through all the characters in the print name
      printName[i] = 0;  //  and set them to NULL
    }  //  for (uint8_t i = 0; i < 256; i++)
  }  //  if (mode != 3)

  setHeaters(false, false, 0);  //  turn off both heaters and the fan

  #ifdef debug
  printf("Door open.\n");  //  print a debug message over the sellected debug port
  #endif
}

//  the function called when the door closes (the door switch is pressed):
void doorClosing() {
  #ifdef debug
  uint8_t core = rp2040.cpuid();
  printf("doorClosing() called from core%u.\n", core);  //  print a debug message over the sellected debug port
  #endif

  if (lights_On_On_Door_Open && (! lightswPressedSenceDoorOpen)) {  //  if the lights should change on door open AND they haven't been changed sence the door was opened
    mainLight.setState(lightStateBeforeDoorOpen);  //  set the lights to be how they were before the door was opened
  }

  doorOpen = false;  //  remember that the door is closed
  printDone = false;  //  the print might have been removed; remember that the print is not done

  #ifdef debug
  printf("Door closed.\n");  //  print a debug message over the sellected debug port
  #endif
}

//  checks if the buttons have been pressed or relesed, and calls the correct functions based on the state of the buttons
void checkButtons() {
  #ifdef debug
  uint8_t core = rp2040.cpuid();
  printf("checkButtons() called from core%u.\n", core);  //  print a debug message over the sellected debug port
  #endif

  //  update the buttons
  door_switch.update();
  light_switch.update();
  coolDown_switch.update();

  if (door_switch.pressed()) {
    doorClosing();
  }

  if (door_switch.released()) {
    doorOpening();
  }

  if (light_switch.released()) {
    #ifdef debug
    printf("Light switch pressed.\n");  //  print a debug message over the sellected debug port
    #endif

    //mainLight.changeState();  //  Change the light state
    lightswitchPressed();
  }

  if (coolDown_switch.isPressed() && coolDown_switch.currentDuration() >= cooldownSwitchHoldTime) {  //  if the cooldown switch was just relesed and was previusly held for over a set length of time:
    #ifdef debug
    printf("Cooldown switch pressed.\n");  //  print a debug message over the sellected debug port
    #endif

    manualCooldown();  //  set mode to cooldown
  }
}


