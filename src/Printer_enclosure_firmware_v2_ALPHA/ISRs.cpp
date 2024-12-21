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

#include <Arduino.h>
#include <EEPROM.h>

#include "vars.hpp"

#include "ISRs.hpp"


void requestEvent() {
  Wire1.write(mode);  //  send (back to the printer) the current mode the enclosure is in

  #if debug
  printf("Printer requested data. Sending: %u", mode);  //  print a debug message over the sellected debug port
  #endif
}

bool parseTemp(uint8_t recVal, uint8_t num, bool final) {
  if ((minSetTemp < recVal) && (recVal < maxSetTemp)) {  //  if the recieved value is within the acceptable range
    globalSetTemp = recVal;  //  set the target temperature to the recieved value
    return true;  //  tell the calling function that we sucessfully set it
  } else {  //  if the received value is out of spec
    return false;  //  tell the calling function so
  }  //  else (  if (minSetTemp < recVal) && (recVal < maxSetTemp)  )
}  //  parseTemp()

bool parseMode(uint8_t recVal, uint8_t num, bool final) {
  if (recVal < maxMode) {  //  if the recieved value is within the acceptable range
    if (recVal == 0) {
      errorOrigin = 4;  //  record that the printer commanded the error
      errorInfo = mode;  //  record the old mode
    }
    
    mode = recVal;  //  set the mode
    return true;  //  tell the calling function that we sucessfully set it
  } else {  //  if the received value is out of spec
    return false;  //  tell the calling function so
  }  //  else (  if (recVal < maxMode)  )
}  //  parseMode()

bool parseMaxFanSpeed(uint8_t recVal, uint8_t num, bool final) {
  maxFanSpeed = recVal;
  return true;  //  tell the calling function something was set
}  //  parseMaxFanSpeed()

bool parseLights(uint8_t recVal, uint8_t num, bool final) {
  switch (recVal) {  //  check the received byte
    case 0:  //  if it is zero
      lightSetState = true;  //  remember to turn on the lights
      return true;  //  tell the calling function something was set
    
    case 1:  //  if it is one
      lightSetState = false;  //  remember to turn off the lights
      return true;  //  tell the calling function something was set
    
    case 2:  //  if it is two
      changeLights = true;  //  remember to change the lights
      return true;  //  tell the calling function something was set

    default:  //  if it is anything else
      return false;  //  tell the calling function nothing was set
  }  //  switch (recVal)
}  //  parseLights()

bool parsePrintDone(uint8_t recVal, uint8_t num, bool final) {
  switch (recVal) {  //  check the received byte
    case 0:  //  if it is zero
      printDone = false;  //  remember that the print isn't done
      return true;  //  tell the calling function that something was set
    
    case 1:  //  if it is one
      printDone = true;  //  remember that the print is done
      return true;  //  tell the calling function something was set
    
    default:  //  if it is anything else
      return false;  //  tell the calling function nothing was set
  }  //  switch (recVal)
}  //  parsePrintDone()

bool parseName(uint8_t recVal, uint8_t num, bool final) {
  //static bool longName;  //  stores weather or not the command specifies a starting point
  uint8_t printChar = 255;  //  the point in the name to start writing to

  for (uint16_t i = 0; i <= 255; i++) {  //  go through up to all the bytes in the print name
    if (printName[i] == 0) {  //  if we have reached the end of the populated part of the name (we found a NULL)
      printChar = i;  //  set where to start writing to
      break;  //  exit the FOR loop
    }
  }

  printName[printChar] = recVal;  //  set a character in the print name to the recieved value

  if (final) {  //  if this is the last character in the name
    for (uint8_t i = (printChar + 1); i != 255; i++) {  //  for each remaining space in the print name
      printName[i] = 0;  //  set a space in the print name to NULL
    }  //  for (uint8_t i = (num + 1); i < 256; i++)
  }  //  if (final)

  return true;  //  tell the calling function that something was set
}  //  parseName()

void compatabilityParser(uint8_t recVal) {
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
    maxFanSpeed = static_cast<uint8_t>(map(long(recVal), 100, 104, 0, 255));  //  set maxFanSpeed to a value from 0-255 based on the recieved value
  
  }
}

