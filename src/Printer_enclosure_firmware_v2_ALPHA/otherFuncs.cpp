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

#include "vars.hpp"

#include "otherFuncs.hpp"


void blinkLED(uint32_t blinkTime) {
  #if debug
  uint8_t core = rp2040.cpuid();
  Serial.printf("blinkLED(%lu) called from core%u.\n", blinkTime, core);  //  print a debug message over the sellected debug port
  #endif

  digitalWrite(ledPin, HIGH);  //  turn on the built-in LED
  delay(blinkTime);  //  wait 1 second
  digitalWrite(ledPin, LOW);  //  turn off the built-in LED
}

bool useI2C(uint16_t timeout_id) {
  uint32_t i;
  for (i = 0; !mutex_try_enter(&I2C_mutex, &I2C_mutex_owner) && i < maxI2CWaitTime; i++) {
    delayMicroseconds(1);
  }

  if (i >= maxI2CWaitTime) {
    mode = ERROR;
    errorOrigin = 13;
    errorInfo = timeout_id;
    errorInfo = errorInfo << 16;
    return false;  //  timeout

  } else {
    return true;  //  got the mutex
  }
}

bool useI2C(uint16_t timeout_id, uint16_t timeout_info) {
  uint32_t i;
  for (i = 0; !mutex_try_enter(&I2C_mutex, &I2C_mutex_owner) && i < maxI2CWaitTime; i++) {
    delayMicroseconds(1);
  }

  if (i >= maxI2CWaitTime) {
    mode = ERROR;
    errorOrigin = 13;
    errorInfo = timeout_id;
    errorInfo = errorInfo << 16;
    errorInfo += timeout_info;
    return false;  //  timeout

  } else {
    return true;  //  got the mutex
  }
}

void doneWithI2C() {
  mutex_exit(&I2C_mutex);
}

bool isI2CDeviceConnected(uint8_t address) {
  #if debug
  uint8_t core = rp2040.cpuid();
  Serial.printf("isI2CDeiceConnected(%u) called from core%u.\n", address, core);  //  print a debug message over the sellected debug port
  #endif

  uint16_t i;
  bool isNotError = false;

  
  useI2C(2);

  for (i = 0; !isNotError && i < 1000; i++) {
    Wire.beginTransmission(address);  //  starts a transmition to the specified device
    uint8_t I2CError = Wire.endTransmission(true);  //  record the response from ending the transmition and releasing the bus
    isNotError = I2CError == 0;
  }

  doneWithI2C();

  #if debug
  Serial.printf("isI2CDeviceConnected returned: %d\n", isNotError);  //  print a debug message over the sellected debug port
  #endif

  return isNotError;  //  this will make the function return true if there were no errors, false otherwise
}

bool areSensorsPresent() {
  #if debug
  uint8_t core = rp2040.cpuid();
  Serial.printf("areSensorsPreasent() called from core%u.\n", core);  //  print a debug message over the sellected debug port
  #endif

  bool sensorNotConnected = false;
  
  {
    uint8_t i;

    for (i = 0; !isI2CDeviceConnected(heaterTempSensorAdress) && i < 255; i++) {  //  test up to 255 times if the sensor is conneceted
      delayMicroseconds(10);  //  wait a tiny bit between each test
    }

    if (i >= 255) {
      mode = ERROR;
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
      mode = ERROR;
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
      mode = ERROR;
      errorOrigin = 6;
      errorInfo = 3;
      sensorNotConnected = true;
    }
  }

  bool allSensorsConnected = ! sensorNotConnected;

  #if debug
  Serial.printf("areSensorsPreasent() returned: %d\n", allSensorsConnected);  //  print a debug message over the sellected debug port
  #endif

  return allSensorsConnected;
}

bool isScreenConnected() {
  #if debug
  uint8_t core = rp2040.cpuid();
  Serial.printf("isScreenConnected() called from core%u.\n", core);  //  print a debug message over the sellected debug port
  #endif

  {
    uint8_t i;

    for (i = 0; !isI2CDeviceConnected(SCREEN_ADDRESS) && i < 255; i++) {  //  test up to 255 times if the screen is conneceted
      delayMicroseconds(10);  //  wait a tiny bit between each test
    }

    if (i >= 255) {
      mode = ERROR;
      errorOrigin = 12;
      errorInfo = SCREEN_ADDRESS;
      return false;

    } else {
      return true;
    }
  }
}

