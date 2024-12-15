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

#ifndef VARS_H
#define VARS_H

//  sets if debug messages will be sent. set to "true" for debuging messages, and to "false" for none (except if there is an error)
#define debug false

#include <Arduino.h>
#include <EEPROM.h>
#include <pico/stdlib.h>
#include <pico/sync.h>
#include <pico/mutex.h>

#include <Wire.h>  //  Include the library for I2C comunication
#include <Adafruit_MCP9808.h>  //  Include the library for using the temp sensors
#include <Adafruit_GFX.h>  //  Include the graphics library
#include <Adafruit_SSD1306.h>  //  Include the library for controling the screen
#include <Servo.h>  //  include the library for controling servos
#include <Bounce2.h>  //  include the debouncing library

//********************************************************************************************************************************************************************************

//  pins (how did you wire everything up?):

const uint8_t ledPin = 25;  //  this is set in hardware; don't change

const uint8_t sellPin = 16;
const uint8_t upPin = 15;
const uint8_t downPin = 14;

const uint8_t testLightPin = 22;

  //  servo pins:
const uint8_t servo2Pin = 20;  //  the pin connected to servo 1 | output
const uint8_t servo1Pin = 19;  //  the pin connected to servo 2 | output

const uint8_t pokPin = 18;  //  the pin connected to the PSUs P_OK pin | input
const uint8_t printDoneLightPin = 17;  //  the pin connected to the print done light | output

  //  mode indicator light pins:
const uint8_t errorLightPin = 13;  //  the pin connected to the "error" mode indicator light | output
  //  user input switch pins:
const uint8_t coolDownSwitchPin = 12;  //  the pin connected to the "cooldown" switch / button | input
const uint8_t lightSwitchPin = 11;  //  the pin connected to the "lights" switch / button | input
  //  heater pins:
const uint8_t heater1Pin = 10;  //  the pin connected to heater 1 | output
const uint8_t heater2Pin = 9;  //  the pin connected to heater 2 | output
  //  pin 8 is used to reset the pico ocasionally to avoid milis() overflow
const uint8_t resetPin = 8;  //  EXPERIMENTAL | output
  
  //  pins 7 & 6 are used for I2C1
const uint8_t I2C1_SCL = 7;  //  this is kind of set in hardware; be careful changing
const uint8_t I2C1_SDA = 6;  //  this is kind of set in hardware; be careful changing

//  pins 4 & 5 are used for I2C0
const uint8_t I2C0_SDA = 4;  //  this is kind of set in hardware; be careful changing
const uint8_t I2C0_SCL = 5;  //  this is kind of set in hardware; be careful changing

const uint8_t doorSwitchPin = 3;  //  the pin connected to the door safety switch | input
const uint8_t lightsPin = 2;  //  the pin connected to the enclosure lights | output
const uint8_t psPin = 1;  //  the pin connected to the PSUs power on pin | output
const uint8_t fanPin = 0;  //  the pin connected to the vent fan | output

//********************************************************************************************************************************************************************************

//  preferences (how it will opperate) | some of these are volitile, not constant, as they can be edited via the menu

#define manualPressedState false  //  are you seting the pressed state of the switches manually, or automatically?

//  version info (e.g. 1.2.3 : majorV. = 1, minorV. = 2, bugFixV. = 3)
const uint8_t majorVersion = 2;
const uint8_t minorVersion = 0;
const uint8_t bugFixVersion = 0;
const uint8_t buildVersion = 12;  //  this might be useful if you make your own changes to the code

extern volatile bool lights_On_On_Door_Open;  //  controlls if the lights turn on when the door is opened.
extern volatile bool saveStateOnPowerLoss;