void parseI2C() {
  checkI2c = false;

  #if debug  //  if we are debuging
  Serial.printf("parseI2C() called.\n");  //  print a debug message over the sellected debug port

  #endif  //  end of that IF statement
  
  static bool (*parsers[])(uint8_t, uint8_t, bool) = {parseMode, parseTemp, parsePrintDone, parseMaxFanSpeed, parseLights, parseName};  //  an array of functions to call
  const static uint8_t parserCount = sizeof(parsers) / sizeof(parsers[0]);  //  find the number of parser functions (at compile, not during runtime)

  while (I2cBuffer.available()) {  //  repeat for all commands sent by the printer
    uint8_t inVal = I2cBuffer.read();  //  get the first byte in the buffer

    #if debug
    Serial.printf("parsing new command: %u\n", inVal);  //  print a debug message over the sellected debug port
    #endif

    if (inVal < 105) {  //  if the byte is a valid command for v1
      #if debug
      Serial.printf("parsing command in compatability mode.\n");  //  print a debug message over the sellected debug port
      #endif

      compatabilityParser(inVal);  //  parse it in compatability mode

    } else {  //  if the command isn't a valid v1 command
      #if debug
      Serial.printf("parsing command in normal mode.\n");  //  print a debug message over the sellected debug port
      #endif

      uint8_t commandType = ~inVal;  //  set the command type to the the value of inVal with all bits flipped (255 becomes 0, 254 becomes 1, etc.)
        //  we do this so that v1 commands (0-104) are still valid, but we can use the bitflipped value for the index of an array (starting at 0)

      if (commandType < parserCount) {  //  if the command type is valid (if it is less than the number of parser functions)
        if (I2cBuffer.available()) {  //  if there are bytes left in the buffer
          uint8_t numBytes = I2cBuffer.read();  //  get the number of commands after this to look in for the command | if this is zero, the function will exit

          for (uint8_t i = 0; i < numBytes; i++) {  //  repeat this for the number of bytes we want to look for the data in
            if (I2cBuffer.available()) {  //  if there are bytes left in the buffer
              uint8_t recVal = I2cBuffer.read();  //  find the next byte in the command (the first remaining byte in the buffer)
              bool isLastByte = (i == (numBytes - 1));  //  set isLastByte to true if i is equal to the last byte in the command

              parsers[commandType](recVal, i, isLastByte);  //  call the correct parser function with the data it needs

            } else {  //  if there arn't any bytes left in the buffer (something is wrong)

              #if debug
              Serial.printf("No bytes left.\n");
              #endif

              return;  //  exit the function
            }  //  else (  if (Wire1.available())  )
          }  //  for (uint8_t i = 0; i < numBytes; i++)

        } else {  //  if there arn't any bytes left in the buffer (something is wrong)

          #if debug
          Serial.printf("No bytes left.\n");
          #endif
          
          return;  //  exit the function
        }  //  else (  if (Wire1.available())  )
      }  //  if (commandType < parserCount)
    }  //  else (  if (inVal < 105)  )
  }  //  while (Wire1.available())

  #if debug
  Serial.printf("Exiting parseI2C().\n");
  #endif
}  //  parseI2C()


void I2cReceived(int numBytes) {
  while (Wire1.available()) {  //  write each byte in the buffer to another buffer
    I2cBuffer.write(Wire1.read());
  }

  checkI2c = true;
}