bool setPSU(bool state) {
  #if debug
  uint8_t core = rp2040.cpuid();
  Serial.printf("setPSU(%d) called from core%u.\n", state, core);  //  print a debug message over the sellected debug port
  #endif

  mutex_enter_blocking(&PSU_mutex);  //  wait for any other instances of this function (the other core?) to be done

  digitalWrite(psPin, state);  //  set the power state of the PSU

  if (state) {  //  if we are turning the PSU on
    uint16_t i;
    for (i = 0; ((!digitalRead(pokPin)) && (i < maxPSUOnTime)); i++) {  //  while the PSU hasn't reported that the power state is the same as requested
      delay(1);  //  wait for one milisecond
      digitalWrite(psPin, HIGH);  //  tell the PSU to turn on | maybe this will fix the bug?
    }

    if (i < maxPSUOnTime) {
      PSUIsOn = true;
      mutex_exit(&PSU_mutex);  //  let any other instance of this function (the other core?) know that we are done
      return true;

    } else {
      mode = ERROR;  //  set mode to error
      errorOrigin = 9;  //  record what caused the error
      errorInfo = state;  //  record some info about the error
      mutex_exit(&PSU_mutex);  //  let any other instance of this function (the other core?) know that we are done
      return false;
    }

  } else {  //  if we are turning the PSU off
    PSUIsOn = false;
    mutex_exit(&PSU_mutex);  //  let any other instance of this function (the other core?) know that we are done
    return true;
  }
}

void setServos(uint8_t s1_Pos, uint8_t s2_Pos) {
  #if debug
  uint8_t core = rp2040.cpuid();
  Serial.printf("setServos(%u, %u) called from core%u.\n", s1_Pos, s2_Pos, core);  //  print a debug message over the sellected debug port
  #endif

  servo1.write(s1_Pos);  //  set the position of servo1
  servo2.write(s2_Pos);  //  set the position of servo2
  if ((s1_Pos != oldS1_Pos) || (s2_Pos != oldS2_Pos)) {  //  if the servos are actually being set to something they wern't before:
    delay(500);  //  wait half a second for the servos to actually move
  }
  oldS1_Pos = s1_Pos;  //  record the state of the servos
  oldS2_Pos = s2_Pos;  //  record the state of the servos
}

void setHeaters(bool h1_On, bool h2_On, uint8_t fanVal) {
  #if debug
  uint8_t core = rp2040.cpuid();
  Serial.printf("setHeaters(%d, %d, %u) called from core%u.\n", h1_On, h2_On, fanVal, core);  //  print a debug message over the sellected debug port
  #endif

  static bool h1_isOn = false;
  static bool h2_isOn = false;

  if (doorOpen) {
    #if debug
    Serial.printf("setHeaters() will turn everything off; the door is open.\n");  //  print a debug message over the sellected debug port
    #endif

    h1_On = false;
    h2_On = false;

    #if fanOn
    fanVal = 0;
    #else
    fanVal = 255;
    #endif

  } else {
    #if debug
    Serial.printf("setHeaters() will do as requested; the door is closed.\n");  //  print a debug message over the sellected debug port
    #endif
  }

  digitalWrite(heater1Pin, !h1_On);  //  set the value of heater1
  digitalWrite(heater2Pin, !h2_On);  //  set the value of heater2

  if ((h1_On != h1_isOn) || (h2_On != h2_isOn)) { // if we are turning the heaters off or on (if the state of the heaters is changing)
    mainLight.blip(25000); // bypass a hardware issue
  }

  h1_isOn = h1_On;
  h2_isOn = h2_On;

  if (fanVal) {  //  if the fan is being set to something other than 0:
    setServos(servo1Open, servo2Open);  //  move the servos to open
    
    if (!fanWasOn) {
      fanWasOn = true;
      analogWrite(fanPin, 255);  //  turn the fan on at 100%
      delay(fanKickstartTime);  //  and wait for a bit to let it spin up
    }
    
    analogWrite(fanPin, min(fanVal, maxFanSpeed));  //  then set the fan speed to its target value or the maximum fan speed, whichever is less

  } else {  //  if the fan is being set to 0:
    fanWasOn = false;
    setServos(servo1Closed, servo2Closed);  //  move the servos to closed
    digitalWrite(fanPin, !fanOn);  //  turn the fan off
  }
}

bool startSerial() {
  Serial.begin(serialSpeed);  //  start serial (over USB) with a baud rate of 115200
  return Serial;  //  send back to the calling function if serial is connected
}

