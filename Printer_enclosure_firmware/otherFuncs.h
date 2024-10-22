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

#ifndef OTHERFUNCS_H
#define OTHERFUNCS_H

//  a simple function to blink the built-in LED
void blinkLED(unsigned long blinkTime) {
  Serial.print("blinkLED(");  //  print a message over serial (USB)
  Serial.print(blinkTime);  //  print a message over serial (USB)
  Serial.println(") called");  //  print a message over serial (USB)
  digitalWrite(ledPin, HIGH);  //  turn on the built-in LED
  delay(blinkTime);  //  wait 1 second
  digitalWrite(ledPin, LOW);  //  turn off the built-in LED
}

//  the function that checks if a given I2C device is connected
bool isI2CDeviceConnected(int address) {
  Serial.print("isI2CDeiceConnected(");  //  print a message over serial (USB)
  Serial.print(address);  //  print a message over serial (USB)
  Serial.println(") called");  //  print a message over serial (USB)
  Wire.beginTransmission(address);  //  starts a transmition to the specified device
  int I2CError = Wire.endTransmission(true);  //  record the response from ending the transmition and releasing the bus
  Serial.print("isI2CDeviceConnected returned ");
  Serial.println(I2CError == 0);
  return (I2CError == 0);  //  this will make the function return true if there were no errors, false otherwise
}

//  the function that checks if all sensors are connected
bool areSensorsPresent() {
  Serial.println("areSensorsPreasent() called");  //  print a message over serial (USB)
  bool sensorNotConnected = false;
  if (! isI2CDeviceConnected(heaterTempSensorAdress)) {
    mode = 0;  //  set the mode to error
    errorOrigin = 6;  //  record what caused the error
    errorInfo = 1;  //  record which sensor is at fault
    sensorNotConnected = true;
  }
  if (! isI2CDeviceConnected(inTempSensorAdress)) {
    mode = 0;  //  set the mode to error
    errorOrigin = 6;  //  record what caused the error
    errorInfo = 2;  //  record which sensor is at fault
    sensorNotConnected = true;
  }
  if (! isI2CDeviceConnected(outTempSensorAdress)) {
    mode = 0;  //  set the mode to error
    errorOrigin = 6;  //  record what caused the error
    errorInfo = 3;  //  record which sensor is at fault
    sensorNotConnected = true;
  }
  Serial.print("areSensorsPreasent() returned ");
  Serial.println(! sensorNotConnected);
  return ! sensorNotConnected;
}


//  the function to set the state of the PSU (on or off)
void setPSU(bool state) {
  Serial.print("setPSU(");  //  print a message over serial (USB)
  Serial.print(state);  //  print a message over serial (USB)
  Serial.println(") called");  //  print a message over serial (USB)
  digitalWrite(psPin, state);  //  set the power state of the PSU
  while (digitalRead(pokPin) != state);  //  wait for the PSU to report the power state is the same as requested
}

//  the function to set the position of the servos
void setServos(int s1_Pos, int s2_Pos) {
  Serial.print("setServos(");  //  print a message over serial (USB)
  Serial.print(s1_Pos);  //  print a message over serial (USB)
  Serial.print(", ");  //  print a message over serial (USB)
  Serial.print(s2_Pos);  //  print a message over serial (USB)
  Serial.println(") called");  //  print a message over serial (USB)
  servo1.write(s1_Pos);  //  set the position of servo1
  servo2.write(s2_Pos);  //  set the position of servo2
  if ((s1_Pos != oldS1_Pos) || (s2_Pos != oldS2_Pos)) {  //  if the servos are actually being set to something they wern't before:
    delay(500);  //  wait half a second for the servos to actually move
  }
  oldS1_Pos = s1_Pos;  //  record the state of the servos
  oldS2_Pos = s2_Pos;  //  record the state of the servos
}