void serialReceiveEvent() {
  #if debug
  printf("serialRecieveEvent() called.\n");  //  print a debug message over the sellected debug port
  #endif
  if (Serial.available()) {  //  if there are received bytes in the buffer:
    uint8_t serialRecVal = static_cast<uint8_t>(Serial.parseInt());  // set serialRecVal ("it") to the first valid set of characters in the buffer, converted from a string of ASCII chars to an int
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
      maxFanSpeed = static_cast<uint8_t>(map(static_cast<long>(serialRecVal), 100, 104, 0, 255));  //  set maxFanSpeed to a value from 0-255 based on the recieved value
    
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

inline void menuBackup() {
  EEPROM.write(0, 0x00);  //  we don't want to use the data unless everything is written

  uint16_t memAdr = 1024;  //  start with the adress at the start of the second kb of memory
      
  for (uint8_t i = 0; i < menuLength; i++) {
    switch (mainMenu[i].getDataType()) {
      case 0: {
        EEPROM.write(memAdr++, mainMenu[i].menuItemToBool() * 255);

        break;
      }
      
      case 1: {
        EEPROM.write(memAdr++, mainMenu[i].menuItemToUInt8());

        break;
      }
      
      case 2: {
        EEPROM.write(memAdr++, mainMenu[i].menuItemToInt8());

        break;
      }
      
      case 3: {
        uint16_t menuData = mainMenu[i].menuItemToUInt16();
        uint8_t second_byte = menuData & 0xFF;  //  get the 8 least significant bits
        uint8_t first_byte = (menuData >>= 8) & 0xFF;  //  get the 8 next least siginificant bits

        EEPROM.write(memAdr++, first_byte);
        EEPROM.write(memAdr++, second_byte);

        break;
      }
      
      case 4: {
        int16_t menuData = mainMenu[i].menuItemToInt16();
        uint8_t second_byte = menuData & 0xFF;  //  get the 8 least significant bits
        uint8_t first_byte = (menuData >>= 8) & 0xFF;  //  get the 8 next least siginificant bits

        EEPROM.write(memAdr++, first_byte);
        EEPROM.write(memAdr++, second_byte);

        break;
      }
      
      case 5: {
        uint32_t menuData = mainMenu[i].menuItemToUInt32();
        uint8_t fourth_byte = menuData & 0xFF;  // Extract the 8 least significant bits
        uint8_t third_byte = (menuData >>= 8) & 0xFF;  // Extract the next 8 bits
        uint8_t second_byte = (menuData >>= 8) & 0xFF;  // Extract the next 8 bits
        uint8_t first_byte = (menuData >>= 8) & 0xFF;  // Extract the most significant 8 bits

        EEPROM.write(memAdr++, first_byte);
        EEPROM.write(memAdr++, second_byte);
        EEPROM.write(memAdr++, third_byte);
        EEPROM.write(memAdr++, fourth_byte);

        break; 
      }
      
      case 6: {
        int32_t menuData = mainMenu[i].menuItemToInt32();
        uint8_t fourth_byte = menuData & 0xFF;  // Extract the 8 least significant bits
        uint8_t third_byte = (menuData >>= 8) & 0xFF;  // Extract the next 8 bits
        uint8_t second_byte = (menuData >>= 8) & 0xFF;  // Extract the next 8 bits
        uint8_t first_byte = (menuData >>= 8) & 0xFF;  // Extract the most significant 8 bits

        EEPROM.write(memAdr++, first_byte);
        EEPROM.write(memAdr++, second_byte);
        EEPROM.write(memAdr++, third_byte);
        EEPROM.write(memAdr++, fourth_byte);

        break; 
      }  //  case 6:
    }  //  switch (mainMenu[i].getDataType())
  }  //  for (uint8_t i = 0; i < menuLength; i++)

  EEPROM.write(0, 0xFF);  //  now we can use the data
}  //  menuBackup()

#if !manualPressedState
inline void buttonBackup() {
  EEPROM.write(9, 0x00);  //  remember to use the stored button data. We assume that the pressed states of the buttons are corect at this point

  EEPROM.write(10, static_cast<uint8_t>(doorSw_ps) * 255);
  EEPROM.write(11, static_cast<uint8_t>(lightSw_ps) * 255);
  EEPROM.write(12, static_cast<uint8_t>(coolDownSw_ps) * 255);
  EEPROM.write(13, static_cast<uint8_t>(sellSw_ps) * 255);
  EEPROM.write(14, static_cast<uint8_t>(upSw_ps) * 255);
  EEPROM.write(15, static_cast<uint8_t>(downSw_ps) * 255);
}
#endif

inline void versionBackup() {
  EEPROM.write(1, majorVersion);
  EEPROM.write(2, minorVersion);
  EEPROM.write(3, bugFixVersion);
  EEPROM.write(4, buildVersion);
}

void losingPower() {
  if (PSUIsOn && !bootsel && mode != 0) {  //  if the PSU should be on (if we are losing power) AND the BOOTSEL button isn't pressed AND the mode isn't error
    if (saveStateOnPowerLoss) {  //  if we are saving the state
      //  turn stuff off
      digitalWrite(lightsPin, !mainLightsOn);  //  turn off the lights
      digitalWrite(fanPin, !fanOn);  //  turn off the fan
      digitalWrite(heater1Pin, HIGH);  //  turn off heater 1
      digitalWrite(heater2Pin, HIGH);  //  turn off heater 2
      digitalWrite(psPin, LOW);  //  turn off the PSU

      //  store the version info of the code that saved the info (we don't want to use data from incompatible past versions)
      versionBackup();

      #if !manualPressedState
      buttonBackup();
      #endif

      menuBackup();

      EEPROM.commit();  //  commit changes to memory. we don't check if this was sucessful, because, well, what would we do if it wasn't?

    } else {  //  if we do not save the state on power loss
      if (static_cast<bool>(EEPROM.read(0))) {  //  if the memory currently says to use the stored data
        EEPROM.write(0, 0x00);  //  remember to not use any of the stored menu data
        EEPROM.write(9, 0xFF);  //  remember to not use any of the stored button data
        EEPROM.commit();  //  commit changes to memory. we don't check if this was sucessful, because, well, what would we do if it wasn't?
      }  //  if (static_cast<bool>(EEPROM.read(0)))
    }  //  else (  if (saveStateOnPowerLoss)  )
  }  //  if (PSUIsOn && !BOOTSEL)
}  //  void losingPower()