bool getTemp() {
  #if debug
  uint8_t core = rp2040.cpuid();
  Serial.printf("getTemp() called from core%u.\n", core);  //  print a debug message over the sellected debug port
  #endif
  
  static volatile uint32_t lastReadTime = 120000;

  if (millis() - lastReadTime >= (sensorReadInterval * 1000)) {  //  if it has been long enough sience last reading
    lastReadTime = millis();  //  update the time

    #if debug
    Serial.printf("It has been long enough between temp readings, reading the temp...\n");
    #endif

    if (!areSensorsPresent()) {  //  check if any sensors were disconnected (We don't want to waste time if sensors are disconnected)
      return false;  //  go back. areSensorsPreasant() should have set the mode to error
    }

    useI2C(4, 1);
    
    //  wake up I2C temp sensors:
    heaterTempSensor.wake();

    doneWithI2C();

    delayMicroseconds(1);  //  wait for a little bit for the other core to be able to take the mutex

    useI2C(4, 2);

    inTempSensor.wake();

    doneWithI2C();

    delayMicroseconds(1);  //  wait for a little bit for the other core to be able to take the mutex

    useI2C(4, 3);

    outTempSensor.wake();

    doneWithI2C();

    delayMicroseconds(1);  //  wait for a little bit for the other core to be able to take the mutex

    //  make some temporary variables to store the sum of all mesured temps (for avereging)
    int16_t tempHeaterTemp = 0;
    int16_t tempInTemp = 0;
    int16_t tempOutTemp = 0;

    for (uint8_t i = 0; i < sensorReads; i++) {  //  do the stuff in this loop the number of times set by sensorReads
      useI2C(4);

      int16_t tHeaterTemp = static_cast<int16_t>(heaterTempSensor.readTempC());  //  get the temperature from the sensor

      doneWithI2C();

      if (tHeaterTemp <= minTemp || tHeaterTemp >= maxHeaterTemp) {  //  if it is not within the acceptable range
        mode = ERROR;  //  set the mode to error
        errorOrigin = 1;  //  record what caused the error

        //  record both the sensor that caused the error, and the value it read, all in one variable
        errorInfo = 1;  //  record the sensor that triggered the error
        errorInfo <<= 16;  //  move that info 8 bits to the left (errorInfo is a 32-bit signed integer)
        errorInfo += static_cast<uint8_t>(tHeaterTemp);  //  add an aproxamation of the read value to errorInfo

        return false;

      } else {  //  if it is withing the acceptable range
        tempHeaterTemp += tHeaterTemp;  //  add the reading to a avriable so it can be averaged
      }

      useI2C(4);

      int16_t tInTemp = static_cast<int16_t>(inTempSensor.readTempC());  //  get the temperature from the sensor

      doneWithI2C();

      if (tInTemp <= minTemp || tInTemp >= maxInOutTemp) {  //  if it is not within the acceptable range
        mode = ERROR;  //  set the mode to error
        errorOrigin = 1;  //  record what caused the error

        //  record both the sensor that caused the error, and the value it read, all in one variable
        errorInfo = 2;  //  record the sensor that triggered the error
        errorInfo <<= 16;  //  move that info 8 bits to the left (errorInfo is a 32-bit signed integer)
        errorInfo += static_cast<uint8_t>(tInTemp);  //  add an aproxamation of the read value to errorInfo

        return false;

      } else {
        tempInTemp += tInTemp;  //  add the reading to a avriable so it can be averaged
      }

      useI2C(4);

      int16_t tOutTemp = static_cast<int16_t>(outTempSensor.readTempC());  //  get the temperature from the sensor

      doneWithI2C();

      if (tOutTemp <= minTemp || tOutTemp >= maxInOutTemp) {  //  if it is not within the acceptable range
        mode = ERROR;  //  set the mode to error
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

    useI2C(4);
    
    //  put the I2C temp sensors to sleep:
    heaterTempSensor.shutdown();
    inTempSensor.shutdown();
    outTempSensor.shutdown();
    
    doneWithI2C();

    uint8_t offset = sensorReads >> 1; // this is the same as sensorReads / 2, just more eficient. Used because, on average, each individual sensor reading is low by 0.5 degrees

    heaterTemp = (tempHeaterTemp / sensorReads) + offset;  //  set the temperature to the temporary variable devided by the number of times the temperature was read
    inTemp = (tempInTemp / sensorReads) + offset;  //  set the temperature to the temporary variable devided by the number of times the temperature was read
    outTemp = (tempOutTemp / sensorReads) + offset;  //  set the temperature to the temporary variable devided by the number of times the temperature was read

    #if debug
    Serial.printf("Heater temp: %d. | In temp: %d. | Out temp: %d.\n", heaterTemp, inTemp, outTemp);  //  print a debug message over the sellected debug port
    #endif

    return true;

  } else {  //  if it hasn't been long enough sience the last temp reading
    #if debug
    Serial.printf("It hasn't been long enough between temp readings, not reading the temp.\n");
    #endif

    return true;
  }  //  else (  if (millis() - lastReadTime >= sensorReadInterval)  )
}

void lightswitchPressed() {
  #if debug
  uint8_t core = rp2040.cpuid();
  Serial.printf("lightswitchPressed() called from core%u.\n", core);  //  print a debug message over the sellected debug port
  #endif

  turnLightOff = false;

  lightSetState = !lightSetState;  //  change the state of the lights in logic

  mainLight.setState(lightSetState);
}

void manualCooldown() {
  #if debug
  uint8_t core = rp2040.cpuid();
  Serial.printf("manualCooldown() called from core%u.\n", core);  //  print a debug message over the sellected debug port
  #endif

  mode = COOLDOWN;  //  set the mode to cooldown
}

void doorOpening() {
  #if debug
  uint8_t core = rp2040.cpuid();
  Serial.printf("doorOpening() called from core%u.\n", core);  //  print a debug message over the sellected debug port
  #endif

  doorOpen = true;  //  remember the door is open
  printDone = false;  //  the print might have been removed; remember that the print is not done

  if (lights_On_On_Door_Open && !mainLight.getState()) {  //  if the lights should change on door open
    turnLightOff = true;
    lightSetState = true;
    mainLight.setState(true);  //  turn on the lights
  }

  if (mode != PRINTING && printName[0] != 0) {  //  if the mode is not printing and the print name isn't empty (well, technically if the first character of the print name isn'y NULL)
    #if debug
    Serial.printf("clearing the print name.\n");  //  print a debug message over the sellected debug port
    #endif

    for (uint16_t i = 0; i < 256; i++) {  //  go through all the characters in the print name
      printName[i] = 0;  //  and set them to NULL
    }  //  for (uint8_t i = 0; i < 256; i++)
  }  //  if (mode != 3)

  setHeaters(false, false, 0);  //  turn off both heaters and the fan

  #if debug
  Serial.printf("Door open.\n");  //  print a debug message over the sellected debug port
  #endif
}

void doorClosing() {
  #if debug
  uint8_t core = rp2040.cpuid();
  Serial.printf("doorClosing() called from core%u.\n", core);  //  print a debug message over the sellected debug port
  #endif

  if (turnLightOff) {  //  if the lights should turn off
    lightSetState = false;
    mainLight.setState(false);
  }

  doorOpen = false;  //  remember that the door is closed
  printDone = false;  //  the print might have been removed; remember that the print is not done

  #if debug
  Serial.printf("Door closed.\n");  //  print a debug message over the sellected debug port
  #endif
}

void checkButtons() {
  #if debug
  uint8_t core = rp2040.cpuid();
  Serial.printf("checkButtons() called from core%u.\n", core);  //  print a debug message over the sellected debug port
  #endif

  bool input = false; // tracks if user input was detected

  //  update the buttons
  door_switch.update();
  light_switch.update();
  coolDown_switch.update();

  if (door_switch.pressed()) {
    doorClosing();
    input = true;

  } else if (door_switch.released()) {
    doorOpening();
    input = true;
  }

  if (light_switch.released()) {
    #if debug
    Serial.printf("Light switch pressed.\n");  //  print a debug message over the sellected debug port
    #endif

    lightswitchPressed();

    input = true;
  }

  static bool cdSwitchWasPressed = false;  //  this variable makes it so that cooldown is triggered only once per button press and hold

  if (coolDown_switch.isPressed()) {  //  if the cooldown switch was just relesed and was previusly held for over a set length of time:
    if ((coolDown_switch.currentDuration() >= cooldownSwitchHoldTime) && !cdSwitchWasPressed) {
      #if debug
      Serial.printf("Cooldown switch pressed.\n");  //  print a debug message over the sellected debug port
      #endif

      cdSwitchWasPressed = true;

      manualCooldown();  //  set mode to cooldown
    }

    input = true;

  } else {
    cdSwitchWasPressed = false;
  }

  if (input) {
    lastUserInput = core1Time;
  }
}
