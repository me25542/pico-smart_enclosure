/**
* @file config.hpp
* @brief main config file
*/

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

#include "constants.hpp"
#include <Arduino.h>

#define DEBUG true // sets if debug messages will be sent. set to "true" for debuging messages, and to "false" for none (except if there is an error)
#define SERIAL_CONTROL false // sets if serial control will be used, and thus if serial should be initialized at the start of the program if debug is false

// version info (e.g. 1.2.3 : majorV. = 1, minorV. = 2, bugFixV. = 3)
constexpr uint8_t majorVersion = 2;
constexpr uint8_t minorVersion = 1;
constexpr uint8_t bugFixVersion = 0;
constexpr uint8_t buildVersion = 18; // this might be useful if you make your own changes to the code

// other

#define MANUAL_PRESSED_STATE false // are you seting the pressed state of the switches manually, or automatically?

#define WATCHDOG_TIME 5000 // the loop time (in milliseconds) of either core at or above which error mode will be entered

#define LED_ON_TIME 100 // the time (in milliseconds) the led will be on for when blinking (not in error mode)
#define LED_OFF_TIME_STANDBY 4900 // the time (in milliseconds) that the LED will stay off for when blinking and the mode is standby
#define LED_OFF_TIME_OTHER 900 // the time (in milliseconds) that the LED will stay off for when blinking and the mode is not standby (or error)

#define MIN_SET_TEMP 5
#define MAX_SET_TEMP 75
#define DEFAULT_SET_TEMP 20

#define MAX_SERIAL_STARTUP_TRIES 1000 // the number of tries (or the number of 10ms periods) that will be taken to connect to USB before giving up
#define MAX_SCREEN_STARTUP_TRIES 100 // the number of times that turning the screen on will be tried
#define MAX_PSU_ON_TIME 10000 // the maximum time (in milliseconds) that the PSU can take to turn on before throwing an error
#define MAX_OTHER_CORE_SHUTDOWN_TIME 2500 // the maximum time (in milliseconds) that the first core will wait for the second core (core1) to acknowledge an error and shut down before proceeding anyways

#define MAX_IN_OUT_TEMP 75 // the maximum temp the inside or outside of the enclosure can be before triggering an error
#define MAX_HEATER_TEMP 90 // the maximum temp the heater can reach before triggering an error
#define MIN_TEMP 0 // the minimum temp any of the sensors can read before triggering an error

#define ERROR_FIX_INTERVAL 3600000 // the time (in miliseconds) after reboot when it will be forgoten if the reboot was to fix an error
#define MAX_I2C_WAIT_TIME 10000000 // the maximum time (in microseconds) to wait for I2C resources to be available
#define ENCLOSURE_ADDRESS 0x08 ///< the I2C address of the enclosure (what address the printer can connect to)
#define SERIAL_TIMOUT 100 // the serial timout time, in miliseconds

#define MENU_SELLECTED_INDICATOR ">" // MUST BE ONLY ONE CHARACTER!

#define USE_DEFAULT_DEBOUNCE_TIME false // if default Bounce2 stable interval time (10ms) is to be used

#if !USE_DEFAULT_DEBOUNCE_TIME
#define DOOR_SWITCH_DEBOUNCE_TIME 50
#define LIGHT_SWITCH_DEBOUNCE_TIME 50
#define COOLDOWN_SWITCH_DEBOUNCE_TIME 50
#define SELL_SWITCH_DEBOUNCE_TIME 20
#define UP_SWITCH_DEBOUNCE_TIME 20
#define DOWN_SWITCH_DEBOUNCE_TIME 20
#endif

#define BOOTSEL_UPDATE_TIME 1000 ///< the interval (in milliseconds) between when the state of the BOOTSEL button is updated

#define COOLDOWN_SWITCH_HOLD_TIME 5000 ///< the time, in miliseconds, that the cooldown switch needs to be held to trigger a cooldown

#define SCREENSAVER_SPEED 7 ///< the time (in milliseconds) between changes of lit pixel when screensaver displayed

#define DEFAULT_LIGHTS_ON_ON_DOOR_OPEN true
#define DEFAULT_SAVE_STATE_ON_POWER_LOSS true

#define DEFAULT_NAME_SCROLL_SPEED 100
#define DEFAULT_MENU_SCROLL_SPEED 75

#define DEFAULT_FAN_OFF_VAL 0
#define DEFAULT_FAN_MID_VAL 128
#define DEFAULT_FAN_ON_VAL 255
#define DEFAULT_DEFAULT_MAX_FAN_SPEED 255

#define DEFAULT_HYSTERESIS 1
#define DEFAULT_BIG_DIFF 3
#define DEFAULT_COOLDOWN_DIFF 3

#define DEFAULT_DIMING_TIME 3
#define DEFAULT_PDL_DIMING_TIME 5

#define DEFAULT_I2C_TEMP_SENSOR_RES 1
#define DEFAULT_SENSOR_READS 3
#define DEFAULT_SENSOR_READ_INTERVAL 10

