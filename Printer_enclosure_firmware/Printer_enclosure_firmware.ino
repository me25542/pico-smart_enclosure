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

//  include the necissary libraries:
#include <Wire.h>  //  Include the library for I2C comunication
#include <Adafruit_MCP9808.h>  //  Include the library for using the temp sensors
#include <Servo.h>  //  include the library for controling servos
#include <Bounce2.h>  //  include the debouncing library

//  define pins, etc. here:

const int ledPin = 25;  //  this is set in hardware; don't change
  //  servo pins:
const int servo2Pin = 20;  //  this is set in hardware; don't change
const int servo1Pin = 19;  //  this is set in hardware; don't change

const int pokPin = 18;  //  this is set in hardware; don't change
const int printDoneLightPin = 17;  //  this is set in hardware; don't change
  //  mode indicator light pins:
const int printingLightPin = 16;  //  this is set in hardware; don't change
const int cooldownLightPin = 15;  //  this is set in hardware; don't change
const int standbyLightPin = 14;  //  this is set in hardware; don't change
const int errorLightPin = 13;  //  this is set in hardware; don't change
  //  user input switch pins:
const int coolDownSwitchPin = 12;  //  this is set in hardware; don't change
const int lightSwitchPin = 11;  //  this is set in hardware; don't change
  //  heater pins:
const int heater1Pin = 10;  //  this is set in hardware; don't change
const int heater2Pin = 9;  //  this is set in hardware; don't change
  //  pin 8 is used to reset the pico ocasionally to avoid milis() overflow
const int resetPin = 8;  //  EXPERIMENTAL
  
  //  pins 7 & 6 are used for I2C1
const int I2C1_SCL = 7;  //  this is set in hardware; don't change
const int I2C1_SDA = 6;  //  this is set in hardware; don't change

//  pins 4 & 5 are used for I2C0
const int I2C0_SDA = 4;  //  this is set in hardware; don't change
const int I2C0_SCL = 5;  //  this is set in hardware; don't change

const int doorSwitchPin = 3;
const int lightsPin = 2;  //  this is set in hardware; don't change
const int psPin = 1;  //  this is set in hardware; don't change
const int fanPin = 0;  //  this is set in hardware; don't change

//  define other things here:
const int maxInOutTemp = 75;  //  the maximum temp the inside or outside of the enclosure can be before triggering an error
const int maxHeaterTemp = 90;  //  the maximum temp the heater can reach before triggering an error
const int minTemp = 0;  //  the minimum temp any of the sensors can read before triggering an error
const int fanOffVal = 0;  //  the pwm value used when the fan should be off
const int fanMidVal = 127;  //  the pwm value used when the fan should be halfway on
const int fanOnVal = 255;  //  the pwm value used when the fan should be all the way on
const int fanKickstartTime = 500; //  the time (in miliseconds) that the fan will be turned on at 100% before being set to its target value
const int bigDiff = 3;  //  the value used to define a large temp difference (in deg. c.)
const int cooldownDif = 7;  //  if the inside and outside temps are within this value of eachother, cooldown() will go to standby()
const int dimingTime = 2;  //  this x 255 = the time (in miliseconds) that togling the lights will take
const int i2cTempSensorRes = 1;  //  0 = 0.5, 1 = 0.25, 2 = 0.125, 3 = 0.625  (higher res takes longer to read)
const int heaterTempSensorAdress = 0x18;  //  this is the I2C adress for the heater temp sensor
const int inTempSensorAdress = 0x19;  //  this is the I2C adress for the in temp sensor
const int outTempSensorAdress = 0x1A;  //  this is the I2C adress for the out temp sensor
const int maxSerialStartupTries = 100;  //  this is the number of tries (or the number of 10ms periods) that will be taken to connect to usb before giving up
const int maxCoreOneShutdownTime = 2500;  //  the maximum time (in miliseconds) that the the second core (core1) can take to acknowledge an error andshut down
const int servo1Open = 180;  //  the "open" position for servo 1
const int servo2Open = 180;  //  the "open" position for servo 2
const int servo1Closed = 0;  //  the "closed" position for servo 1
const int servo2Closed = 0;  //  the "closed" position for servo 2

