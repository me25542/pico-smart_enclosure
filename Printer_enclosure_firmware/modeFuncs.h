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

#ifndef MODEFUNCS_H
#define MODEFUNCS_H

#include "otherFuncs.h"  //  this is just for ease in writing; the include guards will block this from compiling

// the function called every loop when the mode is standby:
void standby() {
  Serial.println("Doing standby()...");  //  print a message over serial (USB)

  //  set the lights:
  setIndicatorLights(false, false, false, true);  //  turn on the "standby" light, turn off the others

  maxFanSpeed = defaultMaxFanSpeed;  //  reset the max fan speed

  //  turn off the fan and heaters:
  setHeaters(false, false, fanOffVal);  //  turn off heater 1 and heater 2, turn off the fan

  //  move servos to home position:
  setServos(servo1Closed, servo2Closed);  //  move servos 1 and 2 to home

  setPSU(lightState || dontTurnOffThePSU || printDone); //  turn on the PSU if the lights are on OR if the PSU shouldn't be turned off OR if the print is done (the print done light is on)

  printingLastLoop = false;  //  remember that last loop wasn't in printing mode

  Serial.println("Did standby()");  //  print a message over serial (USB)
}

//  the function called every loop when the mode is cooldown:
void cooldown() {
  Serial.println("Doing cooldown()...");  //  print a message over serial (USB)

  //  set the lights:
  setIndicatorLights(false, false, true, false);  //  turn on the "cooldown" light, turn off the others

  maxFanSpeed = defaultMaxFanSpeed;  //  reset the max fan speed

  //  get temps:
  getTemp();

  if ((inTemp - cooldownDif) > outTemp) {  //  if the temp inside is much greater than the temp outside:
    //  ensure the fan and the lights have power if they need it:
    setPSU(!doorOpen || lightState || dontTurnOffThePSU || printDone);  //  if the door is closed OR if the lights are on OR the PSU shouldn't be turned off OR if the print is done (print done light on), turn on the PSU. otherwise, turn it off

    //  set the heaters and fan:
    setHeaters(false, false, fanOnVal);  //  turn off heater 1 and heater 2, turn on the fan at 100% if the door is closed

  } else {
    mode = 1;  //  set the mode to standby
  }

  printingLastLoop = false;  //  remember that last loop wasn't in printing mode

  Serial.println("Did cooldown()");  //  print a message over serial (USB)
}

//  the function called every loop when the mode is printing:
void printing() {
  Serial.println("Doing printing()...");  //  print a message over serial (USB)

  //  get temps:
  getTemp();

  //  set the lights:
  setIndicatorLights(false, true, false, false);  //  turn on the "printing" light, turn off the others

  //  ensure the fan, heaters, and servos have power:
  setPSU(true);  //  turn on the PSU

  //  make it so that the logic can't be messed up by the printer setting the set temp at an unconviniant time
  int setTemp = globalSetTemp;

  if (! printingLastLoop) {  //  if printing() wasn't called last loop:
    if (inTemp > setTemp) {  //  if the inside is hoter than it should be:
      heatingMode = false;  //  set the heating mode to cooling (heatingMode tracks if the default should be heating or cooling)
      
    } else {  //  if the inside is colder than it should be:
      heatingMode = true;  //  set the heating mode to heating (heatingMode tracks if the default should be heating or cooling)
    }
  }

  if (heatingMode) {  //  if we think that we should be heating the enclosure:
    if ((inTemp + bigDiff) < setTemp) {  //  if the inside temp is much less than it should be:
      setHeaters(true, false, fanOffVal);

    } else if ((inTemp - bigDiff) > setTemp) {  //  if the inside temp is much higher than it shuld be:
      heatingMode = false;
      setHeaters(false, false, fanMidVal);

    } else if (inTemp > setTemp) {  //  if the inside temp is higher than it should be:
      setHeaters(false, false, fanOffVal);

    }

  } else {  //  if we think we should be cooling the enclosure:
    if ((inTemp - bigDiff) > setTemp) {  //  if the inside temp is much higher than it should be:
      setHeaters(false, false, fanOnVal);

    } else if ((inTemp + bigDiff) < setTemp) {  //  if the inside temp is much less than it should be:
      heatingMode = true;
      setHeaters(true, false, fanOffVal);

    } else if (inTemp < setTemp) {  //  if the inside temp is less than it should be:
      setHeaters(false, false, fanOffVal);

    }
  }

  printingLastLoop = true;  //  remember that last loop was in printing mode

  Serial.println("Did printing()");  //  print a message over serial (USB)
}

