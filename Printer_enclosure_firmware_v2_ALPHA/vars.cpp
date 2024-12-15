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

#include "vars.h"


//  preferences (how it will opperate) | some of these are volitile, not constant, as they can be edited via the menu
volatile bool lights_On_On_Door_Open = true;  //  controlls if the lights turn on when the door is opened.
volatile bool saveStateOnPowerLoss = true;

volatile uint8_t nameScrollSpeed = 100;  //  how fast the print name will scroll by (lower is faster, miliseconds per pixel)
volatile uint8_t menuScrollSpeed = 75;  //  how fast the menu will scroll / values will update when either the "up" or "down" button is held
volatile uint8_t fanOffVal = 0;  //  the pwm value used when the fan should be off
volatile uint8_t fanMidVal = 127;  //  the pwm value used when the fan should be halfway on
volatile uint8_t fanOnVal = 255;  //  the pwm value used when the fan should be all the way on
volatile uint8_t defaultMaxFanSpeed = 255;  //  the default maxumum fan speed (what will be used if nothing else is specified)
volatile uint8_t bigDiff = 3;  //  the value used to define a large temp difference (in deg. c.)
volatile uint8_t cooldownDif = 3;  //  if the inside and outside temps are within this value of eachother, cooldown() will go to standby()
volatile uint8_t dimingTime = 3;  //  this * 255 = the time (in miliseconds) that togling the lights will take
volatile uint8_t pdl_DimingTime = 5;  //  this * 255 = the time (in miliseconds) that changing the state of the print done light will take
volatile uint8_t i2cTempSensorRes = 1;  //  0 = 0.5, 1 = 0.25, 2 = 0.125, 3 = 0.625  (higher res takes longer to read)
volatile uint8_t servo1Open = 180;  //  the "open" position for servo 1
volatile uint8_t servo2Open = 180;  //  the "open" position for servo 2
volatile uint8_t servo1Closed = 0;  //  the "closed" position for servo 1
volatile uint8_t servo2Closed = 0;  //  the "closed" position for servo 2
volatile uint8_t sensorReadInterval = 30;  //  this is set to 30 seconds

volatile uint16_t fanKickstartTime = 1000; //  the time (in miliseconds) that the fan will be turned on at 100% before being set to its target value
volatile uint16_t menuButtonHoldTime = 750;  //  how long the "up" or "down" buttons need to be held for to be counted as being held (in miliseconds)

volatile uint8_t sensorReads = 3;  //  the number of times the temp sensors will be read each time the temp is goten (the values will be averaged)

//********************************************************************************************************************************************************************************

//  utilities (you can't really set these, but they are used for stuff)

mutex_t I2C_mutex;
uint32_t I2C_mutex_owner;

mutex_t PSU_mutex;
uint32_t PSU_mutex_owner;

volatile bool PSUIsOn;
volatile bool tmp_bool;  //  a bool used to store the value of a menu item of the same datatype while it is being edited
volatile uint8_t tmp_uint8t;  //  a byte used to store the value of a menu item of the same datatype while it is being edited
volatile int8_t tmp_int8t;
volatile uint16_t tmp_uint16t;
volatile int16_t tmp_int16t;
volatile uint32_t tmp_uint32t;
volatile int32_t tmp_int32t;

volatile char printName[256];  //  an array of characters to store the print name

volatile uint8_t I2cBuffer_readPos = 0;  //  stores where to read from the buffer next
volatile uint8_t I2cBuffer_numBytes = 0;  //  stores the number of bytes to be read from the buffer

volatile int32_t errorInfo = 0;  //  records aditionall info about any posible errors

volatile bool findPressedState = true;

#if !manualPressedState
volatile bool doorSw_ps = LOW;
volatile bool lightSw_ps = LOW;
volatile bool coolDownSw_ps = LOW;
volatile bool sellSw_ps = LOW;
volatile bool upSw_ps = LOW;
volatile bool downSw_ps = LOW;
#endif

