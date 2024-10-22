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

#include "otherFuncs.h"  //  this is just for ease in writing; the include guards will block this from compiling

//  the function called on data requested:
void requestEvent() {
  Wire1.write(mode);  //  send (back to the printer) the current mode the enclosure is in
  Serial.print("Printer requested data. Sending ");
  Serial.println(mode);
}

//  the function called on data received:
void receiveEvent(int howMany) {
  Serial.println("recieveEvent() called");  //  print a message over serial (USB)
  while (Wire1.available()) {  //  while there are received bytes in the buffer:
    int recVal = Wire1.read();  // set recVal ("it") to the first byte in the buffer
    Serial.print("byte recieved: ");  //  print a message over serial (USB)
    Serial.println(recVal);  //  print a message over serial (USB)

    if (recVal < 4) {  //  if it is 0-3:
      if (recVal == 0) {
        errorOrigin = 4;  //  record that the printer commanded the error
        errorInfo = mode;  //  record the old mode
      }
      mode = recVal;  //  set the mode

    } else if (recVal < 10) {  //  if it is 4-9:
      //  set other things:
      switch (recVal) {  //  switch... case statement; checks the value of recVal and does the coresponding action
        case 4:  //  if it is 4:
          printDone = true;  //  set print done
          break;

        case 5:  //  if it is 5:
          printDone = false;  //  set print not done
          break;

        case 6:  //  if it is 6:
          lightSetState = true;  //  remember to turn the lights on
          break;

        case 7:  //  if it is 7:
          lightSetState = false;  //  remember to turn the lights off
          break;

        case 8:
          changeLights = true;  //  remember to change the state of the lights
          break;
      }

    } else if (recVal < 100) {  //  if it is 10-99:
      globalSetTemp = recVal;  //  set the target temp

    } else if (recVal < 105) {  //  if it is 100-104:
      maxFanSpeed = int(map(long(recVal), 100, 104, 0, 255));  //  set maxFanSpeed to a value from 0-255 based on the recieved value
    
    } else {
      mode = 0;
      errorOrigin = 5;
      errorInfo = recVal;
    }
  }
}

void serialReceiveEvent() {
  Serial.println("serialRecieveEvent() called");  //  print a message over serial (USB)
  if (Serial.available()) {  //  if there are received bytes in the buffer:
    int serialRecVal = int(Serial.parseInt());  // set serialRecVal ("it") to the first valid set of characters in the buffer, converted from a string of ASCII chars to an int
    Serial.print("byte recieved: ");  //  print a message over serial (USB)
    Serial.println(serialRecVal);  //  print a message over serial (USB)

    if (serialRecVal < 4) {  //  if it is 0-3:
      if (serialRecVal == 0) {
        errorOrigin = 7;  //  record that the printer commanded the error
        errorInfo = mode;  //  record the old mode
      }
      mode = serialRecVal;  //  set the mode

    } else if (serialRecVal < 10) {  //  if it is 4-9:
      //  set other things:
      switch (serialRecVal) {  //  switch... case statement; checks the value of recVal and does the coresponding action
        case 4:  //  if it is 4:
          printDone = true;  //  set print done
          break;

        case 5:  //  if it is 5:
          printDone = false;  //  set print not done
          break;

        case 6:  //  if it is 6:
          lightSetState = true;  //  remember to turn the lights on
          break;

        case 7:  //  if it is 7:
          lightSetState = false;  //  remember to turn the lights off
          break;

        case 8:
          changeLights = true;  //  remember to change the state of the lights
          break;
      }

    } else if (serialRecVal < 100) {  //  if it is 10-99:
      globalSetTemp = serialRecVal;  //  set the target temp

    } else if (serialRecVal < 105) {  //  if it is 100-104:
      maxFanSpeed = int(map(long(serialRecVal), 100, 104, 0, 255));  //  set maxFanSpeed to a value from 0-255 based on the recieved value
    
    } else {
      mode = 0;
      errorOrigin = 8;
      errorInfo = serialRecVal;
    }

    //  empty the rest of the buffer:
    while (Serial.available()) {  //  while there are bytes in the buffer:
      Serial.read();  //  read a byte from the buffer (don't realy *do* anything with it, though)
    }
  }
}

#endif