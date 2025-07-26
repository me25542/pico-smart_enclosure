/**
* @file constants.hpp
* @brief contains program-specific constants that aren't configurable | **MUST** BE PLACED *BEFORE* Arduino.h
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

#ifndef USE_TINYUSB // IDK why this is needed, but it prevented redefinition
#define USE_TINYUSB // inportant for debugging to keep this how it is unless you *realy* know what you are doing
#endif

#define MODE_ERROR 0
#define MODE_STANDBY 1
#define MODE_COOLDOWN 2
#define MODE_PRINTING 3

#define MAX_MODE 4 // the value at which the mode parser will start to ignore the sent byte

#define DEFAULT_MODE MODE_STANDBY

#define CHARACTER_HEIGHT 8 ///< the height of a size 1 character
#define CHARACTER_WIDTH 6 ///< the width of a size 1 character

#define CONTROL_MODE_ERROR 0
#define CONTROL_MODE_TEMP 1 ///< maintain target temperature
#define CONTROL_MODE_MANUAL 2 ///< follow manual control

#define SERIAL_SPEED 115200 ///< the buad rate that will be used for serial communication
