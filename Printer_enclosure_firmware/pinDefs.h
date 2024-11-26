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

#ifndef PINDEFS_H
#define PINDEFS_H


  //  Pin definitions:
  //  outputs, high pin# - low pin#
  pinMode(ledPin, OUTPUT);
  pinMode(printDoneLightPin, OUTPUT);
  pinMode(printingLightPin, OUTPUT);
  pinMode(cooldownLightPin, OUTPUT);
  pinMode(standbyLightPin, OUTPUT);
  pinMode(errorLightPin, OUTPUT);
  pinMode(lightsPin, OUTPUT);
  pinMode(heater2Pin, OUTPUT);
  pinMode(heater1Pin, OUTPUT);
  pinMode(psPin, OUTPUT);
  pinMode(fanPin, OUTPUT);

  //  inputs, high pin# - low pin#
  pinMode(pokPin, INPUT_PULLDOWN);

  // set initial output pin values, high pin# - low pin#:
  digitalWrite(ledPin, LOW);  //  turn off the built-in LED
  digitalWrite(printDoneLightPin, LOW);  //  turn off the "print done" light
  digitalWrite(printingLightPin, LOW);  //  turn off the "printing" light
  digitalWrite(cooldownLightPin, LOW);  //  turn off the "cooldown" light
  digitalWrite(standbyLightPin, LOW);  //  turn off the "standby" light
  digitalWrite(errorLightPin, LOW);  //  turn off the "error" light
  digitalWrite(lightsPin, LOW);  //  turn off the enclosure lights
  digitalWrite(heater1Pin, HIGH);  //  turn off heater 1
  digitalWrite(heater2Pin, HIGH);  //  turn off heater 2
  digitalWrite(fanPin, LOW);  //  turn off the fan
  digitalWrite(psPin, HIGH);  //  turn on the power supply

#endif