#define DEFAULT_SERVO1_OPEN 180
#define DEFAULT_SERVO2_OPEN 180
#define DEFAULT_SERVO1_CLOSED 0
#define DEFAULT_SERVO2_CLOSED 0
#define DEFAULT_SERVO1_SPEED 1
#define DEFAULT_SERVO2_SPEED 1

#define DEFAULT_BACKUP_INTERVAL 1
#define DEFAULT_SCREENSAVER_TIME 15
#define DEFAULT_FAN_KICKSTART_TIME 1000
#define DEFAULT_MENU_BUTTON_HOLD_TIME 750

#define DEFAULT_CONTROL_MODE CONTROL_MODE_TEMP // developmental

// hardware (depends on the hardware you use and how you wired everything)

#define FAN_ON HIGH ///< pin state at which the fan will be on
#define MAIN_LIGHTS_ON HIGH ///< pin state at which the main lights will be on
#define PRINT_DONE_LIGHT_ON HIGH ///< pin state at which the print done light will be on
#define ERROR_LIGHT_ON HIGH ///< pin state at which the error light will be on

#define HEATER_TEMP_SENSOR_ADDRESS 0x18 ///< I2C address of the heater temp sensor
#define IN_TEMP_SENSOR_ADDRESS 0x19 ///< I2C address of the in temp sensor
#define OUT_TEMP_SENSOR_ADDRESS 0x1A ///< I2C address of the out temp sensor
#define SCREEN_ADDRESS 0x3C ///< I2C address of screen | See datasheet for Address; usually 0x3D for 128x64, 0x3C for 128x32 (this code not designed with 128x32 in mind; be warned)

#define SCREEN_HEIGHT 64 ///< OLED display height, in pixels | code currently not designed for this to be changed. sorry.
#define SCREEN_WIDTH 128 ///< OLED display width, in pixels | code currently not designed for this to be changed. sorry.
#define LINE_GAP 2 // gap to be inserted between lines of text on screen, in pixels
#define LINE_SPACING (CHARACTER_HEIGHT + LINE_GAP) /// the number of pixels between the top of each line of text
#define VISIBLE_MENU_ITEMS ((SCREEN_HEIGHT - ((CHARACTER_HEIGHT * 2) - LINE_GAP)) / LINE_SPACING) // the number of menu items that are visible when the print name is not displayed

#if MANUAL_PRESSED_STATE
constexpr bool doorSw_ps = LOW;
constexpr bool lightSw_ps = LOW;
constexpr bool coolDownSw_ps = LOW;
constexpr bool sellSw_ps = LOW;
constexpr bool upSw_ps = LOW;
constexpr bool downSw_ps = LOW;
#endif

// pins (how did you wire everything up?):

#define LED_PIN LED_BUILTIN /// the pin connected to the built-in LED | output

#define SELL_SWITCH_PIN 16 /// the pin connected to the "sellect" button | input
#define UP_SWITCH_PIN 15 /// the pin connected to the "up" button | input
#define DOWN_SWITCH_PIN 14 /// the pin connected to the "down" button | input

#define SERVO_2_PIN 20 /// the pin connected to servo 1 | output
#define SERVO_1_PIN 19 /// the pin connected to servo 2 | output

#define POWER_OK_PIN 18 /// the pin connected to the PSUs P_OK pin | input

#define ERROR_LIGHT_PIN 13 /// the pin connected to the "error" mode indicator light | output
#define PRINT_DONE_LIGHT_PIN 17 /// the pin connected to the print done light | output

#define COOLDOWN_SWITCH_PIN 12 /// the pin connected to the "cooldown" switch / button | input
#define LIGHTSWITCH_PIN 11  ///  the pin connected to the "lights" switch / button | input

#define HEATER_1_PIN 10 /// the pin connected to heater 1 | output
#define HEATER_2_PIN 9 /// the pin connected to heater 2 | output

#define I2C1_SCL_PIN 7 /// this is set somewhat in hardware; be careful changing
#define I2C1_SDA_PIN 6 /// this is set somewhat in hardware; be careful changing

#define I2C0_SDA_PIN 4 /// this is set somewhat in hardware; be careful changing
#define I2C0_SCL_PIN 5 /// this is set somewhat in hardware; be careful changing

#define DOOR_SWITCH_PIN 3 /// the pin connected to the door safety switch | input
#define LIGHTS_PIN 2 /// the pin connected to the enclosure lights | output
#define PSU_ON_PIN 1 /// the pin connected to the PSUs power on pin | output
#define FAN_PIN 0 /// the pin connected to the vent fan | output

#if DEBUG
#warning "USB debuging is enabled; remember to have the host connected and active at boot"
#else
#warning "USB debuging is disabled; if USB controll is not enabled USB will only become active in the event of an error"
#endif

#if SERIAL_CONTROL
#warning "USB controll is enabled; remember to have the host connected and active at boot"
#else
#warning "USB controll is disabled; if debuging is not enabled USB will only become active in the event of an error"
#endif

#if MANUAL_PRESSED_STATE
#warning "The pressed states of the buttons are being set manually; ensure they are correctly configured"
#else
#warning "The pressed states of the buttons are being set automatically; ensure none of them are pressed at boot"
#endif
