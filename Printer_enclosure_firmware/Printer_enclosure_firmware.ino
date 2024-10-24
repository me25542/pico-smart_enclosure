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

#define debug  //  this is a flag to print debuging info like, all the time. comment out when everything is working

//  include the necissary libraries:
#include <Wire.h>  //  Include the library for I2C comunication
#include <Adafruit_MCP9808.h>  //  Include the library for using the temp sensors
#include <Servo.h>  //  include the library for controling servos
#include <Bounce2.h>  //  include the debouncing library

//  pins (how did you wire everything up?):

const byte ledPin = 25;  //  this is set in hardware; don't change
  //  servo pins:
const byte servo2Pin = 20;  //  the pin connected to servo 1 | output
const byte servo1Pin = 19;  //  the pin connected to servo 2 | output

const byte pokPin = 18;  //  the pin connected to the PSUs P_OK pin | input
const byte printDoneLightPin = 17;  //  the pin connected to the print done light | output
  //  mode indicator light pins:
const byte printingLightPin = 16;  //  the pin connected to the "printing" mode indicator light | output
const byte cooldownLightPin = 15;  //  the pin connected to the "cooldown" mode indicator light | output
const byte standbyLightPin = 14;  //  the pin connected to the "standby" mode indicator light | output
const byte errorLightPin = 13;  //  the pin connected to the "error" mode indicator light | output
  //  user input switch pins:
const byte coolDownSwitchPin = 12;  //  the pin connected to the "cooldown" switch / button | input
const byte lightSwitchPin = 11;  //  the pin connected to the "lights" switch / button | input
  //  heater pins:
const byte heater1Pin = 10;  //  the pin connected to heater 1 | output
const byte heater2Pin = 9;  //  the pin connected to heater 2 | output
  //  pin 8 is used to reset the pico ocasionally to avoid milis() overflow
const byte resetPin = 8;  //  EXPERIMENTAL | output
  
  //  pins 7 & 6 are used for I2C1
const byte I2C1_SCL = 7;  //  this is kind of set in hardware; be careful changing
const byte I2C1_SDA = 6;  //  this is kind of set in hardware; be careful changing

//  pins 4 & 5 are used for I2C0
const byte I2C0_SDA = 4;  //  this is kind of set in hardware; be careful changing
const byte I2C0_SCL = 5;  //  this is kind of set in hardware; be careful changing

const byte doorSwitchPin = 3;  //  the pin connected to the door safety switch | input
const byte lightsPin = 2;  //  the pin connected to the enclosure lights | output
const byte psPin = 1;  //  the pin connected to the PSUs power on pin | output
const byte fanPin = 0;  //  the pin connected to the vent fan | output

//  preferences (how it will opperate)
const byte fanOffVal = 0;  //  the pwm value used when the fan should be off
const byte fanMidVal = 127;  //  the pwm value used when the fan should be halfway on
const byte fanOnVal = 255;  //  the pwm value used when the fan should be all the way on
const byte defaultMaxFanSpeed = 255;  //  the default maxumum fan speed (what will be used if nothing else is specified)
const int maxInOutTemp = 75;  //  the maximum temp the inside or outside of the enclosure can be before triggering an error
const int maxHeaterTemp = 90;  //  the maximum temp the heater can reach before triggering an error
const int minTemp = 0;  //  the minimum temp any of the sensors can read before triggering an error
const int fanKickstartTime = 500; //  the time (in miliseconds) that the fan will be turned on at 100% before being set to its target value
const int bigDiff = 3;  //  the value used to define a large temp difference (in deg. c.)
const int cooldownDif = 5;  //  if the inside and outside temps are within this value of eachother, cooldown() will go to standby()
const int dimingTime = 2;  //  this * 255 = the time (in miliseconds) that togling the lights will take
const int pdl_DimingTime = 1;  //  this * 255 = the time (in miliseconds) that changing the state of the print done light will take
const int il_DimingTime = 1;  //  this * 255 = the time (in miliseconds) that changing the state of an indicator light will take
const int i2cTempSensorRes = 1;  //  0 = 0.5, 1 = 0.25, 2 = 0.125, 3 = 0.625  (higher res takes longer to read)
const int sensorReads = 3;  //  the number of times the temp sensors will be read each time the temp is goten (the values will be averaged)
const int maxSerialStartupTries = 100;  //  this is the number of tries (or the number of 10ms periods) that will be taken to connect to usb before giving up
const int maxPSUOnTime = 5000;  //  the maximum time (in miliseconds) that the PSU can take to turn on before throwing an error
const int maxCoreOneShutdownTime = 2500;  //  the maximum time (in miliseconds) that the first core will wait for the second core (core1) to acknowledge an error and shut down before proceeding anyways
const int servo1Open = 180;  //  the "open" position for servo 1
const int servo2Open = 180;  //  the "open" position for servo 2
const int servo1Closed = 0;  //  the "closed" position for servo 1
const int servo2Closed = 0;  //  the "closed" position for servo 2