extern volatile uint8_t nameScrollSpeed;  //  how fast the print name will scroll by (lower is faster, miliseconds per pixel)
extern volatile uint8_t menuScrollSpeed;  //  how fast the menu will scroll / values will update when either the "up" or "down" button is held
extern volatile uint8_t fanOffVal;  //  the pwm value used when the fan should be off
extern volatile uint8_t fanMidVal;  //  the pwm value used when the fan should be halfway on
extern volatile uint8_t fanOnVal;  //  the pwm value used when the fan should be all the way on
extern volatile uint8_t defaultMaxFanSpeed;  //  the default maxumum fan speed (what will be used if nothing else is specified)
extern volatile uint8_t bigDiff;  //  the value used to define a large temp difference (in deg. c.)
extern volatile uint8_t cooldownDif;  //  if the inside and outside temps are within this value of eachother, cooldown() will go to standby()
extern volatile uint8_t dimingTime;  //  this * 255 = the time (in miliseconds) that togling the lights will take
extern volatile uint8_t pdl_DimingTime;  //  this * 255 = the time (in miliseconds) that changing the state of the print done light will take
extern volatile uint8_t i2cTempSensorRes;  //  0 = 0.5, 1 = 0.25, 2 = 0.125, 3 = 0.625  (higher res takes longer to read)
extern volatile uint8_t servo1Open;  //  the "open" position for servo 1
extern volatile uint8_t servo2Open;  //  the "open" position for servo 2
extern volatile uint8_t servo1Closed;  //  the "closed" position for servo 1
extern volatile uint8_t servo2Closed;  //  the "closed" position for servo 2
extern volatile uint8_t sensorReadInterval;  //  the minimum time (in s) between temp readings

extern volatile uint16_t fanKickstartTime; //  the time (in miliseconds) that the fan will be turned on at 100% before being set to its target value
extern volatile uint16_t menuButtonHoldTime;  //  how long the "up" or "down" buttons need to be held for to be counted as being held (in miliseconds)

const uint8_t minSetTemp = 0;
const uint8_t maxSetTemp = 75;
const uint8_t maxMode = 4;  //  the value at which the mode parser will start to ignore the sent byte
extern volatile uint8_t sensorReads;  //  the number of times the temp sensors will be read each time the temp is goten (the values will be averaged)

const uint16_t maxSerialStartupTries = 1000;  //  this is the number of tries (or the number of 10ms periods) that will be taken to connect to usb before giving up
const uint16_t maxScreenStartupTries = 100;  //  this is the number of times that turning the screen on will be tried
const uint16_t maxPSUOnTime = 5000;  //  the maximum time (in miliseconds) that the PSU can take to turn on before throwing an error
const uint16_t maxCoreOneShutdownTime = 2500;  //  the maximum time (in miliseconds) that the first core will wait for the second core (core1) to acknowledge an error and shut down before proceeding anyways
const int16_t maxInOutTemp = 75;  //  the maximum temp the inside or outside of the enclosure can be before triggering an error
const int16_t maxHeaterTemp = 90;  //  the maximum temp the heater can reach before triggering an error
const int16_t minTemp = 0;  //  the minimum temp any of the sensors can read before triggering an error

const String modeStrings[] {"Er.", "Sb.", "Cd.", "Pr."};  //  the abreviations for the mode displayed at the top of the screen
const String errorCauses[] {"Unknown / invalid origin.", "Origin: Temp sensor check.", "Origin: Unrecognised mode.", "Origin: Failure to start temp sensors.",
  "Origin: Printer commanded error mode.", "Origin: Invalid command received.","Origin: Temp sensor disconnected.", "Origin: serial commanded error.",
  "Origin: invalid serial command.", "Origin: failure to start PSU in allocated time.", "Origin: failure to start screen.", "Origin: invalid datatype for a menu item.",
  "Origin: screen disconnected.", "Origin: wait for I2C resource timeout."};  //  Plane text explanations of the meaning of different values of errorOrigin

const uint8_t numErrorCauses = sizeof(errorCauses) / sizeof(errorCauses[0]);

const uint32_t maxI2CWaitTime = 2500000;  //  the maximum time (in microseconds) to wait for I2C resources to be available
const uint32_t serialSpeed = 250000;  //  the buad rate that will be used for serial communication
const uint32_t serialTimout = 100;  //  the serial timout time, in miliseconds
const uint32_t debounceTime = 25000;  //  this is the debounce delay, in microseconds (1μs = 1s/1,000,000)
const uint32_t cooldownSwitchHoldTime = 5000;  //  this is the time, in miliseconds, that the cooldown switch needs to be held to trigger a cooldown

