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

#define debug

//********************************************************************************************************************************************************************************

#include <EEPROM.h>

//********************************************************************************************************************************************************************************

#include "vars.h"  //  include all global variables
#include "menuFuncs.h"  //  include all the functions used for the menu (procesing button input, drawing the menu on the screen, etc.)
#include "otherFuncs.h"  //  Include all the other functions
#include "ISRs.h" //  Include all ISRs (Interrupt Service Routines)
#include "modeFuncs.h"  //  Include the functions called based on the enclosure mode
#include "setup.h"  //  include all the functions with the setup code

//********************************************************************************************************************************************************************************

//  the setup code for core 1
void setup() {
  //  this code runs in serial, before the other core does it's startup code | put anything that can't hapen at the same time as other code here
  #ifdef debug
  printf("seting up serial...\n");
  #endif

  serialSetup();

  #ifdef debug
  printf("done seting up serial.\n");
  #endif

  mutex_init(&I2C_mutex);  //  initialize the mutex protecting I2C0

  for (uint16_t i = 0; i < maxSerialStartupTries && !Serial; i++);  //  wait for a computer to be connected, but only for a bit

  core1StartStartup = true;  //  tell the other core to start startup

  //  put the rest of your startup code here, if it can hapen while the other core runs it's startup code

  #ifdef debug
  printf("seting up pins...\n");
  #endif

  pinSetup();

  #ifdef debug
  printf("done seting up pins.\n");
  #endif

  #ifdef debug
  printf("seting up servos...\n");
  #endif

  servoSetup();

  #ifdef debug
  printf("done seting up servos.\n");
  #endif

  #ifdef debug
  printf("seting up the PSU...\n");
  #endif

  if (! setPSU(true)) {  //  try turning on the PSU
    startupError = true;  //  if it failed, we didin't suceed
  }

  #ifdef debug
  printf("done seting up the PSU.\n");
  #endif

  #ifdef debug
  printf("seting up the temp sensors...\n");
  #endif

  if (! tempSensorSetup()) {  //  try turning on the temp sensors
    startupError = true;  //  if it failed, we didin't suceed
  }

  #ifdef debug
  printf("done seting up the temp sensors.\n");
  #endif

  #ifdef debug
  printf("waiting for core1 to start up...\n");
  #endif

  while (! coreOneStartup) {  //  while the second core hasn't started up
    delay(1);  //  wait for a tiny bit
  }

  #ifdef debug
  printf("done waiting for core1 to start up.\n");
  #endif

  if (startupError) {
    printf("Startup failed.\n");
    error();

  } else {
    //  print a message over serial (USB)
    printf("Startup sucessful.\n");
    coreZeroStartup = true;  //  tell the other core that we have finished starting up
    //  blink LED to show startup complete:
    blinkLED(1000);
  }
}

//  the setup code for core 2
void setup1() {
  while (!core1StartStartup);  //  wait for the other core to tell us to start up

  #ifdef debug
  printf("starting the eeprom...\n");
  #endif

  EEPROM.begin(2048);  //  1029 because we have 1kb (1024b) for data, and 5b for version info and if to use the data

  #ifdef debug
  printf("done starting the eeprom.\n");
  #endif

  #ifdef debug
  printf("attaching the power loss interrupt...\n");
  #endif

  attachInterrupt(digitalPinToInterrupt(pokPin), losingPower, FALLING);

  #ifdef debug
  printf("done ataching the power loss interrupt.\n");
  #endif

  #ifdef debug
  printf("seting up the buttons...\n");
  #endif

  buttonSetup();

  #ifdef debug
  printf("done seting up the buttons.\n");
  #endif

  #ifdef debug
  printf("seting up the printer I2C...\n");
  #endif

  printerI2cSetup();

  #ifdef debug
  printf("done seting up the printer I2C.\n");
  #endif

  #ifdef debug
  printf("trying to set up the menu...\n");
  #endif

  if (menuSetup()) {  //  try seting up the menu
    backupRecovery();  //  initialize the menu with the data stored in the simulated EEPROM, if aplicable

  } else {  //  if it failed
    startupError = true;  //  if it failed we didn't suceed
  }

  #ifdef debug
  printf("done trying to set up the menu.\n");
  #endif

  #ifdef debug
  printf("waiting for core0 to be done startup...\n");
  #endif

  do {  //  do this at least once:
    coreOneStartup = true;  //  tell the other core that we have finished starting up

    if (startupError) {  //  if there was an error starting up in either core
      coreOneError();
    }

    delay(1);  //  wait for a tiny bit

  } while (! coreZeroStartup);  //  and then repeat it while the first core hasn't started up

  #ifdef debug
  printf("done waiting for core0 to be done startup.\n");
  #endif
}

//********************************************************************************************************************************************************************************

//  the loop code for core 1
void loop() {
  #ifdef debug
  printf("core 0 loop\n");  //  print a debug message over the sellected debug port
  #endif

  const static void (*modeFuncs[])(void) = {error, standby, cooldown, printing};  //  an array of functions to call based on the mode | this will only be made the first time through the loop
  const static uint8_t modeFuncCount = sizeof(modeFuncs) / sizeof(modeFuncs[0]);  //  find the number of mode functions (at compile, not during runtime)

  areSensorsPresent();  //  check if any sensors were disconnected (we don't want to do this with the other core because of potentiall interferance with reading the sensors)

  printDoneLight.setSpeed(pdl_DimingTime);
  mainLight.setSpeed(dimingTime);

  if (mode >= modeFuncCount) {  //  if the mode is invalid
    errorInfo = mode;  //  record what the mode was
    mode = 0;  // Set mode to error
    errorOrigin = 2;  //  record where the error originated
  }

  modeFuncs[mode]();  //  call the mode function coresponding to the mode

  // Blink built-in LED to indicate loop
  blinkLED(1);
}

//  the loop code for core 2
void loop1() {
  #ifdef debug
  printf("core 1 loop\n");  //  print a debug message over the sellected debug port
  #endif

  //  check if the enclosure is in an error state and get stuck if it is
  if (errorDetected || mode == 0) {  //  errorDetected is actually a boolian variable that is set to true when error() is called. so, we can check it here without anything else (like "if (error == true) {}")
    coreOneError();

  } else {
    serialReceiveEvent();  //  check for any commands sent via USB

    //  update the state of the switches
    checkButtons();

    //  check if the screen is connected
    isScreenConnected();

    //  update the state of the menu navigation buttons, draw the menu
    checkMenuButtons();

    //  set the print done light to the correct state
    printDoneLight.setState(printDone);

    //  set the lights to the correct state
    mainLight.setState(lightSetState);

    // Check if the light status needs to change
    if (changeLights) {  //  if it needs to change:
      mainLight.changeState();
    }

    if (checkI2c) {
      receiveEvent(0);
    }

    printDoneLight.tick();
    mainLight.tick();
  }
}

