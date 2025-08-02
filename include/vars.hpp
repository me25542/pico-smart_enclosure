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

#pragma once

#include "config.hpp"
#include <Arduino.h>
#include <EEPROM.h>
#include <pico/stdlib.h>
#include <pico/mutex.h>
#include <vector>
#include <string>
#include <Wire.h> // I2C library
#include <Adafruit_MCP9808.h> // temp sensors library
#include <Adafruit_GFX.h> // graphics library
#include <Adafruit_SSD1306.h> // screen library
#include <Servo.h> // servo library
#include <Bounce2.h> // button library
#include <lights.hpp>
#include <timers.hpp>
#include <buffers.hpp>
#include <menu.hpp>
#include <relays.hpp>
#include <fans.hpp>

//*********************************************************************************************************************************************************************************

// preferences (how it will opperate) | some of these are volitile or atomic, not constant, as they can be edited via the menu

extern std::atomic<bool> lights_On_On_Door_Open; ///< controlls if the lights turn on when the door is opened.
extern std::atomic<bool> saveStateOnPowerLoss; ///< controlls if state info will be saved on detected power loss

extern std::atomic<uint8_t> nameScrollSpeed; ///< how fast the print name will scroll by (lower is faster, miliseconds per pixel)
extern std::atomic<uint8_t> menuScrollSpeed; ///< how fast the menu will scroll / values will update when either the "up" or "down" button is held
extern std::atomic<uint8_t> fanOffVal; ///< the pwm value used when the fan should be off
extern std::atomic<uint8_t> fanMidVal; ///< the pwm value used when the fan should be halfway on
extern std::atomic<uint8_t> fanOnVal; ///< the pwm value used when the fan should be all the way on
extern std::atomic<uint8_t> defaultMaxFanSpeed; ///< the default maxumum fan speed (what will be used if nothing else is specified)
extern std::atomic<uint8_t> maxFanSpeed; ///< tracks the maximum fan speed alowable
extern std::atomic<uint8_t> hysteresis; ///< the "dead zone" value, the temp can get above or below the target by this much before action is taken
extern std::atomic<uint8_t> bigDiff; ///< the value used to define a large temp difference (in deg. c.)
extern std::atomic<uint8_t> cooldownDif; ///< if the inside and outside temps are within this value of eachother, cooldown() will go to standby()
extern std::atomic<uint8_t> dimingTime; ///< this * 255 = the time (in miliseconds) that togling the lights will take
extern std::atomic<uint8_t> pdl_DimingTime; ///< this * 255 = the time (in miliseconds) that changing the state of the print done light will take
extern std::atomic<uint8_t> i2cTempSensorRes; ///< 0 = 0.5, 1 = 0.25, 2 = 0.125, 3 = 0.625  (higher res takes longer to read)
extern std::atomic<uint8_t> servo1Open; ///< the "open" position for servo 1
extern std::atomic<uint8_t> servo2Open; ///< the "open" position for servo 2
extern std::atomic<uint8_t> servo1Closed; ///< the "closed" position for servo 1
extern std::atomic<uint8_t> servo2Closed; ///< the "closed" position for servo 2
extern std::atomic<uint8_t> servo1Speed; ///< the delay (in miliseconds) in between steps of servo 1's movement
extern std::atomic<uint8_t> servo2Speed; ///< the delay (in miliseconds) in between steps of servo 2's movement
extern std::atomic<uint8_t> sensorReadInterval; ///< the minimum time (in s) between temp readings
extern std::atomic<uint8_t> controlMode; ///< DEVELOPMENTAL | enclosure control mode (temperature, manual)
extern std::atomic<uint8_t> sensorReads; ///< the number of times the temp sensors will be read each time the temp is goten (the values will be averaged)

extern std::atomic<uint16_t> backupInterval; ///< the number of time (in ms) between backups of the menu data
extern std::atomic<uint16_t> screensaverTime; ///< how long without user input intil the screensave is displayed (s)
extern std::atomic<uint16_t> fanKickstartTime; ///<  the time (in miliseconds) that the fan will be turned on at 100% before being set to its target value
extern std::atomic<uint16_t> menuButtonHoldTime; ///< how long the "up" or "down" buttons need to be held for to be counted as being held (in miliseconds)



const String modeStrings[] {"Er.", "Sb.", "Cd.", "Pr."}; ///< the abreviations for the mode displayed at the top of the screen
const String errorCauses[] {"Unknown / invalid origin", "Temp sensor check", "Invalid mode", "Failure to start temp sensors",
	"Printer commanded error mode", "Invalid command","Temp sensor disconnected", "Serial commanded error",
	"Invalid serial command", "Failure to start PSU", "Failure to start screen", "(Software) watchdog timeout",
	"Screen disconnected", "Wait for I2C resource timeout", "Invalid parameters passed to function"}; ///< Plane text explanations of the meaning of different values of errorOrigin

constexpr uint8_t numErrorCauses = sizeof(errorCauses) / sizeof(errorCauses[0]);

//*********************************************************************************************************************************************************************************

// utilities (you can't really set these, but they are used for stuff)

constexpr uint8_t majorVersion = MAJOR_VERSION; ///< the major version of the code (major.minor.bugfix)
constexpr uint8_t minorVersion = MINOR_VERSION; ///< the minor version of the code (major.minor.bugfix)
constexpr uint8_t bugFixVersion = BUGFIX_VERSION; ///< the bugfix version of the code (major.minor.bugfix)
constexpr uint8_t buildVersion = BUILD_VERSION; ///< the build version of the code

