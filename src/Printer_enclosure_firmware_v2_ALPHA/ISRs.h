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

#ifndef ISRS_H
#define ISRS_H

//#define debug

//  the function called on data requested:
void requestEvent();

//  the function called to parse each byte that is marked as setting the temperature
bool parseTemp(uint8_t recVal, uint8_t num, bool final);

//  the function called to parse each byte that is marked as setting the mode
bool parseMode(uint8_t recVal, uint8_t num, bool final);

//  the function called to parse each byte marked as setting the max fan speed
bool parseMaxFanSpeed(uint8_t recVal, uint8_t num, bool final);

//  the function called to parse each byte that is marked as setting the lights
bool parseLights(uint8_t recVal, uint8_t num, bool final);

//  the function called to parse each byte that is marked as seting the print done state
bool parsePrintDone(uint8_t recVal, uint8_t num, bool final);

//  the function called to parse each byte that is marked as containing the print name
bool parseName(uint8_t recVal, uint8_t num, bool final);

//  the function called to parse the received byte the same as v1 would
void compatabilityParser(uint8_t recVal);

//  the function called on data received
void receiveEvent(int howMany);

void I2cReceived(int unused);

void serialReceiveEvent();

void losingPower();


#endif