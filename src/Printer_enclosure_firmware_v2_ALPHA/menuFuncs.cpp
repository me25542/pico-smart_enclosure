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
#include "vars.h"
#include "menuFuncs.h"

void scrollName(uint8_t height) {
  #ifdef debug
  uint8_t core = rp2040.cpuid();
  printf("scrollName(%u) called from core%u.\n", height, core);  //  print a debug message over the sellected debug port
  #endif

  static uint8_t pos = 6;
  static uint8_t startChar = 0;
  static uint32_t time = millis();

  if (! dispLastLoop) {  //  if the print name wasn't displayed last loop
    //  reset the variables to their start values
    pos = 6;
    startChar = 0;
    time = millis();
  }

  if (millis() - time > nameScrollSpeed) {  //  if enough time has elapsed sience the last movement of the cursor
    time = millis();  //  set the new time

    if (pos == 0) {  //  if the first character is all the way to the left
      pos = 5;  //  set the first character to one character's width from the left
      startChar++;  //  move the firs character to be displayed to the next one

    } else {  //  if the first character isn't all the way to the left
      pos--;  //  move it one pixel to the left
    }  //  else (  if (pos == 0)  )
  }  //  if (millis() > time + 10)

  if (printNameLength - 20 < startChar) {  //  if the end of the text is visible
    startChar = 0;  //  reset back to the start of the text
  }

  display.setCursor(pos, height);  //  set the cursor

  uint8_t endChar = min(startChar + 20, printNameLength);  //  set the last character to be printed

  //  draw the visible text
  for (uint8_t i = startChar; i < endChar; i++) {  //  repeat for the visible part of the text
    display.print(printName[i]);  //  print one character

  }  //  for (uint8_t i = startChar; i < printNameLength; i++)
}  //  scrollName()


