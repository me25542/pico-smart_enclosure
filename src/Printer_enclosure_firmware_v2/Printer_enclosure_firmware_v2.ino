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

#include <Arduino.h>
#include "constants.hpp"
#include "config.hpp" // include configuration variables
#include "customLibs.hpp" // include custom helper classes (libraries)
#include "vars.hpp" // include global variables
#include "menuFuncs.hpp" // include the functions used for the menu (procesing button input, drawing the menu on the screen, etc.)
#include "otherFuncs.hpp" // Include the other functions
#include "ISRs.hpp" //  Include ISRs (Interrupt Service Routines)
#include "modeFuncs.hpp" // Include functions called based on the enclosure mode
#include "setup.hpp" // include the setup functions

//********************************************************************************************************************************************************************************

// the setup code for core0
void setup() {
  varInit();
  
  #if debug || SERIAL_CONTROL // only start serial if debuging or serial controll is enabled
  serialSetup();
  while (!Serial); // wait for a USB Host to be connected
  #endif

  core1StartStartup = true; // tell the other core to start startup

  pinSetup();

  if (!setPSU(true)) startupError = true; // try turning on the PSU

  servoSetup();

  if (!tempSensorSetup()) startupError = true; // try turning on the temp sensors

  printerI2cSetup();

  attachInterrupt(digitalPinToInterrupt(POWER_OK_PIN), losingPower, FALLING); // power loss interrupt

  #if debug
  Serial.printf("waiting for core1 to start up...\n"); // print a debug message over USB
  #endif

  while (!coreOneStartup) delayMicroseconds(1); // wait for the second core to start up

  #if debug
  Serial.printf("done waiting for core1 to start up.\n"); // print a debug message over USB
  #endif

  if (startupError) {
    #if debug
    Serial.printf("Startup failed.\n"); // print a debug message over USB
    #endif
    error();

  } else {
    #if debug
    Serial.printf("Startup sucessful.\n"); // print a debug message over USB
    #endif
    coreZeroStartup = true; // tell the other core that we have finished starting up

    digitalWrite(LED_PIN, LOW), delay(1000); // blink LED to show startup complete
    digitalWrite(LED_PIN, HIGH), delay(1000); // same thing this line
  }
}

// the setup code for core1
void setup1() {
  while (!core1StartStartup); // wait for the other core to tell us to start up

  if (menuSetup()) { // try seting up the menu
    backupRecovery(); // initialize the menu with the data stored in the simulated EEPROM, if aplicable

    buttonSetup(); // set up the buttons

  } else { // if it failed
    startupError = true; // if it failed we didn't suceed
  }

  #if debug
  Serial.printf("waiting for core0 to be done startup...\n"); // print a debug message over USB
  #endif

  do { // do this at least once:
    coreOneStartup = true; // tell the other core that we have finished starting up

    if (startupError) { // if there was an error starting up in either core
      error();
    }

    delayMicroseconds(1); // wait for a tiny bit

  } while (! coreZeroStartup); // and then repeat it while the first core hasn't started up

  #if debug
  Serial.printf("done waiting for core0 to be done startup.\n"); // print a debug message over USB
  #endif
}

//********************************************************************************************************************************************************************************

// the loop code for core0
void loop() {
  #if debug
  Serial.printf("Core 0 start of loop.\n"); // print a debug message over the sellected debug port
  #endif

  const static void (*modeFuncs[])(void) = {error, standby, cooldown, printing}; // an array of functions to call based on the mode | this will only be made the first time through the loop
  constexpr static uint8_t modeFuncCount = sizeof(modeFuncs) / sizeof(modeFuncs[0]); // find the number of mode functions (at compile, not during runtime)
  
  if (mode >= modeFuncCount) setError(2, mode, false); // enter error mode if the mode is invalid
  if (mode != MODE_ERROR) standard(); // do the stuff needed every loop, regardless of mode (if the mode isn't error)

  modeFuncs[mode](); // call the mode function coresponding to the mode

  oldMode = mode; // update the variable tracking the old mode
  oldControlMode = controlMode; // update the variable tracking the old control mode
}

// the loop code for core1
void loop1() {
  #if debug
  Serial.printf("Core 1 start of loop.\n"); // print a debug message over the sellected debug port
  #endif

  if (mode == MODE_ERROR) error();

  core1Time = millis();

  core1WatchdogTimer.set(WATCHDOG_TIME); // reset the core 1 (software) watchdog timer

  if (core0WatchdogTimer.isDone()) setError(11, 0, false); // set mode to error if the core 0 (software) watchdog timer ran out

  checkButtons(); // update the state of the switches

  checkMenuButtons(); // update the state of the menu navigation buttons

  updateScreen(); // update the screen

  printDoneLight.tick(); // update the print done light
  mainLight.tick(); // update the main lights
}
