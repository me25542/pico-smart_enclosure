#include <vector>
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

#include "vars.hpp"

//  preferences (how it will opperate) | some of these are volitile, not constant, as they can be edited via the menu
std::atomic<bool> lights_On_On_Door_Open = DEFAULT_LIGHTS_ON_ON_DOOR_OPEN;  //  controlls if the lights turn on when the door is opened.
std::atomic<bool> saveStateOnPowerLoss = DEFAULT_SAVE_STATE_ON_POWER_LOSS;

std::atomic<uint8_t> nameScrollSpeed = DEFAULT_NAME_SCROLL_SPEED;
std::atomic<uint8_t> menuScrollSpeed = DEFAULT_MENU_SCROLL_SPEED;
std::atomic<uint8_t> fanOffVal = DEFAULT_FAN_OFF_VAL;
std::atomic<uint8_t> fanMidVal = DEFAULT_FAN_MID_VAL;
std::atomic<uint8_t> fanOnVal = DEFAULT_FAN_ON_VAL;
std::atomic<uint8_t> defaultMaxFanSpeed = DEFAULT_DEFAULT_MAX_FAN_SPEED;
std::atomic<uint8_t> maxFanSpeed = defaultMaxFanSpeed.load();
std::atomic<uint8_t> hysteresis = DEFAULT_HYSTERESIS;
std::atomic<uint8_t> bigDiff = DEFAULT_BIG_DIFF;
std::atomic<uint8_t> cooldownDif = DEFAULT_COOLDOWN_DIFF;
std::atomic<uint8_t> dimingTime = DEFAULT_DIMING_TIME;
std::atomic<uint8_t> pdl_DimingTime = DEFAULT_PDL_DIMING_TIME;
std::atomic<uint8_t> i2cTempSensorRes = DEFAULT_I2C_TEMP_SENSOR_RES;
std::atomic<uint8_t> servo1Open = DEFAULT_SERVO1_OPEN;
std::atomic<uint8_t> servo2Open = DEFAULT_SERVO2_OPEN;
std::atomic<uint8_t> servo1Closed = DEFAULT_SERVO1_CLOSED;
std::atomic<uint8_t> servo2Closed = DEFAULT_SERVO2_CLOSED;
std::atomic<uint8_t> servo1Speed = DEFAULT_SERVO1_SPEED;
std::atomic<uint8_t> servo2Speed = DEFAULT_SERVO2_SPEED;
std::atomic<uint8_t> sensorReadInterval = DEFAULT_SENSOR_READ_INTERVAL;
std::atomic<uint8_t> controlMode = DEFAULT_CONTROL_MODE;
std::atomic<uint8_t> sensorReads = DEFAULT_SENSOR_READS;

std::atomic<uint16_t> backupInterval = DEFAULT_BACKUP_INTERVAL; // the amount of time (in s) between backups of the menu data
std::atomic<uint16_t> screensaverTime = DEFAULT_SCREENSAVER_TIME; // how long without user input intil the screensave is displayed (s)
std::atomic<uint16_t> fanKickstartTime = DEFAULT_FAN_KICKSTART_TIME; //  the time (in miliseconds) that the fan will be turned on at 100% before being set to its target value
std::atomic<uint16_t> menuButtonHoldTime = DEFAULT_MENU_BUTTON_HOLD_TIME;  //  how long the "up" or "down" buttons need to be held for to be counted as being held (in miliseconds)

//********************************************************************************************************************************************************************************

// utilities (you can't really set these, but they are used for stuff)

std::vector<uint8_t> menuDataBackup;

std::atomic<bool> is_error_recoverable = false;

mutex_t I2C_mutex;
uint32_t I2C_mutex_owner;

mutex_t PSU_mutex;
uint32_t PSU_mutex_owner;

std::atomic<char> printName[256]; // an array of characters to store the print name

std::atomic<int32_t> errorInfo = 0; // records aditionall info about any posible errors

bool findPressedState = true;

#if !manualPressedState
bool doorSw_ps = LOW;
bool lightSw_ps = LOW;
bool coolDownSw_ps = LOW;
bool sellSw_ps = LOW;
bool upSw_ps = LOW;
bool downSw_ps = LOW;
#endif

bool screensaver = false;
bool bootsel = false;
bool hysteresisTriggered = false;
bool dispLastLoop = false;
bool editingMenuItem = false;
bool heatingMode = false;
std::atomic<bool> PSUIsOn;
std::atomic<bool> turnLightOff = false;
std::atomic<bool> printDone = false;
std::atomic<bool> doorOpen = true;
std::atomic<bool> lightSetState = false;
std::atomic<bool> changeLights = false;
std::atomic<bool> core1StartStartup = false;
std::atomic<bool> checkI2c = false;
std::atomic<bool> startupError = false;
std::atomic<bool> coreZeroStartup = false;
std::atomic<bool> coreOneStartup = false;

uint8_t targetFanSpeed = !FAN_ON * 255;
uint8_t oldControlMode = DEFAULT_CONTROL_MODE;
uint8_t oldMode = DEFAULT_MODE;
std::atomic<uint8_t> mode = DEFAULT_MODE;
std::atomic<uint8_t> globalSetTemp = 20;
volatile uint8_t errorOrigin = 0;

uint8_t servo1SetPos;
uint8_t servo2SetPos;

int16_t heaterTemp = 20;
int16_t inTemp = 20;
int16_t outTemp = 20;

uint32_t lastUserInput;

uint32_t core1Time;

uint8_t menu::selectedItem = 0;
uint8_t menu::topDisplayMenuItem = 0;
uint8_t menu::bottomDisplayMenuItem = VISIBLE_MENU_ITEMS - 1;

