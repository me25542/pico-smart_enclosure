/*
 * Writen by Dalen Hardy 2024
 * 
 * This code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * In no event shall the author be liable for any claim, damages, or other liability, 
 * whether in an action of contract, tort, or otherwise, arising from, out of, or in connection with the code 
 * or the use or other dealings in the code.
 * 
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

    } else {
      mode = 0;
      errorOrigin = 5;
      errorInfo = recVal;
    }
  }
}

void serialReceiveEvent() {
  Serial.println("serialRecieveEvent() called");  //  print a message over serial (USB)
  while (Serial.available()) {  //  while there are received bytes in the buffer:
    int serialRecVal = Serial.read();  // set serialRecVal ("it") to the first byte in the buffer
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

    } else {
      mode = 0;
      errorOrigin = 8;
      errorInfo = serialRecVal;
    }
  }
}

#endif