void printMenu(uint8_t lower_Bound, uint8_t uper_Bound, bool dispName) {
  #ifdef debug
  uint8_t core = rp2040.cpuid();
  printf("printMenu(%u, %u, %d) called from core%u.\n", lower_Bound, uper_Bound, dispName, core);  //  print a debug message over the sellected debug port
  #endif

  uint8_t vertPos = 16;  //  set the vertical cursor position (used later)
  display.clearDisplay();  //  clear the dispaly's buffer
  display.setTextSize(2);  //  2x normal size text
  display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);  //  whit text on a black background
  display.setCursor(0, 0);  //  set the cursor in the upper-left hand corner of the display
  display.print(modeStrings[mode]);  //  print the mode (formated in a lovely string, not a number)
  display.setCursor(62, 0);  //  move the cursor to near the right-hand side of the display (still at the top)
  display.print(static_cast<int8_t>(inTemp));  //  print the inside temperature
  display.print("/");  //  realy? you can't figure this one out? it prints a "/"
  display.print(globalSetTemp);  //  print the target temperature
  display.setTextSize(1);
  display.setCursor(122, 0);
  display.print("o");  //  print a "o"

  display.setTextSize(1);  //  set the text size to normal

  if (dispName) {  //  if we are displaying the print name
    scrollName(vertPos);  //  do the scroll name function, it will handel, well, scrolling the print name
    dispLastLoop = true;
    vertPos += lineSpacing;  //  move the vertical position
  } else {
    dispLastLoop = false;
  }

  for (uint8_t i = lower_Bound; i < uper_Bound; i++) {  //  a FOR loop that will draw each menu item in turn...
    display.setCursor(0, vertPos);  //  set the cursor to the left-hand side of the screen, at the correct height (remember earlier?)
    uint8_t dataType = mainMenu[i].getDataType();  //  find out what kind of data should be displayed for this menu item (is it a true/false value, an intager, etc.?)

    if (i == selectedItem) {  //  if this menu item is the one that is selected:
      if (editingMenuItem) {  //  if we are editing the value in it (if it is clicked on)
        display.setTextColor(SSD1306_BLACK, SSD1306_WHITE);  //  black text on a white background

      } else {  //  if we are not editing the value in it (if it is not clicked on)
        display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);  //  white text on a black background
        display.print("> ");  //  kinda obviuse what this does (print a ">")
      }

    } else {  //  if this menu item is not the one that is selected:
      display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);  //  white text on a black background
    }

    display.print(mainMenu[i].getName());  //  print this item's name (stored in the array mainMenu of the class menuItem)
    display.setCursor(104, vertPos);  //  set the cursor near the right side of the screen (same height)

    switch (dataType) {  //  a switch... case statement for the data type ("it")
      case 0: {  //  if it is zero (a bool)
        if (editingMenuItem && i == selectedItem) {  //  if we are editing it (if it is clicked on)
          display.print(tmp_bool);  //  print the preview value of the variable

        } else {  //  otherwise (if it is not clicked on)
          display.print(mainMenu[i].menuItemToBool());  //  print the value of the variable pointed to by the menu item
        }
        break;  //  exit the switch... case statement
      }  //  case 0
      
      case 1: {  //  if it is one (an unsigned byte)
        if (editingMenuItem && i == selectedItem) {  //  if we are editing it (if it is clicked on)
          display.print(tmp_uint8t);  //  print the preview value of the variable

        } else {  //  otherwise (if it is not clicked on)
          display.print(mainMenu[i].menuItemToUInt8());  //  print the value of the variable pointed to by the menu item
        }
        break;  //  exit the switch... case statement
      }  //  case 1
      
      case 2: {  //  if it is two (a signed byte)
        if (editingMenuItem && i == selectedItem) {  //  if we are editing it (if it is clicked on)
          display.print(tmp_int8t);  //  print the preview value of the variable

        } else {  //  otherwise (if it is not clicked on)
          display.print(mainMenu[i].menuItemToInt8());  //  print the value of the variable pointed to by the menu item
        }
        break;  //  exit the switch... case statement
      }  //  case 2
        
      case 3: {  //  if it is three (an unsigned 16-bit integer)
        if (editingMenuItem && i == selectedItem) {  //  if we are editing it (if it is clicked on)
          display.print(tmp_uint16t);  //  print the preview value of the variable

        } else {  //  otherwise (if it is not clicked on)
          display.print(mainMenu[i].menuItemToUInt16());  //  print the value of the variable pointed to by the menu item
        }
        break;  //  exit the switch... case statement
      }  //  case 3

      case 4: {  //  if it is four (a signed 16-bit integer)
        if (editingMenuItem && i == selectedItem) {  //  if we are editing it (if it is clicked on)
          display.print(tmp_int16t);  //  print the preview value of the variable

        } else {  //  otherwise (if it is not clicked on)
          display.print(mainMenu[i].menuItemToInt16());  //  print the value of the variable pointed to by the menu item
        }
        break;  //  exit the switch... case statement
      }

      case 5: {  //  if it is five (an unsigned 32-bit integer)
        if (editingMenuItem && i == selectedItem) {  //  if we are editing it (if it is clicked on)
          display.print(tmp_uint32t);  //  print the preview value of the variable

        } else {  //  otherwise (if it is not clicked on)
          display.print(mainMenu[i].menuItemToUInt32());  //  print the value of the variable pointed to by the menu item
        }
        break;  //  exit the switch... case statement
      }

      case 6: {  //  if it is six (a signed 32-bit integer)
        if (editingMenuItem && i == selectedItem) {  //  if we are editing it (if it is clicked on)
          display.print(tmp_int32t);  //  print the preview value of the variable

        } else {  //  otherwise (if it is not clicked on)
          display.print(mainMenu[i].menuItemToInt32());  //  print the value of the variable pointed to by the menu item
        }
        break;  //  exit the switch... case statement
      }
      
      default: {  //  if it isn't any of those (something went wrong)
        display.print("-");  //  print a "-" (duh)
        mode = 0;  //  set the mode to error
        errorOrigin = 11;  //  record why
        errorInfo = dataType;
        break;  //  exit the switch... case statement
      }  //  default
    }  //  switch... case

    vertPos += lineSpacing;  //  remember to set the cursor lower next iteration of the FOR loop
  }  //  FOR loop

  /*
  {
    uint32_t i;
    for (i = 0; i2c0InUse && i < maxI2CWaitTime; i++) {
      delayMicroseconds(1);
    }
    
    if (i >= maxI2CWaitTime) {
      mode = 0;
      errorOrigin = 13;
      errorInfo = 8;
      errorInfo = errorInfo << 16;
      printf("wait for I2C resource timeout.\n");
      return;
    }
  }

  i2c0InUse = true;
  */
  //mutex_enter_blocking(&I2C_mutex);  //  wait for I2C resources to be available
  {
    uint32_t i;
    for (i = 0; !mutex_try_enter(&I2C_mutex, &I2C_mutex_owner) && i < maxI2CWaitTime; i++) {
      delayMicroseconds(1);
    }

    if (i >= maxI2CWaitTime) {
      mode = 0;
      errorOrigin = 13;
      errorInfo = 8;
      errorInfo = errorInfo << 16;
      return;
    }
  }

  display.display();  //  write everything to the display

  //i2c0InUse = false;
  mutex_exit(&I2C_mutex);
}  //  printMenu()