const unsigned long serialSpeed = 250000;  //  the buad rate that will be used for serial communication
const unsigned long serialTimout = 100;  //  the serial timout time, in miliseconds
const unsigned long debounceTime = 25000;  //  this is the debounce delay, in microseconds (1μs = 1s/1,000,000)
const unsigned long cooldownSwitchHoldTime = 5000;  //  this is the time, in miliseconds, that the cooldown switch needs to be held to trigger a cooldown

//  hardware (depends on how you wired everything)
const byte heaterTempSensorAdress = 0x18;  //  this is the I2C adress for the heater temp sensor
const byte inTempSensorAdress = 0x19;  //  this is the I2C adress for the in temp sensor
const byte outTempSensorAdress = 0x1A;  //  this is the I2C adress for the out temp sensor

//  default values for dynamic variables
volatile byte mode = 1;  //  tracks the enclosures operating mode (0 = error, 1 = standby, 2 = cooldown, 3 = printing)
volatile byte maxFanSpeed = defaultMaxFanSpeed;  //  tracks the maximum fan speed alowable
volatile byte errorOrigin = 0;  /*  records where an error originated (usefull for diagnostics)
(0 = N/A, 1 = Heater check failure, 2 = unrecognised mode, 3 = failure to start I2C temp sensors, 4 = printer commanded error, 5 = invalid printer command,
6 = sensors disconnected, 7 = serial commanded error, 8 = invalid serial command, 9 = failure to start PSU in allocated time)*/
volatile int errorInfo = 0;  //  records aditionall info about any posible errors
volatile int globalSetTemp = 10;  //  tracks what temperature the enclosure should be at

volatile bool errorDetected = false;  //  tracks if an error has been detected (used to instruct the second core (core1) to shut down)
volatile bool coreZeroShutdown = false;  //  tracks if the first core (core0) has safed everything and entered an infinite loop (used only when an error is detected)
volatile bool coreOneShutdown = false;  //  tracks if the second core (core1) has stoped doing stuff and entered an infinite loop (used only when an error is detected)
volatile bool coreZeroStartup = false;  //  tracks if the first core (core0) has finished starting up
volatile bool coreOneStartup = false;  //  tracks if the second core (core1) has finished starting up
volatile bool printDone = false;  //  tracks if the print is done (used to turn on the print done light)
volatile bool doorOpen = true;  //  tracks if the door is open
volatile bool lightSetState = false;  //  tracks the state the lights should be in
volatile bool changeLights = false;  //  tracks if the lights need to be changed (only used for printer-commanded changes)
volatile bool dontTurnOffThePSU = false;  //  set to true during the changing of the lights state, to avoid the first core turning off the PSU at inconvinient times
volatile bool lightState = false;  //  tracks the status of the lights

bool printingLastLoop = false;  //  tracks, basically, if the last loop went to printing() or not. used to avoid falsely seting heatingMode
bool heatingMode = false;  //  tracks if the enclosure is in heating or cooling mode (false = cooling, true = heating)

int oldS1_Pos;  //  tracks the old position of servo 1
int oldS2_Pos;  //  tracks the old position of servo 2

float heaterTemps[sensorReads];  //  stores all mesured values from the heater temp sensor each time the temp is read (not just an average), used for error checking
float inTemps[sensorReads];  //  stores all mesured values from the in temp sensor each time the temp is read (not just an average), used for error checking
float outTemps[sensorReads];  //  stores all mesured values from the out temp sensor each time the temp is read (not just an average), used for error checking

float heaterTemp = 20;  //  tracks the heater temp
float inTemp = 20;  //  tracks the temp inside the enclosure
float outTemp = 20;  //  tracks the temp outside the enclosure

