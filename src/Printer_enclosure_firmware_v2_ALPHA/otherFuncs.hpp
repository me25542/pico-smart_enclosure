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

#ifndef OTHERFUNCS_HPP
#define OTHERFUNCS_HPP


/**
* @brief a simple function to blink the built-in LED
*/
void blinkLED(unsigned long blinkTime);

/**
* @brief tries to obtain the I2C mutex, sets mode to error if it times out
* @param timeout_id stored in errorInfo on timeout
*/
bool useI2C(uint16_t timeout_id);

/**
* @brief tries to obtain the I2C mutex, sets mode to error if it times out
* @param timeout_id stored in errorInfo on timeout
* @param timeout_info also stored in errorInfo on timeout
*/
bool useI2C(uint16_t timeout_id, uint16_t timeout_info);

/**
* @brief releases the I2C mutex
*/
void doneWithI2C();

/**
* @brief checks if a given I2C address is responsive on the bus (I2C0)
*/
bool isI2CDeviceConnected(uint8_t address);

/**
* @brief checks if all temp sensors are connected / responsive
*/
bool areSensorsPresent();

/**
* @brief checks if the screen is responsive on the I2C bus
*/
bool isScreenConnected();

/**
* @brief sets the state of the PSU (on or off) | returns true on sucess, false on failure
*/
bool setPSU(bool state);

/**
* @brief sets the position of the servos
*/
void setServos(uint8_t s1_Pos, uint8_t s2_Pos);

/**
* @brief sets the state of booth heaters, and the fan (the temp-related stuff)
*/
void setHeaters(bool h1_On, bool h2_On, uint8_t fanVal);

/**
* @brief starts serial (USB) comunication (takes ~10ms) | returns 1 if a computer is connected, 0 if not
*/
bool startSerial();

/**
* @brief gets all temp sensor data | returns 0 on failure, 1 on sucess
*/
bool getTemp();

/**
* @brief called if the lightswitch was pressed. does the stuff that should be done when this happens
*/
void lightswitchPressed();

/**
* @brief called when a manual cooldown is triggered (the cooldown button is released)
*/
void manualCooldown();

/**
* @brief called when the door opens (the door switch is released)
*/
void doorOpening();

/**
* @brief called when the door closes (the door switch is pressed)
*/
void doorClosing();

/**
* @brief checks if the buttons have been pressed or relesed, and calls the correct functions based on the state of the buttons
*/
void checkButtons();

#endif