//  the function called when the "sell." menu button is pressed
void sell_switch_Pressed() {
  #ifdef debug
  uint8_t core = rp2040.cpuid();
  printf("sell_switch_Pressed() called from core%u.\n", core);  //  print a debug message over the sellected debug port
  #endif

  if (editingMenuItem) {  //  if we are currently editing a item on the menu (before the button was pressed)
    uint8_t dataType = mainMenu[selectedItem].getDataType();  //  find out what kind of data was edited

    switch (dataType) {  //  based on the kind of data, do a different thing
      case 0:  //  if we were editing a bool
        mainMenu[selectedItem].boolToMenuItem(tmp_bool);  //  set the value of the variable tied to the menu item to the preview value we were editing
        break;  //  exit the switch... case statement
      
      case 1:  //  if we were editing an unsigned 8-bit integer
        mainMenu[selectedItem].uint8ToMenuItem(tmp_uint8t);  //  set the value of the variable tied to the menu item to the preview value we were editing
        break;  //  exit the switch... case statement
      
      case 2:  //  if we were editing a signed 8-bit integer
        mainMenu[selectedItem].int8ToMenuItem(tmp_int8t);  //  set the value of the variable tied to the menu item to the preview value we were editing
        break;  //  exit the switch... case statement
      
      case 3:  //  if we were editing an unsigned 16-bit integer
        mainMenu[selectedItem].uint16ToMenuItem(tmp_uint16t);  //  set the value of the variable tied to the menu item to the preview value we were editing
        break;  //  exit the switch... case statement

      case 4:  //  if we were editing a signed 16-bit integer
        mainMenu[selectedItem].int16ToMenuItem(tmp_int16t);  //  set the value of the variable tied to the menu item to the preview value we were editing
        break;  //  exit the switch... case statement
      
      case 5:  //  if we were editing an unsigned 32-bit integer
        mainMenu[selectedItem].uint32ToMenuItem(tmp_uint32t);  //  set the value of the variable tied to the menu item to the preview value we were editing
        break;  //  exit the switch... case statement
      
      case 6:  //  if we were editing a signed 32-bit integer
        mainMenu[selectedItem].int32ToMenuItem(tmp_int32t);  //  set the value of the variable tied to the menu item to the preview value we were editing
        break;  //  exit the switch... case statement
      
      default:  //  if the we can't find the data type (something is wrong)
        mode = 0;  //  set the mode to error
        errorOrigin = 11;  //  record why
        break;  //  exit the switch... case statement
    }  //  switch... case

    editingMenuItem = false;  //  we are no longer editing the menu item

  } else {  //  if we are not editing a menu item (before the button was pressed)
    editingMenuItem = true;  //  we should now be editing a menu item
    uint8_t dataType = mainMenu[selectedItem].getDataType();  //  get the type of data in the item we will be editing
    switch (dataType) {  //  based on the datatype, do something different:
      case 0:  //  if it is a bool
        tmp_bool = mainMenu[selectedItem].menuItemToBool();  //  set a preview value to the currrent value of the data pointed to by the item
        break;  //  exit the switch... case statement
      
      case 1:  //  if it is an unsigned 8-bit integer
        tmp_uint8t = mainMenu[selectedItem].menuItemToUInt8();  //  set a preview value to the currrent value of the data pointed to by the item
        break;  //  exit the switch... case statement
      
      case 2:  //  if it is a signed 8-bit integer
        tmp_int8t = mainMenu[selectedItem].menuItemToInt8();  //  set a preview value to the currrent value of the data pointed to by the item
        break;  //  exit the switch... case statement
        
      case 3:  //  if it is an unsigned 16-bit integer
        tmp_uint16t = mainMenu[selectedItem].menuItemToUInt16();  //  set a preview value to the currrent value of the data pointed to by the item
        break;  //  exit the switch... case statement
      
      case 4:  //  if it is a signed 16-bit integer
        tmp_int16t = mainMenu[selectedItem].menuItemToInt16();  //  set a preview value to the currrent value of the data pointed to by the item
        break;  //  exit the switch... case statement
      
      case 5:  //  if it is an unsigned 32-bit integer
        tmp_uint32t = mainMenu[selectedItem].menuItemToUInt32();  //  set a preview value to the currrent value of the data pointed to by the item
        break;  //  exit the switch... case statement
      
      case 6:  //  if it is a signed 16-bit integer
        tmp_int32t = mainMenu[selectedItem].menuItemToInt32();  //  set a preview value to the currrent value of the data pointed to by the item
        break;  //  exit the switch... case statement
      
      default:  //  if we couldn't find the datatype (somethings wrong)
        mode = 0;  //  set the mode to error
        errorOrigin = 11;  //  record why
        break;  //  exit the switch... case statement
    }  //  switch... case
  }  //  else (  if (editingMenuItem)  )
}  //  sell_switch_Pressed()

