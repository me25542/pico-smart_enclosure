/*
 * Copyright (c) 2024-2025 Dalen Hardy
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

#include "modeFuncs.hpp"
#include "otherFuncs.hpp"

void standard() {
  static bool rebootForgoten = false; // tracks if the reason for reboot is erased
  static timers::Timer bootselTimer;

  // reset the core 0 (software) watchdog timer
  core0WatchdogTimer.set(WATCHDOG_TIME);

  if (bootselTimer.isDone() || !bootselTimer.isSet()) {
    bootselTimer.set(BOOTSEL_UPDATE_TIME - bootselTimer.overdone());
    bootsel = BOOTSEL; // update if the BOOTSEL button is pressed
  }

  if (!rebootForgoten && millis() > ERROR_FIX_INTERVAL) {
    rebootForgoten = true;
    if (static_cast<bool>(EEPROM.read(23))) {
      EEPROM.write(23, 0);
      EEPROM.commit();
    }
  }

  if (core1WatchdogTimer.isDone()) setError(11, 1, false); // set mode to error if the core 1 (software) watchdog timer ran out
  if (controlMode == CONTROL_MODE_TEMP && oldControlMode == CONTROL_MODE_MANUAL) globalSetTemp = DEFAULT_SET_TEMP;

  // todo: fix
  mainMenu[1]->setIsEditable(!(controlMode == CONTROL_MODE_MANUAL)); // SCARY! BAD! FIND A BETTER WAY TO DO THIS! - self

  blinkLED(); // handle blinking the LED to indicate status
  updateServos(); // handle servo movement
  updateFan(); // handle fan speed

  // update lights
  printDoneLight.setSpeed(pdl_DimingTime);
  mainLight.setSpeed(dimingTime);
  printDoneLight.setState(printDone); // set print done light to correct state
  mainLight.setState(lightSetState); // set lights to correct state

  if (changeLights) { // if the light's state needs to change:
    mainLight.changeState();
  }

  #if SERIAL_CONTROL
  serialReceiveEvent(); // check for any commands sent via USB
  #endif

  if (checkI2c) {
    parseI2C();
  }

  periodicMenuBackup(); // backup menu data every <backupInterval> milliseconds

  isScreenConnected(); // check if the screen is connected

  getTemp(); // get temps
}

const void standby() {
  #if DEBUG
  Serial.printf("Doing standby()\n"); // print a debug message over USB
  #endif

  if (oldMode != MODE_STANDBY) {
    maxFanSpeed.store(defaultMaxFanSpeed.load()); // reset the max fan speed
    controlMode = DEFAULT_CONTROL_MODE; // reset the control mode
    globalSetTemp = DEFAULT_SET_TEMP;
  }

  //  turn off the fan and heaters:
  setHeaters(false, false); // turn off both heaters
  setFan(fanOffVal); // turn off the fan

  //  move servos to home position:
  setServos(servo1Closed, servo2Closed); // move servos 1 and 2 to home

  setPSU(mainLight.needsPower() || printDoneLight.needsPower()); // turn on the PSU if the lights are on OR if the PSU shouldn't be turned off OR if the print is done (the print done light is on)

  #if DEBUG
  Serial.printf("Did standby().\n"); // print a debug message over USB
  #endif
}

const void cooldown() {
  #if DEBUG
  Serial.printf("Doing cooldown()...\n"); // print a debug message over USB
  #endif

  maxFanSpeed.store(defaultMaxFanSpeed.load()); // reset the max fan speed

  if ((inTemp - cooldownDif) > outTemp) { // if the temp inside is much greater than the temp outside:
    // ensure the fan and the lights have power if they need it:
    setPSU(!doorOpen || mainLight.needsPower() || printDoneLight.needsPower()); // if the door is closed OR if the lights are on OR the PSU shouldn't be turned off OR if the print is done (print done light on), turn on the PSU. otherwise, turn it off

    // set the heaters and fan:
    setHeaters(false, false); // turn off heaters
    setFan(fanOnVal); // turn on fan if door closed

  } else {
    mode = MODE_STANDBY; // set the mode to standby
  }

  #if DEBUG
  Serial.printf("Did cooldown().\n"); // print a debug message over USB
  #endif
}

const void printing() {
  #if DEBUG
  Serial.printf("printing() called.\n"); // print a debug message over USB
  #endif

  switch (controlMode) {
    case CONTROL_MODE_TEMP:
      heatingLogic_temp();
      break; // exit the switch... case statement

    case CONTROL_MODE_MANUAL:
      heatingLogic_manual();
      break; // exit the switch... case statement
    
    default:
      setHeaters(false, false); // turn off heaters
      setFan(fanOffVal); // turn off fan
      break;  // exit the switch... case statement
  }

  #if DEBUG
  Serial.printf("Exiting printing().\n"); // print a debug message over USB
  #endif
}

const void error() {
  static std::atomic<uint8_t> callNumber = 0; // how many times the function has been called
  bool firstCaller = !static_cast<bool>(callNumber.fetch_add(1));
  
  if (!firstCaller) while (true); // loop forever if the other core already called this function

  for (uint32_t checks = 0; (callNumber <= 1 && checks < MAX_OTHER_CORE_SHUTDOWN_TIME); checks++) delay(1); // wait up to two and a half seconds for the other core to call this function
  
  bool otherCoreShutdown = callNumber >= 2;
  
  if (!otherCoreShutdown) { // if the other core didn't call this function
    rp2040.idleOtherCore(); // force the other core to stop
  }

  #if !(DEBUG || SERIAL_CONTROL)
  serialSetup(); // set up USB comunication
  #endif

  uint8_t coreNumber = rp2040.cpuid();
  uint8_t otherCoreNumber = coreNumber ^ 1; // other core is this core with the one's place bit flipped

  if (Serial) { // if a computer is connected via USB
    Serial.printf("A fatal error was detected; the system will enter a safe state and attempt automatic recovery.\nOrigin: %u (%s).\nAdditional info: %ld.\n", errorOrigin, errorCauses[errorOrigin].c_str(), static_cast<int32_t>(errorInfo)); // print a debug message over USB
    
    if (otherCoreShutdown) {
      Serial.printf("Core%u (the other core) acknowledged the error and stopped looping.\n", otherCoreNumber); // print a debug message over USB if a computer is connected

    } else {
      Serial.printf("Core%u (the other core) failed to acknowledge the error, and its shutdown was forced. Proceeding with shutdown operations.\n", otherCoreNumber); // print a debug message over USB if a computer is connected
    }
  }

  Wire.end(); // turn off I2C0
  Wire1.end(); // turn off I2C1
  
  digitalWrite(ERROR_LIGHT_PIN, ERROR_LIGHT_ON); // turn on the error light
  digitalWrite(LED_PIN, LOW); // turn off built-in LED

  gpioDisable(); // set pins do a disconnected, high-impedance state (excludes the error light and built-in LED)

  if (Serial) Serial.printf("The enclosure has entered safe mode.\n"); // print a debug message over USB if a computer is connected

  if (is_error_recoverable && !static_cast<bool>(EEPROM.read(23))) {
    if (Serial) Serial.printf("Error is marked as recoverable; rebooting...\n"); // print a debug message over USB if a computer is connected
    EEPROM.write(23, 255); // remember that recovery was atemted
    EEPROM.commit(); // commit to flash
    rp2040.reboot(); // reboot the Pico

  } else if (Serial) {
    Serial.printf("Error is not recoverable, entering infinite loop.\n"); // print a debug message over USB if a computer is connected
  }

  set_sys_clock_khz(133000, true); // try to lower the system clock to 133 Mhz exactly

  while (true) { // the code in here will loop forever
    delay(10000); // wait for 10 seconds

    blinkErrorCode(errorOrigin); // use the built-in LED to flash out a series of blinks indicating the error origin

    if (Serial) { // if a computer is connected via USB
      Serial.printf("A fatal error was detected by core %u, and could not be recovered.\nOrigin: %u (%s).\nAdditional info: %ld.\n", coreNumber, errorOrigin, errorCauses[errorOrigin].c_str(), static_cast<int32_t>(errorInfo)); // print an error mesage over USB

      if (otherCoreShutdown) {
        Serial.printf("Both cores successfully shut down after the error.\n");

      } else {
        Serial.printf("Core%u successfully shut down after the error, core%u was forced to stop.\n", coreNumber, otherCoreNumber);
      }
    }
  }
}