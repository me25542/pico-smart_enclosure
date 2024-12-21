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

#ifndef MENUFUNCS_HPP
#define MENUFUNCS_HPP

/**
* @brief clears the print name variable
*/
void clearName();

/**
* @brief handels scroling of the print name. call as often as possible
*/
void scrollName(uint8_t height);

/**
* @brief the lower-level function that prints the header on the screen
*/
void printHeader();

/**
* @brief the lower-level function that actually handels printing syuff to the screen | working on making this *only* print the menu
*/
void printMenu(int32_t lower_Bound, int32_t uper_Bound, bool dispName);

/**
* @brief the higher-level function called whenever the screen needs to be updated
*/
void updateScreen();

/**
* @brief the function called when the "sell." menu button is pressed
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

#endif