//********************************************************************************************************************************************************************************

//  hardware (depends on the hardware you decided to use and how you wired everything)
  //  remember that HIGH = true = 1, and LOW = false = 0
#define fanOn true
const bool mainLightsOn = HIGH;
const bool printDoneLightOn = HIGH;
const bool errorLightOn = HIGH;

const uint8_t heaterTempSensorAdress = 0x18;  //  this is the I2C adress for the heater temp sensor
const uint8_t inTempSensorAdress = 0x19;  //  this is the I2C adress for the in temp sensor
const uint8_t outTempSensorAdress = 0x1A;  //  this is the I2C adress for the out temp sensor
const uint8_t SCREEN_ADDRESS = 0x3C;  //  See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
const uint8_t screen_height = 64;  // OLED display height, in pixels
const uint8_t screen_width = 128;  // OLED display width, in pixels
const uint8_t lineSpacing = 10;  //  the number of pixels between the top of each line of text | A character with no vertical padding is 8 pixels high
const uint8_t visibleMenuItems = (screen_height - (16 - (lineSpacing - 8))) / lineSpacing;  /*  the number of menu items that are visible when the print name is not displayed
| 16 because at the top of the screen will be double-size text (16 pixels), and 8 because that is the height of one character
*/

//********************************************************************************************************************************************************************************

//  utilities (you can't really set these, but they are used for stuff)

extern mutex_t I2C_mutex;  //  a mutex to protect I2C0
extern uint32_t I2C_mutex_owner;

extern mutex_t PSU_mutex;  //  a mutex to protect controling the PSU
extern uint32_t PSU_mutex_owner;

extern volatile bool tmp_bool;  //  a bool used to store the value of a menu item of the same datatype while it is being edited
extern volatile uint8_t tmp_uint8t;  //  a byte used to store the value of a menu item of the same datatype while it is being edited
extern volatile int8_t tmp_int8t;
extern volatile uint16_t tmp_uint16t;
extern volatile int16_t tmp_int16t;
extern volatile uint32_t tmp_uint32t;
extern volatile int32_t tmp_int32t;

extern volatile char printName[256];  //  an array of characters to store the print name

extern volatile uint8_t I2cBuffer_readPos;  //  stores where to read from the buffer next
extern volatile uint8_t I2cBuffer_numBytes;  //  stores the number of bytes to be read from the buffer

extern volatile int32_t errorInfo;  //  records aditionall info about any posible errors

#if manualPressedState  //  if we are seting the switches pressed state manually

//  set states here
const bool doorSw_ps = LOW;
const bool lightSw_ps = LOW;
const bool coolDownSw_ps = LOW;
const bool sellSw_ps = LOW;
const bool upSw_ps = LOW;
const bool downSw_ps = LOW;

#else  //  if we are seting the switches pressed state automatically
extern volatile bool findPressedState;

extern volatile bool doorSw_ps;
extern volatile bool lightSw_ps;
extern volatile bool coolDownSw_ps;
extern volatile bool sellSw_ps;
extern volatile bool upSw_ps;
extern volatile bool downSw_ps;

#endif


extern volatile bool core1StartStartup;
extern volatile bool PSUIsOn;
extern volatile bool checkI2c;
extern volatile bool startupError;
extern volatile bool lightswPressedSenceDoorOpen;
extern volatile bool lightStateBeforeDoorOpen;
extern volatile bool dispLastLoop;  //  tracks if the print name was displayed last loop
extern volatile bool errorDetected;  //  tracks if an error has been detected (used to instruct the second core (core1) to shut down)
extern volatile bool coreZeroShutdown;  //  tracks if the first core (core0) has safed everything and entered an infinite loop (used only when an error is detected)
extern volatile bool coreOneShutdown;  //  tracks if the second core (core1) has stoped doing stuff and entered an infinite loop (used only when an error is detected)
extern volatile bool coreZeroStartup;  //  tracks if the first core (core0) has finished starting up
extern volatile bool coreOneStartup;  //  tracks if the second core (core1) has finished starting up
extern volatile bool printDone;  //  tracks if the print is done (used to turn on the print done light)
extern volatile bool doorOpen;  //  tracks if the door is open
extern volatile bool lightSetState;  //  tracks the state the lights should be in
extern volatile bool changeLights;  //  tracks if the lights need to be changed (only used for printer-commanded changes)
extern volatile bool lightState;  //  tracks the status of the lights
extern volatile bool editingMenuItem;  //  tracks wheather the sellected menu item is being edited
extern volatile bool printingLastLoop;  //  tracks, basically, if the last loop went to printing() or not. used to avoid falsely seting heatingMode
extern volatile bool heatingMode;  //  tracks if the enclosure is in heating or cooling mode (false = cooling, true = heating)