volatile bool core1StartStartup = false;
volatile bool checkI2c = false;
volatile bool startupError = false;
volatile bool lightswPressedSenceDoorOpen = false;
volatile bool lightStateBeforeDoorOpen = false;
volatile bool dispLastLoop = false;  //  tracks if the print name was displayed last loop
volatile bool errorDetected = false;  //  tracks if an error has been detected (used to instruct the second core (core1) to shut down)
volatile bool coreZeroShutdown = false;  //  tracks if the first core (core0) has safed everything and entered an infinite loop (used only when an error is detected)
volatile bool coreOneShutdown = false;  //  tracks if the second core (core1) has stoped doing stuff and entered an infinite loop (used only when an error is detected)
volatile bool coreZeroStartup = false;  //  tracks if the first core (core0) has finished starting up
volatile bool coreOneStartup = false;  //  tracks if the second core (core1) has finished starting up
volatile bool printDone = false;  //  tracks if the print is done (used to turn on the print done light)
volatile bool doorOpen = true;  //  tracks if the door is open
volatile bool lightSetState = false;  //  tracks the state the lights should be in
volatile bool changeLights = false;  //  tracks if the lights need to be changed (only used for printer-commanded changes)
volatile bool lightState = false;  //  tracks the status of the lights
volatile bool editingMenuItem = false;  //  tracks wheather the sellected menu item is being edited
volatile bool printingLastLoop = false;  //  tracks, basically, if the last loop went to printing() or not. used to avoid falsely seting heatingMode
volatile bool heatingMode = false;  //  tracks if the enclosure is in heating or cooling mode (false = cooling, true = heating)

volatile uint8_t selectedItem = 0;  //  tracks the sellected menu item
volatile uint8_t topDisplayItem = 0;  //  tracks the menu item that is at the top of the display
volatile uint8_t mode = 1;  //  tracks the enclosures operating mode (0 = error, 1 = standby, 2 = cooldown, 3 = printing)
volatile uint8_t oldMode = 1;  //  tracks the mode the enclosure was in last loop
volatile uint8_t maxFanSpeed = defaultMaxFanSpeed;  //  tracks the maximum fan speed alowable
volatile uint8_t globalSetTemp = 20;  //  tracks what temperature the enclosure should be at
volatile uint8_t errorOrigin = 0;  /*  records where an error originated (usefull for diagnostics)
(0 = N/A, 1 = Heater check failure, 2 = unrecognised mode, 3 = failure to start I2C temp sensors, 4 = printer commanded error, 5 = invalid printer command,
6 = sensors disconnected, 7 = serial commanded error, 8 = invalid serial command, 9 = failure to start PSU in allocated time, 10 = failure to start screen,
11 = invalid datatype for a menu item)*/

uint8_t oldS1_Pos;  //  tracks the old position of servo 1
uint8_t oldS2_Pos;  //  tracks the old position of servo 2

int16_t heaterTemp = 20;  //  tracks the heater temp
int16_t inTemp = 20;  //  tracks the temp inside the enclosure
int16_t outTemp = 20;  //  tracks the temp outside the enclosure

//********************************************************************************************************************************************************************************

circularBuffer::circularBuffer() {
  circularBuffer_readPos = 0;
  circularBuffer_writePos = 0;
  circularBuffer_isFull = false;

  //  write each byte in the buffer to 0
  for (uint8_t i = 0; i != 255; i++) {
    circularBuffer_buffer[i] = 0;
  }
}

uint8_t circularBuffer::read() {
  circularBuffer_isFull = false;

  return circularBuffer_buffer[circularBuffer_readPos++];  //  return a byte from the buffer, move where to read from next
    //  remember, circularBuffer_readPos is a uint8_t and will wrap around on its own
}

bool circularBuffer::write(uint8_t data) {
  if (circularBuffer_isFull) {  //  if the buffer is full
    return false;  //  we can't write anything
  }

  circularBuffer_buffer[circularBuffer_writePos++] = data;  //  write a byte to the buffer, move where to write to next
    //  remember, circularBuffer_writePos is a uint8_t and will wrap around on its own

  if (circularBuffer_writePos == circularBuffer_readPos) {
    circularBuffer_isFull = true;
  }

  return true;
}

uint8_t circularBuffer::available() {
  if (circularBuffer_isFull) {  //  if the buffer is full (the read and write positions are the same)
    return 255;

  } else {
    return circularBuffer_writePos - circularBuffer_readPos;  //  return the number of bytes still to be read
  }
}

bool circularBuffer::isFull() {
  return circularBuffer_isFull;
}

bool circularBuffer::isEmpty() {
  return (!circularBuffer_isFull && (circularBuffer_writePos == circularBuffer_readPos));  //  return true if the buffer isn't full AND the read and write positions are the same
}

//**********************************************************

menuItem::menuItem(volatile void *dat, uint8_t datType, String n)
  : data(dat), dataType(datType), name(n) {
  menuItem_minVal = 0;
  menuItem_maxVal = 9999;
}

menuItem::menuItem(volatile void *dat, uint8_t datType, uint32_t minVal, uint32_t maxVal, String n)
  : data(dat), dataType(datType), menuItem_minVal(minVal), menuItem_maxVal(maxVal), name(n) {

}

void menuItem::setData(volatile void *dat) {
  data = dat;
}

