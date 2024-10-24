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

#ifndef BUTTONSETUP_H
#define BUTTONSETUP_H

    //  set switch pinModes
  door_switch.attach(doorSwitchPin, INPUT_PULLUP);
  light_switch.attach(lightSwitchPin, INPUT_PULLUP);
  coolDown_switch.attach(coolDownSwitchPin, INPUT_PULLUP);

  //  set switch debounce intervals
  door_switch.interval(100);
  light_switch.interval(100);
  coolDown_switch.interval(100);

  //  set switch pressed states
  door_switch.setPressedState(LOW);
  light_switch.setPressedState(LOW);
  coolDown_switch.setPressedState(LOW);

#endif