extern volatile uint8_t selectedItem;  //  tracks the sellected menu item
extern volatile uint8_t topDisplayItem;  //  tracks the menu item that is at the top of the display
extern volatile uint8_t mode;  //  tracks the enclosures operating mode (0 = error, 1 = standby, 2 = cooldown, 3 = printing)
extern volatile uint8_t oldMode;  //  tracks the mode the enclosure was in last loop
extern volatile uint8_t maxFanSpeed;  //  tracks the maximum fan speed alowable
extern volatile uint8_t globalSetTemp;  //  tracks what temperature the enclosure should be at
extern volatile uint8_t errorOrigin;  /*  records where an error originated (usefull for diagnostics)
(0 = N/A, 1 = Heater check failure, 2 = unrecognised mode, 3 = failure to start I2C temp sensors, 4 = printer commanded error, 5 = invalid printer command,
6 = sensors disconnected, 7 = serial commanded error, 8 = invalid serial command, 9 = failure to start PSU in allocated time, 10 = failure to start screen,
11 = invalid datatype for a menu item, 12 = screen disconnected, 13 = wait for I2C timeout)*/

extern uint8_t oldS1_Pos;  //  tracks the old position of servo 1
extern uint8_t oldS2_Pos;  //  tracks the old position of servo 2

extern int16_t heaterTemp;  //  tracks the heater temp
extern int16_t inTemp;  //  tracks the temp inside the enclosure
extern int16_t outTemp;  //  tracks the temp outside the enclosure

//********************************************************************************************************************************************************************************

/**
* @brief class to implament a 256-byte circular buffer
*/
class circularBuffer {
  private:
    /**
    * @brief the actual buffer, a 256-item array of uint8_t's
    */
    volatile uint8_t circularBuffer_buffer[256];
    /**
    * @brief where to write to next in the buffer
    */
    volatile uint8_t circularBuffer_writePos;
    /**
    * @brief where to read from next in the buffer
    */
    volatile uint8_t circularBuffer_readPos;
    /**
    * @brief stores weather or not the buffer is full; if the write and read positions are the same the buffer could be full or empty
    */
    volatile bool circularBuffer_isFull;

  public:
    circularBuffer();

    /**
    * @brief reads the next byte from the buffer
    */
    uint8_t read();

    /**
    * @brief writes a byte to the buffer. returns true on sucessfull write, false if buffer full
    */
    bool write(uint8_t data);

    /**
    * @brief returns the number of remaining bytes to be read in the buffer
    */
    uint8_t available();

    /**
    * @brief returns true if the buffer is full or not
    */
    bool isFull();

    /**
    * @brief return true if the buffer is empty
    */
    bool isEmpty();
};

/**
* @brief class to store items on the main menu
*/
class menuItem {

  private:
    volatile void *data;
    /**
    * @brief stores the type of data pointed to by data. (all volatile) 0 = bool, 1 = uint8_t, 2 = int8_t, 3 = uint16_t, 4 = int16_t, 5 = uint32_t, 6 = int32_t
    */
    uint8_t dataType;
    uint32_t menuItem_minVal;
    uint32_t menuItem_maxVal;
    String name;

  public:
    menuItem(volatile void *dat, uint8_t datType, String n);

    menuItem(volatile void *dat, uint8_t datType, uint32_t minVal, uint32_t maxVal, String n);

    void setData(volatile void *dat);

    volatile void * getData();

