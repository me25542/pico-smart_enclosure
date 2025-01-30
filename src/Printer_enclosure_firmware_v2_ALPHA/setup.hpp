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

#ifndef SETUP_HPP
#define SETUP_HPP


/**
* @brief initializes non-const variables to there initial values
*/
void varInit();

/**
* @brief attaches the servos, moves them to closed position
*/
void servoSetup();

/**
* @brief tries to start the temp sensors, checks if they are rsponsive, gets the temp. returns 1 on sucess, 0 on failure
*/
bool tempSensorSetup();

/**
* @brief sets up I2C0, or the one connected to the printer
*/
void printerI2cSetup();

/**
* @brief sets up the buttons
*/
void buttonSetup();

/**
* @brief sets up a bunch of pins with their pinModes and startup states
*/
void pinSetup();

/**
* @brief attempts to start the screen, and verifies it is responsive on the I2C bus. If sucessfull, displays a "Loading..." splash screen
*/
bool menuSetup();

/**
* @brief returns true if the stored version info matches that of the running code, false if it doesn't
*/
bool checkVersion();

#if !manualPressedState
/**
* @brief updates the pressed states of the switches from stored data
*/
bool buttonRecovery();
#endif

/**
* @brief returns true if the system was not in error, false if it was
*/
inline bool checkError();

/**
* @brief updates the data of all menu items from stored data
*/
bool menuRecovery();

/**
* @brief sets the data of all the menu items to that stored in EEPROM after a power outage
*/
void backupRecovery();

/**
* @brief starts serial (USB) communication | Call from the core you want to handel serial comunication
*/
void serialSetup();

#endif