lights::Light printDoneLight(PRINT_DONE_LIGHT_PIN, pdl_DimingTime, PRINT_DONE_LIGHT_ON, false, &PSUIsOn);
lights::Light mainLight(LIGHTS_PIN, dimingTime, MAIN_LIGHTS_ON, false, &PSUIsOn);

// an instance of the Adafruit_SSD1306 class (the display)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

std::vector<String> modeSubs = {"Sb.", "Cd.", "Pr."}; // offset of 1
std::vector<String> onOffSubs = {"Off", "On"}; // offset of 0, intended for bools
std::vector<String> yesNoSubs = {"No", "Yes"}; // offset of 0, intended for bools
std::vector<String> controlModeSubs = {"Temp", "Manl"}; // offset of 1, for the control mode

menu::baseMenuItem* mainMenu[] = {
  new menu::menuItem<std::atomic<uint8_t>>(&mode, MODE_STANDBY, MODE_PRINTING, "Mode", modeSubs, 1),
  new menu::menuItem<std::atomic<uint8_t>>(&globalSetTemp, MIN_SET_TEMP, MAX_SET_TEMP, "Set temp"),
  new menu::menuItem<std::atomic<bool>>(&lightSetState, 0, 1, "Lights", onOffSubs),
  new menu::menuItem<std::atomic<bool>>(&printDone, 0, 1, "Print done", yesNoSubs),
  new menu::menuItem<std::atomic<uint8_t>>(&maxFanSpeed, 0, 255, "Max fan speed"),
  new menu::menuItem<std::atomic<bool>>(&lights_On_On_Door_Open, 0, 1, "L. on door open", yesNoSubs),
  new menu::menuItem<std::atomic<uint8_t>>(&menuScrollSpeed, 1, 255, "Scroll speed"),
  new menu::menuItem<std::atomic<uint8_t>>(&nameScrollSpeed, 1, 255, "Name scroll spd"),
  new menu::menuItem<std::atomic<uint8_t>>(&dimingTime, 1, 255, "M.l. diming spd"),
  new menu::menuItem<std::atomic<uint8_t>>(&pdl_DimingTime, 1, 255, "Pd.l. dimng spd"),
  new menu::menuItem<std::atomic<uint16_t>>(&menuButtonHoldTime, 10, 9999, "Button hld time"),
  new menu::menuItem<std::atomic<uint16_t>>(&screensaverTime, 10, 3600, "Scrensaver time"),
  new menu::menuItem<std::atomic<uint8_t>>(&bigDiff, 1, 99, "Big temp diff"),
  new menu::menuItem<std::atomic<uint8_t>>(&cooldownDif, 1, 99, "Cooldown diff"),
  new menu::menuItem<std::atomic<uint8_t>>(&hysteresis, 0, 99, "Hysterisis"),
  new menu::menuItem<std::atomic<uint8_t>>(&controlMode, 1, 2, "Control mode", controlModeSubs, 1),
  new menu::menuItem<std::atomic<uint8_t>>(&defaultMaxFanSpeed, 0, 255, "Def max fan spd"),
  new menu::menuItem<std::atomic<uint16_t>>(&fanKickstartTime, 0, 9999, "Fan kstart time"),
  new menu::menuItem<std::atomic<uint8_t>>(&fanOnVal, 0, 255, "Fan on value"),
  new menu::menuItem<std::atomic<uint8_t>>(&fanMidVal, 0, 255, "Fan mid value"),
  new menu::menuItem<std::atomic<uint8_t>>(&fanOffVal, 0, 255, "Fan off value"),
  new menu::menuItem<std::atomic<uint8_t>>(&servo1Closed, 0, 180, "Servo1 clsd pos"),
  new menu::menuItem<std::atomic<uint8_t>>(&servo2Closed, 0, 180, "Servo2 clsd pos"),
  new menu::menuItem<std::atomic<uint8_t>>(&servo1Open, 0, 180, "Servo1 open pos"),
  new menu::menuItem<std::atomic<uint8_t>>(&servo2Open, 0, 180, "Servo2 open pos"),
  new menu::menuItem<std::atomic<uint8_t>>(&servo1Speed, 0, 20, "Servo1 speed"),
  new menu::menuItem<std::atomic<uint8_t>>(&servo2Speed, 0, 20, "Servo2 speed"),
  new menu::menuItem<std::atomic<uint8_t>>(&sensorReads, 1, 10, "Sensor reads"),
  new menu::menuItem<std::atomic<uint8_t>>(&sensorReadInterval, 0, 255, "Snsr read intvl"),
  new menu::menuItem<std::atomic<uint16_t>>(&backupInterval, 0, 1800, "Data save intvl"),
  new menu::menuItem<std::atomic<bool>>(&saveStateOnPowerLoss, 0, 1, "Pwr loss backup", yesNoSubs)
};

const uint8_t menuLength = sizeof(mainMenu) / sizeof(mainMenu[0]);

buffers::CircularBuffer I2cBuffer;

//  set servo variables:
Servo servo1;
Servo servo2;

//  set button variables
Bounce2::Button door_switch = Bounce2::Button();
Bounce2::Button light_switch = Bounce2::Button();
Bounce2::Button coolDown_switch = Bounce2::Button();
Bounce2::Button sell_switch = Bounce2::Button();
Bounce2::Button up_switch = Bounce2::Button();
Bounce2::Button down_switch = Bounce2::Button();

//  set I2C temp sensor variables:
Adafruit_MCP9808 heaterTempSensor = Adafruit_MCP9808();
Adafruit_MCP9808 outTempSensor = Adafruit_MCP9808();
Adafruit_MCP9808 inTempSensor = Adafruit_MCP9808();

timers::Timer core0WatchdogTimer;
timers::Timer core1WatchdogTimer;