//  the function called if an error is detected:
void error() {
  Serial.println("Doing error()");  //  print a message over serial (USB)

  errorDetected = true;  //  tell the other core to shut down (record that error() was called; the other core will check if this is true)

  Serial.println("Dang it! An error was detected!");  //  print an error message

  switch (errorOrigin) {  //  check the origin of the error
    case 1:  //  if the origin is 1:
      Serial.print("Origin: Temp sensor check.");  //  print a message over serial (USB)
      break;  //  exit the switch... case statement

    case 2:  //  if the origin is 2:
      Serial.print("Origin: Unrecognised mode.");  //  print a message over serial (USB)
      break;  //  exit the switch... case statement

    case 3:  //  if the origin is 3:
      Serial.print("Origin: Failure to start temp sensors.");  //  print a message over serial (USB)
      break;  //  exit the switch... case statement

    case 4:  //  if the origin is 4:
      Serial.print("Origin: Printer commanded error mode.");  //  print a message over serial (USB)
      break;  //  exit the switch... case statement

    case 5:  //  if the origin is 5:
      Serial.print("Origin: Invalid command received.");  //  print a message over serial (USB)
      break;  //  exit the switch... case statement

    case 6:
      Serial.print("Origin: Temp sensor disconnected.");  //  print a message over serial (USB)
      break;  //  exit the switch... case statement

    case 7:
      Serial.print("Origin: serial commanded error.");  //  print a message over serial (USB)
      break;  //  exit the switch... case statement

    case 8:
      Serial.print("Origin: invalid serial command.");  //  print a message over serial (USB)
      break;  //  exit the switch... case statement

    default:  //  if the origin is anything else:
      Serial.print("Unknown / invalid origin.");  //  print a message over serial (USB)
      Serial.print(" Listed origin: ");  //  print a message over serial (USB)
      Serial.print(errorOrigin);  //  print a message over serial (USB)
      Serial.print(".");  //  print a message over serial (USB)
      break;  //  exit the switch... case statement
  }

  Serial.print(" additional info: ");  //  print a message over serial (USB)
  Serial.print(errorInfo);  //  print any extra info about the error
  Serial.println(".");  //  print a message over serial (USB)

  for (int checks = 0; (!coreOneShutdown && checks < maxCoreOneShutdownTime); checks++) {  //  wait up to one second for the first core to acknowledge the error and stop looping
    delay(1);  //  wait for one milisecond
  }
  if (coreOneShutdown) {  //  check if the second core shut down
    Serial.println("The second core has acknowledged the error and stoped looping.");  //  print a message over serial (USB)
  } else {
    Serial.println("The second core has failed to acknowledge the error in the allotted time. Proceeding with shutdown operations.");  //  print a message over serial (USB)
  }

  Wire.end();  //  turn off I2C0
  Wire1.end();  //  turn off I2C1
  
  setIndicatorLights(true, false, false, false);  //  turn on the error light, turn off the others
  Serial.println("The indicator lights have been set.");

  setServos(servo1Closed, servo2Closed);  //  move servos 1 and 2 to home position
  servo1.detach();
  servo2.detach();
  Serial.println("The servos have been moved to home position and detached.");  //  print a message over serial (USB)

  setHeaters(false, false, 0);  //  turn off both heaters, turn off the fan
  Serial.println("The heaters and fan have been turned off.");  //  print a message over serial (USB)

  setPSU(false);  //  turn off the PSU
  Serial.println("The PSU has been turned off.");  //  print a message over serial (USB)

  coreZeroShutdown = true;

  Serial.println("The enclosure has entered safe mode");  //  print a message over serial (USB)

  //  make shure the "error()" function will never end:
  while (true) {  //  the code in here will loop forever
    delay(30000);  //  wait for 30 seconds

    errorDetected = true;  //  tell the other core to shut down (record that error() was called; the other core will check if this is true)

    Serial.print("An error was detected. ");  //  print an error mesage over serial
    if (coreZeroShutdown) {
      Serial.print("The first core successfully shut down after the error. ");  //  print a message over serial (USB)
    } else {
      Serial.print("The first core failed to shut down successfully after the error. ");  //  print a message over serial (USB)
    }
    if (coreOneShutdown) {
      Serial.print("The second core successfully shut down after the error. ");  //  print a message over serial (USB)
    } else {
      Serial.print("The second core failed to successfully shut down after the error. ");  //  print a message over serial (USB)
    }
    Serial.print("Origin: ");  //  print a message over serial (USB)
    Serial.print(errorOrigin);  //  print a message over serial (USB)
    Serial.print(". Info: ");  //  print a message over serial (USB)
    Serial.print(errorInfo);  //  print a message over serial (USB)
    Serial.println(".");  //  print a message over serial (USB)
  }
}

#endif