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

#include "vars.hpp"
#include "otherFuncs.hpp"

#include "modeFuncs.hpp"


// the function called every loop when the mode is standby:
const void standby() {
  #if debug
  printf("Doing standby()\n");  //  print a debug message over the sellected debug port
  #endif

  getTemp();

  maxFanSpeed = defaultMaxFanSpeed;  //  reset the max fan speed

  //  turn off the fan and heaters:
  setHeaters(false, false, fanOffVal);  //  turn off heater 1 and heater 2, turn off the fan

  //  move servos to home position:
  setServos(servo1Closed, servo2Closed);  //  move servos 1 and 2 to home

  setPSU(mainLight.needsPower() ||  printDoneLight.needsPower()); //  turn on the PSU if the lights are on OR if the PSU shouldn't be turned off OR if the print is done (the print done light is on)

  printingLastLoop = false;  //  remember that last loop wasn't in printing mode

  delay(10);  //  wait for 10ms

  #if debug
  printf("Did standby().\n");  //  print a debug message over the sellected debug port
  #endif
}

//  the function called every loop when the mode is cooldown:
const void cooldown() {
  #if debug
  printf("Doing cooldown()...\n");  //  print a debug message over the sellected debug port
  #endif

  maxFanSpeed = defaultMaxFanSpeed;  //  reset the max fan speed

  //  get temps:
  getTemp();

  if ((inTemp - cooldownDif) > outTemp) {  //  if the temp inside is much greater than the temp outside:
    //  ensure the fan and the lights have power if they need it:
    setPSU(!doorOpen || mainLight.needsPower() ||  printDoneLight.needsPower());  //  if the door is closed OR if the lights are on OR the PSU shouldn't be turned off OR if the print is done (print done light on), turn on the PSU. otherwise, turn it off

    //  set the heaters and fan:
    setHeaters(false, false, fanOnVal);  //  turn off heater 1 and heater 2, turn on the fan at 100% if the door is closed

  } else {
    mode = 1;  //  set the mode to standby
  }

  printingLastLoop = false;  //  remember that last loop wasn't in printing mode

  #if debug
  printf("Did cooldown().\n");  //  print a debug message over the sellected debug port
  #endif
}

//  the function called every loop when the mode is printing:
const void printing() {
  #if debug
  printf("Doing printing()...\n");  //  print a debug message over the sellected debug port
  #endif

  //  get temps:
  getTemp();

  //  ensure the fan, heaters, and servos have power:
  setPSU(true);  //  turn on the PSU

  //  make it so that the logic can't be messed up by the printer setting the set temp at an unconviniant time
  uint8_t setTemp = globalSetTemp;

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

  #if debug
  printf("Did printing().\n");  //  print a debug message over the sellected debug port
  #endif
}

//  the function called if an error is detected:
const void error() {
  Serial.printf("error() called.\n");  //  print a debug message over the sellected debug port

  errorDetected = true;  //  tell the other core to shut down (record that error() was called; the other core will check if this is true)

  Serial.printf("Dang it! An error was detected!\n");  //  print a debug message over the sellected debug port

  Serial.printf("%s Listed origin: %u.\nAdditional info: %ld.\n", errorCauses[errorOrigin].c_str(), errorOrigin, errorInfo);  //  print a debug message over the sellected debug port

  for (uint16_t checks = 0; (!coreOneShutdown && checks < maxCoreOneShutdownTime); checks++) {  //  wait up to two and a half seconds for the first core to acknowledge the error and stop looping
    delay(1);  //  wait for one milisecond
  }

  if (rp2040.cpuid() == 0) {
    rp2040.idleOtherCore();  //  stop the other core. if it already stopped, thats great, if not this is better than nothing
  }

  if (coreOneShutdown) {  //  check if the second core shut down
    Serial.printf("The second core has acknowledged the error and stoped looping.\n");  //  print a debug message over the sellected debug port

  } else {
    Serial.printf("The second core has failed to acknowledge the error in the allotted time, and its shutdown was forced. Proceeding with shutdown operations.\n");  //  print a debug message over the sellected debug port
  }

  Wire.end();  //  turn off I2C0
  Wire1.end();  //  turn off I2C1
  
  digitalWrite(errorLightPin, errorLightOn);  //  turn on the error light
  Serial.printf("The error light has been turned on.\n");  //  print a debug message over the sellected debug port

  digitalWrite(lightsPin, LOW);
  digitalWrite(printDoneLightPin, LOW);
  Serial.printf("The lights have been turned off.\n");  //  print a debug message over the sellected debug port

  setServos(servo1Closed, servo2Closed);  //  move servos 1 and 2 to home position
  servo1.detach();
  servo2.detach();
  Serial.printf("The servos have been moved to home position and detached.\n");  //  print a debug message over the sellected debug port

  setHeaters(false, false, 0);  //  turn off both heaters, turn off the fan
  Serial.printf("The heaters and fan have been turned off.\n");  //  print a debug message over the sellected debug port

  setPSU(false);  //  turn off the PSU
  Serial.printf("The PSU has been turned off.\n");  //  print a debug message over the sellected debug port

  coreZeroShutdown = true;

  Serial.printf("The enclosure has entered safe mode.\n");  //  print a debug message over the sellected debug port

  set_sys_clock_khz(96000, true);  //  try to set the system clock to 96 Mhz exactly

  //  make shure the "error()" function will never end:
  while (true) {  //  the code in here will loop forever
    delay(30000);  //  wait for 30 seconds

    errorDetected = true;  //  tell the other core to shut down (record that error() was called; the other core will check if this is true)

    Serial.printf("An error was detected. ");  //  print an error mesage over serial
    if (coreZeroShutdown) {
      Serial.printf("The first core successfully shut down after the error. ");  //  print a debug message over the sellected debug port
    } else {
      Serial.printf("The first core failed to shut down successfully after the error. ");  //  print a debug message over the sellected debug port
    }
    if (coreOneShutdown) {
      Serial.printf("The second core successfully shut down after the error.\n");  //  print a debug message over the sellected debug port
    } else {
      Serial.printf("The second core failed to successfully shut down after the error.\n");  //  print a debug message over the sellected debug port
    }
    Serial.printf("%s Listed origin: %u. Info: %li.\n", errorCauses[errorOrigin].c_str(), errorOrigin, errorInfo);  //  print a debug message over the sellected debug port
  }
}

void coreOneError() {
  #if debug
  printf("coreOneError() called\n");
  #endif

  while (true) {  //  loop forever
    coreOneShutdown = true;  //  report that the second core (core1) has shut down
    delay(30000);  //  wait 30 seconds
  }
}

