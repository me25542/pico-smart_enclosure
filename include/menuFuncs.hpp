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
#include <cstdint>
#include "vars.hpp"
#include "otherFuncs.hpp"

/**
* @brief clears the print name variable
*/
void clearName();

/**
* @brief handels scroling of the print name
*/
void scrollName(uint8_t height);

/**
* @brief the lower-level function that prints the header on the screen
*/
void printHeader();

/**
* @brief Prints one menu item
* @param index the index of the menu item in the main menu
* @param height the height to print the menu item at, in pixels from the top of the screen
* @param onTop whether or not this is the top-most menu item
*/
void PrintMenuItem(uint8_t index, uint8_t height, bool topItem);

/**
* @brief the lower-level function that prints the menu to the screen
* @param startHeight the height at which to start printing the menu items, in pixels from the top of the screen
* @param lower_bound the lower numarical bound (inclusive) of the range of menu items to be printed (goes at the top of the screen)
* @param upper_bound the upper numarical bound (inclusive) of the range of menu items to be printed (goes at the bottom of the screen)
*/
bool printMenu(uint8_t startHeight, uint8_t lowerBound, uint8_t uperBound);

/**
* @brief prints a screensaver on the entire screen
*/
void printScreensaver();

/**
* @brief the higher-level function called whenever the screen needs to be updated
*/
void updateScreen();

/**
* @brief the function called when the "sellect" menu button is pressed
*/
void sell_switch_Pressed();

/**
* @brief the function called when the "up" menu button is pressed
*/
void up_switch_Pressed();

/**
* @brief the function called when the "down" menu button is pressed
*/
void down_switch_Pressed();

/**
* @brief the function that checks the buttons related to the screen (UI) and calls the apropriet functions. These other functions handle the logic of navigating the menu.
*/
void checkMenuButtons();

/**
* @brief updates the backup copy of menu data
*/
void updateMenuBackup();

/**
* @brief updates the menu backup if it has been long enough since the last backup
*/
bool periodicMenuBackup();
