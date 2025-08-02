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
#include "vars.hpp"

/**
* @brief the function called on data requested from the printer
*/
void requestEvent();

/**
* @brief the function called to parse each byte that is marked as setting the temperature
*/
bool parseTemp(uint8_t recVal, uint8_t num, bool final);

/**
* @brief the function called to parse each byte that is marked as setting the mode
*/
bool parseMode(uint8_t recVal, uint8_t num, bool final);

/**
* @brief the function called to parse each byte marked as setting the max fan speed
*/
bool parseMaxFanSpeed(uint8_t recVal, uint8_t num, bool final);

/**
* @brief the function called to parse each byte that is marked as setting the lights
*/
bool parseLights(uint8_t recVal, uint8_t num, bool final);

/**
* @brief the function called to parse each byte that is marked as seting the print done state
*/
bool parsePrintDone(uint8_t recVal, uint8_t num, bool final);

/**
* @brief the function called to parse each byte that is marked as containing the print name
*/
bool parseName(uint8_t recVal, uint8_t num, bool final);

/**
* @brief parses each byte marked as setting the mode of control for the heater and fan
* @note developmental
*/
bool parseControlMode(uint8_t recVal, uint8_t num, bool final);

/**
* @brief parses each byte marked as setting the heater when the control mode is set to manual
* @note developmental
*/
bool parseHeater(uint8_t recVal, uint8_t num, bool final);

/**
* @brief parses each byte marked as setting the fan speed when the control mode is set to manual
* @note developmental
*/
bool parseFan(uint8_t recVal, uint8_t num, bool final);

/**
* @brief the function called to parse a byte the same as v1 would
*/
void compatabilityParser(uint8_t recVal);

/**
* @brief reads the next byte from the secondary I2C buffer. desined to be somwhat interchangable with Wire1.read()
*/
uint8_t read();

/**
* @brief returns the number of bytes available to read in the secondary I2C buffer
*/
uint8_t available();

/**
* @brief the function to handel parsing received I2C data
*/
void parseI2C();

/**
* @brief the ISR called on I2C data receive. stores all data in a secondary buffer
*/
void I2cReceived(int numBytes);

#if serialControll
/**
* @brief called to parse any data that might have been received over Serial (USB)
*/
void serialReceiveEvent();
#endif

/**
* @brief backs up all settings in the menu to flash
*/
inline void menuBackup();

#if !manualPressedState
/**
* @brief backs up the pressed states of the switches to flash
*/
inline void buttonBackup();
#endif

/**
* @brief backs up the version info of the running code to flash
*/
inline void versionBackup();

/**
* @brief stores wether or not the system was in error mode on power loss
*/
inline void errorBackup();

/**
* @brief the ISR called when the POK pin on the PSU signals an imanent power loss. if this is unexpected, will backup all settings and some other stuff to flash
*/
void losingPower();