volatile void * menuItem::getData() {
  return data;
}

void menuItem::setDataType(uint8_t datType) {
  dataType = datType;
}

uint8_t menuItem::getDataType() {
  return dataType;
}

uint32_t menuItem::getMinVal() {
  return menuItem_minVal;
}

uint32_t menuItem::getMaxVal() {
  return menuItem_maxVal;
}

void menuItem::setName(String n) {
  name = n;
}

String menuItem::getName() {
  return name;
}

bool menuItem::menuItemToBool() {
  bool toReturn = *(reinterpret_cast<volatile bool*>(data));  //  find the value of the variable pointed to by the item
  return toReturn;
}

int8_t menuItem::menuItemToInt8() {
  int8_t toReturn = *(reinterpret_cast<volatile int8_t*>(data));  //  find the value of the variable pointed to by the item
  return toReturn;
}

uint8_t menuItem::menuItemToUInt8() {
  uint8_t toReturn = *(reinterpret_cast<volatile uint8_t*>(data));  //  find the value of the variable pointed to by the item
  return toReturn;
}

int16_t menuItem::menuItemToInt16() {
  int16_t toReturn = *(reinterpret_cast<volatile int16_t*>(data));  //  find the value of the variable pointed to by the item
  return toReturn;
}

uint16_t menuItem::menuItemToUInt16() {
  uint16_t toReturn = *(reinterpret_cast<volatile uint16_t*>(data));  //  find the value of the variable pointed to by the item
  return toReturn;
}

int32_t menuItem::menuItemToInt32() {
  int32_t toReturn = *(reinterpret_cast<volatile int32_t*>(data));  //  find the value of the variable pointed to by the item
  return toReturn;
}

uint32_t menuItem::menuItemToUInt32() {
  uint32_t toReturn = *(reinterpret_cast<volatile uint32_t*>(data));  //  find the value of the variable pointed to by the item
  return toReturn;
}

//  boolian data seting function
void menuItem::boolToMenuItem(volatile bool dat) {
  *reinterpret_cast<volatile bool*>(data) = dat;
}

//  unsigned 8-bit integer data seting function
void menuItem::uint8ToMenuItem(volatile uint8_t dat) {
  *reinterpret_cast<volatile uint8_t*>(data) = dat;
}

//  8-bit integer data seting function
void menuItem::int8ToMenuItem(volatile int8_t dat) {
  *reinterpret_cast<volatile int8_t*>(data) = dat;
}

//  unsigned 16-bit integer data seting function
void menuItem::uint16ToMenuItem(volatile uint16_t dat) {
  *reinterpret_cast<volatile uint16_t*>(data) = dat;
}

//  16-bit integer data seting function
void menuItem::int16ToMenuItem(volatile int16_t dat) {
  *reinterpret_cast<volatile int16_t*>(data) = dat;
}

//  unsigned 32-bit integer data seting function
void menuItem::uint32ToMenuItem(volatile uint32_t dat) {
  *reinterpret_cast<volatile uint32_t*>(data) = dat;
}

//  32-bit integer data seting function
void menuItem::int32ToMenuItem(volatile int32_t dat) {
  *reinterpret_cast<volatile int32_t*>(data) = dat;
}

//**********************************************************

light::light(uint8_t pin, uint8_t speed, bool onState, bool state)
  : light_pin(pin), light_speed(speed), light_onState(onState), light_state(state) {
  pinMode(light_pin, OUTPUT);
  digitalWrite(light_pin, light_state);
  light_time = millis();
}

uint8_t light::getPin() {
  return light_pin;
}

bool light::needsPower() {
  if (light_onState) {
    return (light_state || light_changing);

  } else {
    return (!light_state || light_changing);
  }
}

void light::setSpeed(uint8_t speed) {
  light_speed = speed;
}

uint8_t light::getSpeed() {
  return light_speed;
}

void light::setState(bool state) {
  //  this if statemet is to reverse the set state if the lights are on when LOW, as the internal state is equal to HIGH or LOW not on and off
  if (light_onState) {
    if (state != light_state) {
      light_toChange = true;
      light_changing = true;
    }

    light_state = state;

  } else {
    if (state == light_state) {
      light_toChange = true;
      light_changing = true;
    }

    light_state = !state;
  }
}

void light::changeState() {
  light_state = !light_state;
  light_toChange = true;
  light_changing = true;
}

bool light::getState() {
  //  this if statement flips the returned state if the lights are on when LOW, as the internal state is equal to HIGH or LOW not on and off
  if (light_onState) {
    return light_state;

  } else {
    return !light_state;
  }
}