const long serialSpeed = 250000;  //  the buad rate that will be used for serial communication

const unsigned long debounceTime = 25000;  //  this is the debounce delay, in microseconds (1μs = 1s/1,000,000)

//const float testThermalRunawayTemps[] = {-3, 0, 1, 2.5, 4, 5.5, 7, 8.5};  //  the temp diference between unheated and heated air at some checkpoints
//const unsigned long testThermalRunawayTimes[] = {0, 10000, 20000, 40000, 60000, 80000, 100000, 120000};  //  the times that the above temps will be checked at

volatile int mode = 1;  //  tracks the enclosures operating mode (0 = error, 1 = standby, 2 = cooldown, 3 = printing)
volatile int globalSetTemp = 10;  //  tracks what temperature the enclosure should be at
volatile int errorOrigin = 0;  /*  records where an error originated (usefull for diagnostics)
(0 = N/A, 1 = Heater check failure, 2 = unrecognised mode, 3 = failure to start I2C temp sensors, 4 = printer commanded error, 5 = invalid printer command, 6 = sensors disconnected, 7 = serial commanded error, 8 = invalid serial command)*/
volatile int errorInfo = 0;  //  records aditionall info about any posible errors

volatile bool errorDetected = false;  //  tracks if an error has been detected (used to instruct the second core (core1) to shut down)
volatile bool coreZeroShutdown = false;  //  tracks if the first core (core0) has safed everything and entered an infinite loop (used only when an error is detected)
volatile bool coreOneShutdown = false;  //  tracks if the second core (core1) has stoped doing stuff and entered an infinite loop (used only when an error is detected)
volatile bool printDone = false;  //  tracks if the print is done (used to turn on the print done light)
volatile bool doorOpen = false;  //  tracks if the door is open
volatile bool lightSetState = false;  //  tracks the state the lights should be in
volatile bool changeLights = false;  //  tracks if the lights need to be changed (only used for printer-commanded changes)
volatile bool dontTurnOffThePSU = false;  //  set to true during the changing of the lights state, to avoid the first core turning off the PSU at inconvinient times
volatile bool lightState = false;  //  tracks the status of the lights

int oldS1_Pos;  //  tracks the old position of servo 1
int oldS2_Pos;  //  tracks the old position of servo 2

bool printingLastLoop = false;  //  tracks, basically, if the last loop went to printing() or not. used to avoid falsely seting heatingMode
bool heatingMode = false;  //  tracks if the enclosure is in heating or cooling mode (false = cooling, true = heating)

float heaterTemp = 20;  //  tracks the heater temp
float inTemp = 20;  //  tracks the temp inside the enclosure
float outTemp = 20;  //  tracks the temp outside the enclosure

//  set servo variables:
Servo servo1;
Servo servo2;

//  define button variables
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
  // put your setup code here, to run once:

  #include "pinDefs.h"  //  define all the pins; their modes, and their initial outputs
  #include "tempSensorsSetup.h" //  start the temp sensors and their I2C buss, then check if they work
  #include "servosSetup.h"  //  setup the servos, move them to home position
  
  for (int serialStartupTries = 0; serialStartupTries < maxSerialStartupTries && !startSerial(); ++ serialStartupTries);  //  start serial (USB) comunication (and wait for up to one second for a computer to be connected)

  //  print a message over serial (USB)
  Serial.println("Startup sucessful");

  //  blink LED to show startup compleat:
  blinkLED(1000);
}

//  the setup code for core 2
void setup1() {
  #include "buttonSetup.h"  //  set up the switches
  #include "printerI2cSetup.h"  //  start the I2C buss connected to the printer

}

//  the loop code for core 1
void loop() {
  // put your main code here, to run repeatedly:
  
  Serial.println("loop");  //  print a message over serial (USB)

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
    if (coolDown_switch.released()) {
      manualCooldown();
    }

    setPrintDoneLight(printDone); //  set the print done light to the correct state
    setLights(lightSetState); //  set the lights to the correct state
    // Check if the light status needs to change
    if (changeLights) {  //  if it needs to change:
      lightChange();  // Change the light state
    }

    delay(10);  //  wait for 10ms | you might change this later, it isn't neciccarily needed
  }
}