//  the function called when the "up" menu button is pressed
void up_switch_Pressed() {
  #ifdef debug
  uint8_t core = rp2040.cpuid();
  printf("up_switch_Pressed() called from core%u.\n", core);  //  print a debug message over the sellected debug port
  #endif

  if (editingMenuItem) {  //  if we are editing the data pointed to by an item
    uint8_t dataType = mainMenu[selectedItem].getDataType();  //  find out what type of data it is

    switch (dataType) {  //  based on the datatype, edit a different preview variable
      case 0:  //  if it is a bool
        tmp_bool = ! tmp_bool;  //  change the preview variable to the opposit of its current value
        break;  //  exit the switch... case statement
      
      case 1:  //  if it is an unsigned 8-bit integer
        tmp_uint8t += 1;  //  add one to the preview variable
        break;  //  exit the switch... case statement
      
      case 2:  //  if it is a signed 8-bit integer
        tmp_int8t += 1;  //  add one to the preview variable
        break;  //  exit the switch... case statement
      
      case 3:  //  if it is an unsigned 16-bit integer
        tmp_uint16t += 1;  //  add one to the preview variable
        break;  //  exit the switch... case statement
      
      case 4:  //  if it is a signed 16-bit integer
        tmp_int16t += 1;  //  add one to the preview variable
        break;  //  exit the switch... case statement
      
      case 5:  //  if it is an unsigned 32-bit integer
        tmp_uint32t += 1;  //  add one to the preview variable
        break;  //  exit the switch... case statement
      
      case 6:  //  if it is a signed 32-bit integer
        tmp_int32t += 1;  //  add one to the preview variable
        break;  //  exit the switch... case statement
      
      default:  //  if we couldn't find the datatype (somethings wrong)
        mode = 0;  //  set the mode to error
        errorOrigin = 11;  //  record why
        break;  //  exit the switch... case statement
    }  //  switch... case

  } else {  //  if we are not editing the data pointed to by a menu item (if none are "sellected" or clicked on)
    selectedItem--;  //  change the one that is sellected (but isn't actually "sellected" or clicked on)
  }  //  else (  if (editingMenuItem)  )
}  //  up_switch_Pressed()