extern std::vector<uint8_t> menuDataBackup; ///< stores menu backup data

extern std::atomic<bool> is_error_recoverable; ///< tracks if a given error is recoverable or not. Default value is false

extern mutex_t I2C_mutex; ///< the mutex protecting I2C0
extern uint32_t I2C_mutex_owner;

extern mutex_t PSU_mutex; ///< the mutex protecting control of the PSU
extern uint32_t PSU_mutex_owner;

extern std::atomic<char> printName[256]; ///< an array of characters to store the print name

extern std::atomic<int32_t> errorInfo; ///< records aditionall info about any posible errors

extern bool findPressedState; ///< used during startup to track if the pressed state of the switches and buttons needs to be set

extern bool screensaver; ///< tracks if the screensaver is active
extern bool bootsel; ///< tracks the state of the BOOTSEL button
extern bool hysteresisTriggered; ///< if the temp has risen above the target by the <hysteresis> amount
extern bool dispLastLoop; ///< tracks if the print name was displayed last loop
extern bool editingMenuItem; ///< tracks wheather the sellected menu item is being edited
extern bool heatingMode; ///< tracks if the enclosure is in heating or cooling mode (false = cooling, true = heating)

#if !MANUAL_PRESSED_STATE
extern bool doorSw_ps;
extern bool lightSw_ps;
extern bool coolDownSw_ps;
extern bool sellSw_ps;
extern bool upSw_ps;
extern bool downSw_ps;
#endif

extern std::atomic<bool> PSUIsOn; ///< tracks if the PSU is on
extern std::atomic<bool> turnLightOff; ///< tracks if the light needs to turn off (only used for printer-commanded changes)
extern std::atomic<bool> printDone; ///< tracks if the print is done (used to turn on the print done light)
extern std::atomic<bool> doorOpen; ///< tracks if the door is open
extern std::atomic<bool> lightSetState; ///< tracks the state the lights should be in
extern std::atomic<bool> changeLights; ///< tracks if the lights need to be changed (only used for printer-commanded changes)
extern std::atomic<bool> core1StartStartup; ///< flag to tell core1 so start its startup procedure
extern std::atomic<bool> checkI2c; ///< flag to parse stored I2C data located in the circular buffer
extern std::atomic<bool> startupError; ///< tracks if an error was encountered during startup
extern std::atomic<bool> coreZeroStartup; ///< tracks if the first core (core0) has finished starting up
extern std::atomic<bool> coreOneStartup; ///< tracks if the second core (core1) has finished starting up

extern uint8_t oldControlMode; ///< tracks the control mode the enclosure was in last loop
extern uint8_t oldMode; ///< tracks the mode the enclosure was in last loop
extern std::atomic<uint8_t> mode; ///< tracks the enclosures operating mode (0 = error, 1 = standby, 2 = cooldown, 3 = printing)
extern std::atomic<uint8_t> globalSetTemp; ///< tracks what temperature the enclosure should be at
extern volatile uint8_t errorOrigin; /* records where an error originated (usefull for diagnostics)
(0 = N/A, 1 = Heater check failure, 2 = unrecognised mode, 3 = failure to start I2C temp sensors, 4 = printer commanded error, 5 = invalid printer command,
6 = sensors disconnected, 7 = serial commanded error, 8 = invalid serial command, 9 = failure to start PSU in allocated time, 10 = failure to start screen,
11 = (software) watchdog timeout, 12 = screen disconnected, 13 = wait for I2C timeout, 14 = invalid parameters passed to function)*/

extern int16_t heaterTemp; ///< tracks the heater temp
extern int16_t inTemp; ///< tracks the temp inside the enclosure
extern int16_t outTemp; ///< tracks the temp outside the enclosure

extern uint32_t lastUserInput; ///< tracks when the last user input was

extern uint32_t core1Time; ///< updated each loop of core1, keeps track of the time for that loop

namespace menu {
	extern uint8_t topDisplayMenuItem; ///< the meu item to be displayed at the top of the visible portion of the screen

	extern uint8_t bottomDisplayMenuItem; ///< the meu item to be displayed at the bottom of the visible portion of the screen

	extern uint8_t selectedItem; ///< tracks the sellected menu item
}

// heater
extern relays::Heater heater; ///< the heater

// lights
extern lights::Light printDoneLight;
extern lights::Light mainLight;

// fan
extern fans::TwoServoFan fan;

// screen
extern Adafruit_SSD1306 display;

// main menu
extern menu::baseMenuItem* mainMenu[];

extern const uint8_t menuLength; ///< the length of the menu (the number of items it contains) | automatically found

// buffers
extern buffers::CircularBuffer I2cBuffer;

// buttons
extern Bounce2::Button door_switch;
extern Bounce2::Button light_switch;
extern Bounce2::Button coolDown_switch;
extern Bounce2::Button sell_switch;
extern Bounce2::Button up_switch;
extern Bounce2::Button down_switch;

// I2C temp sensors
extern Adafruit_MCP9808 heaterTempSensor;
extern Adafruit_MCP9808 outTempSensor;
extern Adafruit_MCP9808 inTempSensor;

// timers
extern timers::Timer core0WatchdogTimer;
extern timers::Timer core1WatchdogTimer;
