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

#include "Adafruit_MCP9808.h"
#include "config.hpp"
#include <Arduino.h>
#include <pico/stdlib.h>
#include <Wire.h>
#include <Bounce2.h>
#include "vars.hpp"

/**
* @brief handels setting the mode to error and recording data about the error in flash
*/
void setError(uint8_t origin, uint32_t info, bool recoverable);

/**
* @brief converts a number to base 4, encoded as a std::vector with index [0] as the least significant digit
*/
std::vector<uint8_t> base2ToBase4(uint8_t inut);

/**
* @brief handels the blinking of the built-in LED to indicate status
*/
void blinkLED();

/**
* @brief blinks out an error code through the built-in LED
*/
void blinkErrorCode(uint8_t code);

/**
* @brief tries to obtain the I2C mutex, sets mode to error if it times out
* @param timeoutId stored in errorInfo on timeout
*/
bool useI2C(uint16_t timeoutId);

/**
* @brief tries to obtain the I2C mutex, sets mode to error if it times out
* @param timeout_id stored in errorInfo on timeout
* @param timeout_info also stored in errorInfo on timeout
*/
bool useI2C(uint16_t timeoutId, uint16_t timeout_info);

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
* @brief call every loop, handles updating the fan
* @note non-critical info is only updated periodically (interval defined on config file)
*/
void updateFan();

/**
* @brief sets the speed (duty cycle) of the fan
*/
void setFan(uint8_t dutyCycle);

/**
* @brief starts serial (USB) comunication (takes ~10ms) | returns 1 if a computer is connected, 0 if not
*/
bool startSerial();

/**
* @brief reads the temperature from a sensor, and returns the result in fixed-point formating
 */
uint32_t readTempSensor(Adafruit_MCP9808* sensor, uint8_t sensorID, uint8_t SCALE_MULT, uint8_t SCALE_SHIFT, uint16_t scaledMaxTemp);

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

/**
* @brief sets the state of most GPIO pins to a disconnected, high-impedance state. Exceptions: error light and built-in LED
*/
void gpioDisable();

/**
* @brief handels logic for controling the heaters and fan when the control mode is set to controlMode_TEMP
*/
void heatingLogic_temp();

/**
* @brief handels the logic for controling the heaters and fan when the control mode is set to controlMode_MANUAL
*/
void heatingLogic_manual();
