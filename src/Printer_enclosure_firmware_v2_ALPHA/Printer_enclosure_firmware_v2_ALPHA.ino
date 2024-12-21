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


//********************************************************************************************************************************************************************************

#include <EEPROM.h>

//********************************************************************************************************************************************************************************

#include "vars.hpp"  //  include all global variables
#include "menuFuncs.hpp"  //  include all the functions used for the menu (procesing button input, drawing the menu on the screen, etc.)
#include "otherFuncs.hpp"  //  Include all the other functions
#include "ISRs.hpp" //  Include all ISRs (Interrupt Service Routines)
#include "modeFuncs.hpp"  //  Include the functions called based on the enclosure mode
#include "setup.hpp"  //  include all the functions with the setup code


//********************************************************************************************************************************************************************************

//  maybe I should change where (in what file) this goes
#if debug
#warning "Debuging is enabled, check that your sellected port is correct. (Don't worry about this warning, the code is fine)"
#else
#warning "Debuging is not enabled, debug messages will not be printed. It is still advised that you check that the sellected port is correct, as it will be used in the event of an error. (Don't worry about this warning, the code is fine)"
#endif

#if manualPressedState
#warning "The pressed states of the buttons are being set manually, make sure they are correct. (Don't worry about this warning, the code is fine)"
#else
#warning "The pressed states of the buttons are being set automatically, make sure the first time you run this none of them are pressed. (Don't worry about this warning, the code is fine)"
#endif

//********************************************************************************************************************************************************************************

//  the setup code for core 1
void setup() {
  //  this code runs in serial, before the other core does it's startup code | put anything that can't hapen at the same time as other code here

  varInit();
  
  serialSetup();

  for (uint16_t i = 0; i < maxSerialStartupTries && !Serial; i++);  //  wait for a computer to be connected, but only for a bit

  core1StartStartup = true;  //  tell the other core to start startup

  //  put the rest of your startup code here, if it can hapen while the other core runs it's startup code

  pinSetup();

  servoSetup();

  #if debug
  printf("seting up the PSU...\n");
  #endif

  if (! setPSU(true)) {  //  try turning on the PSU
    startupError = true;  //  if it failed, we didin't suceed
  }

  #if debug
  printf("done seting up the PSU.\n");
  #endif

  if (! tempSensorSetup()) {  //  try turning on the temp sensors
    startupError = true;  //  if it failed, we didin't suceed
  }

  printerI2cSetup();

  attachInterrupt(digitalPinToInterrupt(pokPin), losingPower, FALLING);  //  power loss interrupt

  #if debug
  printf("waiting for core1 to start up...\n");
  #endif

  while (! coreOneStartup) {  //  while the second core hasn't started up
    delay(1);  //  wait for a tiny bit
  }

  #if debug
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

  #if debug
  printf("trying to set up the menu...\n");
  #endif

  if (menuSetup()) {  //  try seting up the menu
    backupRecovery();  //  initialize the menu with the data stored in the simulated EEPROM, if aplicable

    buttonSetup();  //  set up the buttons

  } else {  //  if it failed
    startupError = true;  //  if it failed we didn't suceed
  }

  #if debug
  printf("done trying to set up the menu.\n");
  #endif

  #if debug
  printf("waiting for core0 to be done startup...\n");
  #endif

  do {  //  do this at least once:
    coreOneStartup = true;  //  tell the other core that we have finished starting up

    if (startupError) {  //  if there was an error starting up in either core
      coreOneError();
    }

    delayMicroseconds(1);  //  wait for a tiny bit

  } while (! coreZeroStartup);  //  and then repeat it while the first core hasn't started up

  #if debug
  printf("done waiting for core0 to be done startup.\n");
  #endif
}

//********************************************************************************************************************************************************************************

//  the loop code for core 1
  //  this core is intended to run everything that doesn't need to be real-time. (still, try to keep it fast)
void loop() {
  #if debug
  printf("Core 0 start of loop.\n");  //  print a debug message over the sellected debug port
  #endif

  //  some variables
  const static void (*modeFuncs[])(void) = {error, standby, cooldown, printing};  //  an array of functions to call based on the mode | this will only be made the first time through the loop
  const static uint8_t modeFuncCount = sizeof(modeFuncs) / sizeof(modeFuncs[0]);  //  find the number of mode functions (at compile, not during runtime)

  //  update if the BOOTSEL button is pressed
  bootsel = BOOTSEL;

  //  update the diming speeds of the lights. we don't need to do this in real-time, so we do it here not in the other core
  printDoneLight.setSpeed(pdl_DimingTime);
  mainLight.setSpeed(dimingTime);

  //  set the print done light to the correct state
  printDoneLight.setState(printDone);

  //  set the lights to the correct state (only used for printer-commanded states)
  mainLight.setState(lightSetState);

  if (changeLights) {  //  if the light's state needs to change:
    mainLight.changeState();
  }

  //  check for any commands sent via USB
  serialReceiveEvent();
  
  if (checkI2c) {
    parseI2C();
  }

  //  check if the screen is connected
  isScreenConnected();
  
  if (mode >= modeFuncCount) {  //  if the mode is invalid
    errorInfo = mode;  //  record what the mode was
    mode = 0;  // Set mode to error
    errorOrigin = 2;  //  record where the error originated
  }

  //  call the mode function coresponding to the mode
  modeFuncs[mode]();

  //  update the variable tracking the old mode
  oldMode = mode;

  // Blink built-in LED to indicate loop
  blinkLED(1);
}

//  the loop code for core 2
  //  this core is intended to run real-time stuff; keep all code as fast as possible. anything too slow should go in the other core
void loop1() {
  #if debug
  printf("Core 1 start of loop.\n");  //  print a debug message over the sellected debug port
  #endif

  //  check if the enclosure is in an error state and get stuck if it is
  if (errorDetected || mode == 0) {  //  errorDetected is actually a boolian variable that is set to true when error() is called. so, we can check it here without anything else (like "if (error == true) {}")
    coreOneError();

  } else {
    //  update the state of the switches
    checkButtons();

    //  update the state of the menu navigation buttons
    checkMenuButtons();

    //  update the screen
    updateScreen();

    //  update the lights
    printDoneLight.tick();
    mainLight.tick();
  }
}