void light::tick() {
  if (light_changing) {  //  If we are in the process of changing the light state
    if (light_toChange) {  //  initialization for if we haven't started changing yet
      light_toChange = false;

      if (light_state) {  //  if we are going to HIGH
        light_i = 0;  //  Start from fully LOW

      } else {  //  if we are going to LOW
        light_i = 255;  //  Start from fully HIGH
      }

      while (!PSUIsOn);  //  wait for the PSU to be on

      light_time = millis();  //  reset the last updtate time so that the lights don't turn on all at once
    }

    uint32_t currentTime = millis();
    uint32_t elapsedTime = currentTime - light_time;

    if (elapsedTime >= light_speed) {  //  Check if it's time to update the light brightness
      light_time = currentTime;  //  Update the last update time

      if (light_state) {  //  Turning the light HIGH
          //  Calculate the increment to apply this tick
          uint8_t increment = min(uint8_t(elapsedTime / light_speed), uint8_t(255 - light_i));
          light_i += increment;
          analogWrite(light_pin, light_i);

          if (light_i >= 255) {  //  If we've reached fully HIGH
            light_i = 255;  //  Clamp to max value
            light_changing = false;  //  Stop changing the light
          }

      } else { // Turning the light to LOW
        //  Calculate the decrement to apply this tick
        uint8_t decrement = min(uint8_t(elapsedTime / light_speed), light_i);
        light_i -= decrement;
        analogWrite(light_pin, light_i);

        if (light_i <= 0) {  //  If we've reached fully LOW
          light_i = 0;  //  Clamp to min value
          light_changing = false;  //  Stop changing the light
          
        }  //  if (light_i <= 0)
      }  //  else (  if (light_state)  )
    }  //  if (elapsedTime >= light_speed)
  }  //  if (light_changing)
}  //  light::tick()


//********************************************************************************************************************************************************************************

light printDoneLight(printDoneLightPin, pdl_DimingTime, printDoneLightOn, false);
light mainLight(lightsPin, dimingTime, mainLightsOn, false);

//  an instance of the Adafruit_SSD1306 class (the display)
Adafruit_SSD1306 display(screen_width, screen_height, &Wire, -1);

//  the main menu (an array of instances of the menuItem class)
menuItem mainMenu[] = { 
  menuItem(&mode, 1, 1, 3, "Mode"),
  menuItem(&globalSetTemp, 1, 10, 65, "Set tmp."),
  menuItem(&maxFanSpeed, 1, 0, 255, "Max fan spd."),
  menuItem(&lightSetState, 0, "Lights"),
  menuItem(&printDone, 0, "Print done"),
  menuItem(&fanKickstartTime, 3, 0, 9999, "Fan ks. time"),
  menuItem(&bigDiff, 1, 1, 99, "Big tmp. diff."),
  menuItem(&cooldownDif, 1, 1, 99, "Cd. diff."),
  menuItem(&defaultMaxFanSpeed, 1, 0, 255, "Def. max f. spd."),
  menuItem(&dimingTime, 1, 1, 255, "M.l. dmng. spd."),
  menuItem(&pdl_DimingTime, 1, 1, 255, "Pd.l. dmng. spd."),
  menuItem(&servo1Closed, 1, 0, 199, "S. 1 c. pos."),
  menuItem(&servo2Closed, 1, 0, 199, "S. 2 c. pos."),
  menuItem(&servo1Open, 1, 0, 199, "S. 1 o. pos."),
  menuItem(&servo2Open, 1, 0, 199, "S. 2 o. pos."),
  menuItem(&fanOnVal, 1, 0, 255, "F. on val."),
  menuItem(&fanOffVal, 1, 0, 255, "F. off val."),
  menuItem(&fanMidVal, 1, 0, 255, "F. mid val."),
  menuItem(&menuScrollSpeed, 1, 1, 255, "Scrl. spd."),
  menuItem(&menuButtonHoldTime, 3, 10, 9999, "Btn. hld. t."),
  menuItem(&nameScrollSpeed, 1, 1, 255, "Name scrl. spd."),
  menuItem(&lights_On_On_Door_Open, 0, "L. on door open"),
  menuItem(&sensorReads, 1, 1, 10, "sensor reads"),
  menuItem(&sensorReadInterval, 1, 0, 255, "S. read intrvl."),
  menuItem(&saveStateOnPowerLoss, 0, "pwr. ls. backup")
  };  //  create the main menu with some number of items in it

const uint8_t menuLength = sizeof(mainMenu) / sizeof(mainMenu[0]);  //  automatically find the number of menu items created

circularBuffer I2cBuffer;

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