    /**
    * @brief the datatype pointed to by data. (all volatile) 0 = bool, 1 = uint8_t, 2 = int8_t, 3 = uint16_t, 4 = int16_t, 5 = uint32_t, 6 = int32_t
    */
    void setDataType(uint8_t datType);

    /**
    * @brief returns the type of data pointed to by data. (all volatile) 0 = bool, 1 = uint8_t, 2 = int8_t, 3 = uint16_t, 4 = int16_t, 5 = uint32_t, 6 = int32_t
    */
    uint8_t getDataType();

    uint32_t getMinVal();

    uint32_t getMaxVal();

    void setName(String n);

    String getName();

    //  boolian Retrieval Function
    bool menuItemToBool();

    // 8-bit Integer Retrieval Function
    int8_t menuItemToInt8();

    // unsigned 8-bit Integer Retrieval Function
    uint8_t menuItemToUInt8();

    // 16-bit Integer Retrieval Function
    int16_t menuItemToInt16();

    // unsigned 16-bit Integer Retrieval Function
    uint16_t menuItemToUInt16();

    // 32-bit Integer Retrieval Function
    int32_t menuItemToInt32();

    // unsigned 32-bit Integer Retrieval Function
    uint32_t menuItemToUInt32();

    //  boolian data seting function
    void boolToMenuItem(volatile bool dat);

    //  unsigned 8-bit integer data seting function
    void uint8ToMenuItem(volatile uint8_t dat);

    //  8-bit integer data seting function
    void int8ToMenuItem(volatile int8_t dat);

    //  unsigned 16-bit integer data seting function
    void uint16ToMenuItem(volatile uint16_t dat);

    //  16-bit integer data seting function
    void int16ToMenuItem(volatile int16_t dat);

    //  unsigned 32-bit integer data seting function
    void uint32ToMenuItem(volatile uint32_t dat);

    //  32-bit integer data seting function
    void int32ToMenuItem(volatile int32_t dat);
};

/**
* @brief class to smoothly controll a light's state without blocking other code
*/
class light {
  private:
    const uint8_t light_pin;
    volatile uint8_t light_speed;
    const bool light_onState;
    volatile bool light_state;
    volatile bool light_changing;
    volatile bool light_toChange;
    volatile uint8_t light_i;
    volatile uint32_t light_time;

  public:
    /**
    * @brief initializes the light with it's inital state and the speed at which to change it
    * @param pin the pin the light is connected to
    * @param speed the speed the light will change at (lower is faster)
    * @param state the initial state of the light
    */
    light(uint8_t pin, uint8_t speed, bool onState, bool state);

    uint8_t getPin();

    /**
    * @brief returns true if the light is changing OR on, false otherwise (if it is off AND not changing)
    */
    bool needsPower();

    /**
    * @brief sets the speed to change the light at (lower is faster)
    */
    void setSpeed(uint8_t speed);

    uint8_t getSpeed();

    /**
    * @brief sets the state of the light
    */
    void setState(bool state);

    /**
    * @brief changes the state of the light
    */
    void changeState();

    bool getState();

    /**
    * @brief call as often as possible
    */
    void tick();
};

//********************************************************************************************************************************************************************************

extern light printDoneLight;
extern light mainLight;

//  an instance of the Adafruit_SSD1306 class (the display)
extern Adafruit_SSD1306 display;

//  the main menu (an array of instances of the menuItem class)
extern menuItem mainMenu[];  //  create the main menu with a set number of items in it

extern const uint8_t menuLength;  //  the length of the menu (the number of items it contains) | automatically found

extern circularBuffer I2cBuffer;

//  set servo variables:
extern Servo servo1;
extern Servo servo2;

//  set button variables
extern Bounce2::Button door_switch;
extern Bounce2::Button light_switch;
extern Bounce2::Button coolDown_switch;
extern Bounce2::Button sell_switch;
extern Bounce2::Button up_switch;
extern Bounce2::Button down_switch;

//  set I2C temp sensor variables:
extern Adafruit_MCP9808 heaterTempSensor;
extern Adafruit_MCP9808 outTempSensor;
extern Adafruit_MCP9808 inTempSensor;

#endif