//  the function called when the "down" menu button is pressed
void down_switch_Pressed() {
  #ifdef debug
  uint8_t core = rp2040.cpuid();
  printf("down_switch_Pressed() called from core%u.\n", core);  //  print a debug message over the sellected debug port
  #endif

  if (editingMenuItem) {  //  if we are editing the data pointed to by an item
    uint8_t dataType = mainMenu[selectedItem].getDataType();  //  find out what type of data it is

    switch (dataType) {  //  based on the datatype, edit a different preview variable
      case 0:  //  if it is a bool
        tmp_bool = ! tmp_bool;  //  change the preview variable to the opposit of its current value
        break;  //  exit the switch... case statement
      
      case 1:  //  if it is an unsigned 8-bit integer
        tmp_uint8t -= 1;  //  add one to the preview variable
        break;  //  exit the switch... case statement
      
      case 2:  //  if it is a signed 8-bit integer
        tmp_int8t -= 1;  //  add one to the preview variable
        break;  //  exit the switch... case statement
      
      case 3:  //  if it is an unsigned 16-bit integer
        tmp_uint16t -= 1;  //  add one to the preview variable
        break;  //  exit the switch... case statement
      
      case 4:  //  if it is a signed 16-bit integer
        tmp_int16t -= 1;  //  add one to the preview variable
        break;  //  exit the switch... case statement
      
      case 5:  //  if it is an unsigned 32-bit integer
        tmp_uint32t -= 1;  //  add one to the preview variable
        break;  //  exit the switch... case statement
      
      case 6:  //  if it is a signed 32-bit integer
        tmp_int32t -= 1;  //  add one to the preview variable
        break;  //  exit the switch... case statement
      
      default:  //  if we couldn't find the datatype (somethings wrong)
        mode = 0;  //  set the mode to error
        errorOrigin = 11;  //  record why
        break;  //  exit the switch... case statement
    }  //  switch... case

  } else {  //  if we are not editing the data pointed to by a menu item (if none are "sellected" or clicked on)
    selectedItem++;  //  change the one that is sellected (but isn't actually "sellected" or clicked on)
  }  //  else (  if (editingMenuItem)  )
}  //  down_switch_Pressed

//  the function that checks the buttons related to the screen (UI) and calls the apropriet functions. These other functions handle the logic of navigating the menu. The screen is then updated
void checkMenuButtons() {
  #ifdef debug
  uint8_t core = rp2040.cpuid();
  printf("checkMenuButtons() called from core%u.\n", core);  //  print a debug message over the sellected debug port
  #endif

  //  update the switches
  sell_switch.update();
  up_switch.update();
  down_switch.update();

  if (sell_switch.released()) {  //  if the "sell." button was released sience the last update
    sell_switch_Pressed();  //  call a function (can you guess which one?)
  }

  if (up_switch.released()) {  //  if the "up" button was released sience the last update
    up_switch_Pressed();  //  call a function (can you guess which one?)

  } else if (up_switch.isPressed() && (up_switch.currentDuration() >= menuButtonHoldTime)) {  //  otherwise (if the switch wasn't released sience the last update), if the switch is pressed and has been for a set length of time
    up_switch_Pressed();  //  call a function (can you guess which one?)
    delay(menuScrollSpeed);  //  wait for a set length of time (so the menu dosen't scroll by way to fast)
  }

  if (down_switch.released()) {  //  if the "down" button was released sience the last update
    down_switch_Pressed();  //  call a function (can you guess which one?)

  } else if (down_switch.isPressed() && (down_switch.currentDuration() >= menuButtonHoldTime)) {
    down_switch_Pressed();  //  call a function (can you guess which one?)
    delay(menuScrollSpeed);  //  wait for a set length of time (so the menu dosen't scroll by way to fast)
  }

  uint8_t fVisibleMenuItems;  //  a variable to store the functionall number of visible menu items (we will set this shortly)
  bool showName;  //  a variable to store if we should display the print name (we will set this shortly)

  if ((mode == 3 || printDone) && printName[0] != 0) {  //  if (we are printing OR if the print is done (and the door hasen't been opened)) AND the first character in the print name isn't NULL
    fVisibleMenuItems = visibleMenuItems - 1;  //  set the visible number of menu items to one less than normal
    showName = true;  //  show the print name

  } else {  //  if we arn't printing and the print is not done
    fVisibleMenuItems = visibleMenuItems;  //  set the visible number of menu items to the normal number
    showName = false;  //  don't show the print name
  }

  uint8_t minTopItem = (menuLength - fVisibleMenuItems);  //  find the lowest down menu item that can be on top
  uint8_t topDisplayMenuItem = min(selectedItem, minTopItem);  //  find the menu item that should be on top (the lowest down one that can be, or the sellected one, wichever is higher)
  uint8_t bottomDisplayMenuItem = topDisplayMenuItem + fVisibleMenuItems;  //  find the bottom visible menu item (the one that should be on the bottom)

  printMenu(topDisplayMenuItem, bottomDisplayMenuItem, showName);  //  display the visible part of the menu, and the print name, if aplicable
  
}  //  checkMenuButtons()