//  the function to set the state of booth heaters, and the fan (the temp-related stuff)
void setHeaters(bool h1_On, bool h2_On, int fanVal) {
  Serial.print("setHeaters(");  //  print a message over serial (USB)
  Serial.print(h1_On);  //  print a message over serial (USB)
  Serial.print(", ");  //  print a message over serial (USB)
  Serial.print(h2_On);  //  print a message over serial (USB)
  Serial.print(", ");  //  print a message over serial (USB)
  Serial.print(fanVal);  //  print a message over serial (USB)
  Serial.println(") called");  //  print a message over serial (USB)

  if (doorOpen) {
    Serial.println("setHeaters() will turn everything off; the door is open");  //  print a message over serial (USB)
    h1_On = false;
    h2_On = false;
    fanVal = 0;
  } else {
    Serial.println("setHeaters() will do as requested; the door is closed");  //  print a message over serial (USB)
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

//  the function to set the state of the mode indicator lights
void setIndicatorLights(bool errorLight_On, bool printingLight_On, bool cooldownLight_On, bool standbyLight_On) {
  Serial.print("setIndicatorLights(");  //  print a message over serial (USB)
  Serial.print(errorLight_On);  //  print a message over serial (USB)
  Serial.print(", ");  //  print a message over serial (USB)
  Serial.print(printingLight_On);  //  print a message over serial (USB)
  Serial.print(", ");  //  print a message over serial (USB)
  Serial.print(cooldownLight_On);  //  print a message over serial (USB)
  Serial.print(", ");  //  print a message over serial (USB)
  Serial.print(standbyLight_On);  //  print a message over serial (USB)
  Serial.println(") called");  //  print a message over serial (USB)
  digitalWrite(errorLightPin, errorLight_On);  //  set the error light
  digitalWrite(printingLightPin, printingLight_On);  //  set the printing light
  digitalWrite(cooldownLightPin, cooldownLight_On);  //  set the cooldown light
  digitalWrite(standbyLightPin, standbyLight_On);  //  set the standby light
}

//  the function to set the print done light
void setPrintDoneLight(bool pdl_State) {
  Serial.print("setPrintDoneLight(");  //  print a message over serial (USB)
  Serial.print(pdl_State);  //  print a message over serial (USB)
  Serial.println(") called");  //  print a message over serial (USB)
  digitalWrite(printDoneLightPin, pdl_State);  //  set the state of the print done light
}

//  the function that starts serial (USB) comunication (takes ~10ms) | returns 1 if a computer is connected, 0 if not
bool startSerial() {
  Serial.begin(serialSpeed);  //  start serial (over USB) with a baud rate of 115200
  return Serial;  //  send back to the calling function if serial is connected
}

// the function that gets all temp sensor data | returns 0 on failure, 1 on sucess
bool getTemp() {
  Serial.print("getTemp() called");  //  print a message over serial (USB)

  //  wake up I2C temp sensors:
  heaterTempSensor.wake();
  inTempSensor.wake();
  outTempSensor.wake();

  //  make some temporary variables to store the sum of all mesured temps (for avereging)
  float tempHeaterTemp = 0;
  float tempInTemp = 0;
  float tempOutTemp = 0;

  for (int i = 0; i < sensorReads; i++) {  //  do the stuff in this loop the number of times set by sensorReads
    heaterTemps[i] = heaterTempSensor.readTempC();  //  store the mesured temperature
    tempHeaterTemp += heaterTemps[i];  //  add the messured temperature to the temporary variable
    inTemps[i] = inTempSensor.readTempC();  //  store the mesured temperature
    tempInTemp += inTemps[i];  //  add the messured temperature to the temporary variable
    outTemps[i] = outTempSensor.readTempC();  //  store the mesured temperature
    tempOutTemp += outTemps[i];  //  add the messured temperature to the temporary variable
  }

  heaterTemp = (tempHeaterTemp / sensorReads);  //  set the temperature to the temporary variable devided by the number of times the temperature was read
  inTemp = (tempInTemp / sensorReads);  //  set the temperature to the temporary variable devided by the number of times the temperature was read
  outTemp = (tempOutTemp / sensorReads);  //  set the temperature to the temporary variable devided by the number of times the temperature was read

  //  check individual sensor readings for reasonability (if all these are acceptable, the average will also be, making checking the average redundant):
  for (int i = 0; i < sensorReads; i++) {
    if ((heaterTemps[i] <= minTemp || inTemps[i] <= minTemp || outTemps[i] <= minTemp) || (heaterTemps[i] >= maxHeaterTemp || inTemps[i] >= maxInOutTemp || outTemps[i] >= maxInOutTemp)) {
      mode = 0;  //  set mode to error
      errorOrigin = 1;  //  record where the error came from
      Serial.println("error with temp sensor data detected in getTemp()");
      return 0;  //  go to the start of the main loop
    }
  }

  //  put the I2C temp sensors to sleep:
  heaterTempSensor.shutdown_wake(1);
  inTempSensor.shutdown_wake(1);
  outTempSensor.shutdown_wake(1);

  Serial.print("heater temp: ");  //  print a message over serial (USB)
  Serial.print(heaterTemp);  //  print a message over serial (USB)
  Serial.print(" | in temp: ");  //  print a message over serial (USB)
  Serial.print(inTemp);  //  print a message over serial (USB)
  Serial.print(" | out temp: ");  //  print a message over serial (USB)
  Serial.println(outTemp);  //  print a message over serial (USB)
  return 1;
}

//  the function to set the value of the lights (turn the lights on or off)
void setLights(bool on) {
  Serial.print("setLights(");  //  print a message over serial (USB)
  Serial.print(on);  //  print a message over serial (USB)
  Serial.println(") called");  //  print a message over serial (USB)
  if (lightState != on) {  //  check if the state of the lights matches their target state. in this case, "on" is a variable of the type "bool"
    dontTurnOffThePSU = true;  //  make shure there will be power through the whole diming / brightening process

    if (on) {
      setPSU(true);  //  turn on the PSU

      for (int lightVal = 0; lightVal < 255; lightVal++) {  //  while "lightVal" (which starts at 0) is less than 255, run the enclosed code and add one to "lightVal"
        analogWrite(lightsPin, lightVal);  //  set the lights to a brightness cooldownLightOntrolled by "lightVal"
        delay(dimingTime);  //  wait 2 miliseconds
      }

      lightState = true;  //  record that the lights are on

    } else {
      for (int lightVal = 255; lightVal > 0; lightVal--) {  //  while "lightVal" (which starts at 255) is greater than 0, run the enclosed code and subtract one from "lightVal"
        analogWrite(lightsPin, lightVal);  //  set the lights to a brightness cooldownLightOntrolled by "lightVal"
        delay(dimingTime);  //  wait 2 miliseconds
      }

      lightState = false;  //  record that the lights are off
    }
  }
  
  dontTurnOffThePSU = false;  //  let the other core turn off the PSU again
  lightSetState = lightState;  //  make shure the target state of the lights is updated if the light switch was pressed or the printer requested a change
}

//  a function to change the lights value (turn them off if on and vice versa). called on light switch relese
void lightChange() {
  Serial.println("lightChange() called");  //  print a message over serial (USB)
  setLights(! lightState);  //  print a message over serial (USB)
  changeLights = false;  //  disable the flag to change the light's state
}

//  the functoin called when a manual cooldown is triggered (the cooldown button is released):
void manualCooldown() {
  Serial.println("manualCooldown() called");  //  print a message over serial (USB)
  mode = 2;  //  set the mode to cooldown
}

//  the function called when the door opens (the door switch is released):
void doorOpening() {
  doorOpen = true;  //  remember the door is open
  printDone = false;  //  the print might have been removed; remember that the print is not done
  setHeaters(false, false, 0);  //  turn off both heaters and the fan
  Serial.println("door open");  //  print a message over serial (USB)
}

//  the function called when the door closes (the door switch is pressed):
void doorClosing() {
  doorOpen = false;  //  remember that the door is closed
  Serial.println("door closed");  //  print a message over serial (USB)
}

//  checks if the buttons have been pressed or relesed, and calls the correct functions based on the state of the buttons
void checkButtons() {
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
    Serial.println("light switch pressed");  //  print a message over serial (USB)
    lightChange();  //  Change the light state
  }

  if (coolDown_switch.released() && coolDown_switch.previousDuration() >= cooldownSwitchHoldTime) {  //  if the cooldown switch was just relesed and was previusly held for over a set length of time:
    Serial.println("cooldown switch pressed");  //  print a message over serial (USB)
    manualCooldown();  //  set mode to cooldown
  }
}

#endif