//  set servo variables:
Servo servo1;
Servo servo2;

//  set button variables
Bounce2::Button door_switch = Bounce2::Button();
Bounce2::Button light_switch = Bounce2::Button();
Bounce2::Button coolDown_switch = Bounce2::Button();

//  set I2C temp sensor variables:
Adafruit_MCP9808 heaterTempSensor = Adafruit_MCP9808();
Adafruit_MCP9808 outTempSensor = Adafruit_MCP9808();
Adafruit_MCP9808 inTempSensor = Adafruit_MCP9808();

#include "otherFuncs.h"  //  Include all functions except ISRs and the ones called based on the enclosures mode
#include "ISRs.h" //  Include all ISRs
#include "modeFuncs.h"  //  Include the functions called based on the enclosure mode

//  the setup code for core 1
void setup() {
  setPSU(true);  //  turn on the PSU

  #include "pinDefs.h"  //  define all the pins; their modes, and their initial outputs
  #include "tempSensorsSetup.h" //  start the temp sensors and their I2C buss, then check if they work
  #include "servosSetup.h"  //  setup the servos, move them to home position
  
  for (int serialStartupTries = 0; serialStartupTries < maxSerialStartupTries && !startSerial(); ++ serialStartupTries);  //  start serial (USB) comunication (and wait for up to one second for a computer to be connected)
  Serial.setTimeout(serialTimout);  //  set the serial timout time

  while (! coreOneStartup) {  //  while the second core hasn't started up
    delay(1);  //  wait for a tiny bit
  }

  //  print a message over serial (USB)
  Serial.println("Startup sucessful");

  coreZeroStartup = true;  //  tell the other core that we have finished starting up

  //  blink LED to show startup compleat:
  blinkLED(1000);
}

//  the setup code for core 2
void setup1() {
  #include "buttonSetup.h"  //  set up the switches
  #include "printerI2cSetup.h"  //  start the I2C buss connected to the printer

  coreOneStartup = true;

  while (! coreZeroStartup) {  //  while the first core hasn't started up
    delay(1);  //  wait for a tiny bit
  }
}

//  the loop code for core 1
void loop() {
  #ifdef debug
  Serial.println("loop");  //  print a message over serial (USB)
  #endif

  areSensorsPresent();  //  check if any sensors were disconnected (we don't want to do this with the other core because of potentiall interferance with reading the sensors)

  // Check the mode and perform corresponding actions
  switch (mode) {  //  switch... case statement that checks the value of "mode"
    case 0:  // Error mode (if it is 0):
      error();  // Execute error handling (safe everything and get stuck in an infinite loop)
      break;  //  exit the switch... case statement

    case 1:  // Standby mode (if it is 1):
      standby();  // Execute standby operations
      break;  //  exit the switch... case statement

    case 2:  // Cooldown mode (if it is 2):
      cooldown();  // Execute cooldown operations
      break;  //  exit the switch... case statement

    case 3:  // Printing mode (if it is 3):
      printing();  // Execute printing operations
      break;  //  exit the switch... case statement

    default:  // Default case for invalid mode (if it is anything other than 0-3):
      errorInfo = mode;  //  record what the mode was
      mode = 0;  // Set mode to error
      errorOrigin = 2;  //  record where the error originated
      return;  // Go to the start of the main loop
  }

  // Blink built-in LED to indicate loop
  blinkLED(5);
}

//  the loop code for core 2
void loop1() {
  //  check if the enclosure is in an error state and get stuck if it is
  if (errorDetected) {  //  errorDetected is actually a boolian variable that is set to true when error() is called. so, we can check it here without anything else (like "if (error == true) {}")
    coreOneShutdown = true;  //  report that the second core (core1) has shut down
    while (true) {  //  loop forever
      delay(30000);  //  wait 30 seconds
      coreOneShutdown = true;  //  report that the second core (core1) has shut down
    }

  } else {
    serialReceiveEvent();  //  check for any commands sent via USB

    //  update the state of the switches
    checkButtons();

    setPrintDoneLight(printDone); //  set the print done light to the correct state
    setLights(lightSetState); //  set the lights to the correct state
    // Check if the light status needs to change
    if (changeLights) {  //  if it needs to change:
      lightChange();  // Change the light state
    }

    delay(10);  //  wait for 10ms | you might change this later, it isn't neciccarily needed
  }
}
