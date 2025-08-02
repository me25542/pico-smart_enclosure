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

#include "menuFuncs.hpp"
#include "config.hpp"
#include "vars.hpp"

using namespace menu;


// clears the print name
void clearName() {
	#if DEBUG
	uint8_t core = rp2040.cpuid();
	Serial.printf("clearName() called from core%u.\n", core); // print a debug message over USB
	#endif

	for (uint8_t i = 0; i != 255; i++) {
		printName[i] = 0;
	}
}

void scrollName(uint8_t height) {
	#if DEBUG
	uint8_t core = rp2040.cpuid();
	Serial.printf("scrollName(%u) called from core%u.\n", height, core); // print a debug message over USB
	#endif

	static uint8_t pos = CHARACTER_WIDTH;
	static uint8_t startChar = 0;
	static timers::Timer updateTimer;

	if (!updateTimer.isDone() && updateTimer.isSet() && dispLastLoop) return; // if enough time hasn't elapsed since the last movement of the cursor, return

	display.setTextSize(1); // normal (1x) text size

	if (!dispLastLoop) { // if the print name wasn't displayed last loop
		// reset the variables to their start values
		pos = CHARACTER_WIDTH;
		startChar = 0;
		updateTimer.set(nameScrollSpeed);

	} else {
		uint32_t overdoneTime = updateTimer.overdone();
		updateTimer.set(nameScrollSpeed - min(static_cast<uint32_t>(nameScrollSpeed.load()), overdoneTime));
	}

	if (pos == 0) { // if the first character is all the way to the left
		pos = CHARACTER_WIDTH - 1; // set the first character to one character's width from the left
		startChar++; // move the first character to be displayed to the next one

		if (printName[startChar] == '\0') { // if the end of the text is reached
			startChar = 0;  // reset back to the start of the text
		}
	} else { // if the first character isn't all the way to the left
		pos--; // move it one pixel to the left
	}

	display.setCursor(pos, height);  // set the cursor

	// draw the visible text
	for (uint8_t i = 0; i < 20; i++) { // repeat for the visible part of the text
		uint8_t charIndex = startChar + i;
		if (printName[charIndex] == '\0') break; // stop if the end of the text is reached

		display.print(printName[charIndex]); // print one character
	}
}

void printHeader() {
	#if DEBUG
	uint8_t core = rp2040.cpuid();
	Serial.printf("printHeader() called from core%u.\n", core); // print a debug message over USB
	#endif

	const uint8_t whereToPrintDeg = (SCREEN_WIDTH - 6);
	const uint8_t whereToPrintTemp = (SCREEN_WIDTH - 66);

	display.setTextSize(2); // 2x normal size text
	display.setTextColor(SSD1306_WHITE, SSD1306_BLACK); // white text on a black background
	display.setCursor(0, 0); // set the cursor in the upper-left hand corner of the display
	display.print(modeStrings[mode]); // print the mode (formated in a lovely string, not a number)

	switch (mode) {
		case MODE_PRINTING: // if the mode is printing
			display.setCursor(whereToPrintTemp, 0); // move the cursor to near the right-hand side of the display (still at the top)
			display.print(static_cast<int8_t>(inTemp)); // print the inside temperature
			display.print("/"); // realy? you can't figure this one out? it prints a "/"
			display.print(globalSetTemp); // print the target temperature (the set temp)
			break;
		
		case MODE_COOLDOWN: // if the mode is cooldown
			display.setCursor(whereToPrintTemp, 0); // move the cursor to near the right-hand side of the display (still at the top)
			display.print(static_cast<int8_t>(inTemp)); // print the inside temperature
			display.print("/");
			display.print(outTemp); // print the target temperature (the outside temperature)
			break;
		
		default: // if the mode is anything else (standby)
			display.setCursor((whereToPrintTemp + 36), 0); // one double-size character is 12 pixels wide, so three are 36 pixels wide
			display.print(static_cast<int8_t>(inTemp));
			break;
	}
	
	display.setTextSize(1); // normal text size (we were at 2x size)
	display.setCursor(whereToPrintDeg, 0);
	display.print("o"); // print a "o"
}

void PrintMenuItem(uint8_t index, uint8_t height, bool topItem) {
	if ((index >= menuLength) || (height >= SCREEN_HEIGHT)) setError(14, 3, false); // invalid parameters passed

	String toPrint = " "; // by default, we print a blank space before the menu item name
	uint16_t textColor = SSD1306_WHITE; // by default, use white text
	uint16_t backgroundColor = SSD1306_BLACK; // by default, use black background

	if (index == selectedItem) { // if this menu item is selected:
		toPrint = MENU_SELLECTED_INDICATOR; // indicate it is sellected

		if (editingMenuItem) { // if we are editing this menu item (if it is clicked on):
			if (topItem) { // if the menu item is at the top of the screen
				display.fillRect(0, height, SCREEN_WIDTH, CHARACTER_HEIGHT, SSD1306_WHITE); // Draw a white rectangle to indicate menu item being sellected

			} else {
				display.fillRect(0, height - 1, SCREEN_WIDTH, CHARACTER_HEIGHT + 1, SSD1306_WHITE); // Draw a white rectangle to indicate menu item being sellected
			}

			textColor = SSD1306_BLACK; // in this case we use black text
			backgroundColor = SSD1306_WHITE; // in this case we use a white background
		}
	}

	toPrint.concat(mainMenu[index]->getName()); // add the menu item's name to the String to be printed
	display.setCursor(0, height); // set the cursor to the left-hand side of the screen, at the correct height
	display.setTextColor(textColor, backgroundColor); // set text and background colors
	display.print(toPrint); // print the complete String
	display.setCursor(SCREEN_WIDTH - (CHARACTER_WIDTH * 4), height); // set the cursor near the right side of the screen (same height)

	if (editingMenuItem && index == selectedItem) { // if we are editing it (if it is clicked on)
		display.print(mainMenu[index]->getTempDataString()); // print temporary data to the screen

	} else { // otherwise (if it is not clicked on)
		display.print(mainMenu[index]->getDataString()); // print the menu item data to the screen
	}
}

bool printMenu(uint8_t startHeight, uint8_t lowerBound, uint8_t uperBound) {
	#if DEBUG
	uint8_t core = rp2040.cpuid();
	Serial.printf("printMenu(%u, %u, %u) called from core%u.\n", startHeight, lowerBound, uperBound, core); // print a debug message over USB
	#endif

	if ((lowerBound >= menuLength) || (uperBound >= menuLength) || (lowerBound > uperBound) || (startHeight >= SCREEN_HEIGHT)) { // if invalid parameters passed
		setError(14, 1, false); // invalid parameters passed to function
		return false; // return with error
	}

	uint8_t vertPos = startHeight; // set the vertical cursor position (used later)

	display.setTextSize(1); // set the text size to normal

	for (uint16_t i = lowerBound; i <= uperBound; i++) { // for each menu item
		PrintMenuItem(i, vertPos, (i == lowerBound)); // print a menu item

		vertPos += LINE_SPACING; // remember to set the cursor lower next iteration of the FOR loop
	} // for (uint16_t i = lowerBound; i <= uperBound; i++)

	return true; // return without errors
} // printMenu()

void printScreensaver() {
	#if DEBUG
	uint8_t core = rp2040.cpuid();
	Serial.printf("printScreenSaver() called from core%u.\n", core); // print a debug message over USB
	#endif

	static uint8_t vert = 0;
	static uint8_t horiz = 0;
	static timers::Timer updateTimer;

	if (updateTimer.isSet() && !updateTimer.isDone() && screensaver) return; // don't update the screen if it hasn't been long enough and it was updated since the screensaver started

	updateTimer.set(SCREENSAVER_SPEED);
	screensaver = true;

	if ((horiz + 1) >= SCREEN_WIDTH) { // if we would go off the right edge of the screen
		horiz = 0; // reset horizontal placement

		if ((vert + 1) >= SCREEN_HEIGHT) { // if we would go off the bottom edge of the screen
			vert = 0; // reset vertical position

		} else {
			vert += 1; // move down one pixel
		}

	} else {
		horiz += 1;
	}

	display.writePixel(static_cast<int16_t>(horiz), static_cast<int16_t>(vert), SSD1306_WHITE); // print a single pixel
}

void updateScreen() {
	#if DEBUG
	uint8_t core = rp2040.cpuid();
	Serial.printf("updateScreen() called from core%u.\n", core); // print a debug message over USB
	#endif

	uint8_t startPos = 0;  //  where to start printing the main menu
	uint8_t fVisibleMenuItems = VISIBLE_MENU_ITEMS;  //  a variable to store the functionall number of visible menu items (we will set this shortly)
	bool showName;  //  a variable to store if we should display the print name (we will set this shortly)

	display.clearDisplay();  //  clear the dispaly's buffer

	if ((mode == MODE_STANDBY) && ((core1Time - lastUserInput) > (screensaverTime * 1000))) {
		printScreensaver();
		goto displayImage;
	}

	screensaver = false;

	if ((mode == MODE_PRINTING || printDone) && (printName[0] != 0)) {  //  if (we are printing OR if the print is done (and the door hasen't been opened)) AND the first character in the print name isn't NULL
		fVisibleMenuItems = VISIBLE_MENU_ITEMS - 1; // set the visible number of menu items to one less than normal
		showName = true; // show the print name

	} else { // if we arn't printing and the print is not done, or if there is no print name to display
		showName = false; // don't show the print name
	}

	if (selectedItem < topDisplayMenuItem) { // if the sellected menu Item isn't visible because it is off the top of the screen
		topDisplayMenuItem = selectedItem; // move the window of menu items being viewed up so that it is visible
		bottomDisplayMenuItem = topDisplayMenuItem + fVisibleMenuItems - 1; // find the bottom visible menu item (the one that should be on the bottom)

	} else if (selectedItem > bottomDisplayMenuItem) { // if the sellected menu Item isn't visible because it is off the bottom of the screen
		bottomDisplayMenuItem = selectedItem; // move the window of menu items being viewed down so that it is visible
		topDisplayMenuItem = bottomDisplayMenuItem - fVisibleMenuItems + 1; // find the top visible menu item (the one that should be at the top)
	}

	printHeader();

	startPos = 16; // the header takes up 16 pixels in height

	if (showName) { // if we are displaying the print name
		scrollName(startPos); // do the scroll name function, it will handel, well, scrolling the print name
		startPos += LINE_SPACING; // move the vertical position

	} else if (dispLastLoop) { // if we are not displaying the print name, and displayed the print name last loop
		clearName(); // clear the print name
	}

	dispLastLoop = showName;

	#if DEBUG
	Serial.printf("Printing menu; startPos = %u, topDisplayMenuItem = %u, bottomDisplayMenuItem = %u.\n", startPos, topDisplayMenuItem, bottomDisplayMenuItem); // print a debug message over USB
	#endif

	printMenu(startPos, topDisplayMenuItem, bottomDisplayMenuItem); // display the visible part of the menu, and the print name, if aplicable

displayImage:
	useI2C(8);

	display.display(); // write everything to the display

	doneWithI2C();
}

// the function called when the "sell." menu button is pressed
void sell_switch_Pressed() {
	#if DEBUG
	uint8_t core = rp2040.cpuid();
	Serial.printf("sell_switch_Pressed() called from core%u.\n", core); // print a debug message over USB
	#endif

	if (editingMenuItem) { // if we are currently editing a item on the menu (before the button was pressed)
		mainMenu[selectedItem]->setData(mainMenu[selectedItem]->getTempData()); // set the menu item data to it's temporary data

		editingMenuItem = false; // we are no longer editing the menu item

	} else { // if we are not editing a menu item (before the button was pressed)
		editingMenuItem = true; // we should now be editing a menu item
		mainMenu[selectedItem]->setTempData(mainMenu[selectedItem]->getData()); // set the temporary menu item data to the value of the menu item data
	} // else (  if (editingMenuItem)  )
} // sell_switch_Pressed()

// the function called when the "up" menu button is pressed
void up_switch_Pressed() {
	#if DEBUG
	uint8_t core = rp2040.cpuid();
	Serial.printf("up_switch_Pressed() called from core%u.\n", core); // print a debug message over USB
	#endif

	if (editingMenuItem) { // if we are editing the data pointed to by an item
		mainMenu[selectedItem]->incrementTempData(); // add one to the temporary data

	} else { // if we are not editing the data pointed to by a menu item (if none are "sellected" or clicked on)
		if (--selectedItem >= menuLength) { // change the one that is sellected (but isn't actually "sellected" or clicked on), and check if it underflowed
			selectedItem = menuLength - 1; // if it did, reset it
		}
	} // else (  if (editingMenuItem)  )
} // up_switch_Pressed()

// the function called when the "down" menu button is pressed
void down_switch_Pressed() {
	#if DEBUG
	uint8_t core = rp2040.cpuid();
	Serial.printf("down_switch_Pressed() called from core%u.\n", core); // print a debug message over USB
	#endif

	if (editingMenuItem) { // if we are editing the data pointed to by an item
		mainMenu[selectedItem]->decrementTempData(); // subtract one form the temporary data

	} else { // if we are not editing the data pointed to by a menu item (if none are "sellected" or clicked on)
		if (++selectedItem >= menuLength) { // change the one that is sellected (but isn't actually "sellected" or clicked on) and check if it is outside of the menu
			selectedItem = 0; // if it is, reset it to 0
		}
	} // else (  if (editingMenuItem)  )
} // down_switch_Pressed

// the function that checks the buttons related to the screen (UI) and calls the apropriet functions. These other functions handle the logic of navigating the menu. The screen is then updated
void checkMenuButtons() {
	#if DEBUG
	uint8_t core = rp2040.cpuid();
	Serial.printf("checkMenuButtons() called from core%u.\n", core); // print a debug message over USB
	#endif

	static timers::Timer holdTimer;

	bool input = false; // tracks if user input was detected

	// update the switches
	sell_switch.update();
	up_switch.update();
	down_switch.update();

	if (sell_switch.released()) { // if the "sell." button was released sience the last update
		if (!screensaver) {
			sell_switch_Pressed(); // call a function (can you guess which one?)
		}
		input = true;
	}

	if (up_switch.released()) { // if the "up" button was released sience the last update
		if (!screensaver) {
			up_switch_Pressed(); // call a function (can you guess which one?)
		}
		input = true;

	} else if (up_switch.isPressed() && (up_switch.currentDuration() >= menuButtonHoldTime) && (!holdTimer.isSet() || holdTimer.isDone())) { // otherwise (if the switch wasn't released sience the last update), if the switch is pressed and has been for a set length of time
		if (!screensaver) {
			up_switch_Pressed(); // call a function (can you guess which one?)
		}

		holdTimer.set(menuScrollSpeed); // wait for a set length of time (so the menu dosen't scroll by way to fast)
		
		input = true;

	}

	if (down_switch.released()) { // if the "down" button was released sience the last update
		if (!screensaver) {
			down_switch_Pressed(); // call a function (can you guess which one?)
		}
		
		input = true;

	} else if (down_switch.isPressed() && (down_switch.currentDuration() >= menuButtonHoldTime) && (!holdTimer.isSet() || holdTimer.isDone())) {
		if (!screensaver) {
			down_switch_Pressed(); // call a function (can you guess which one?)
		}

		holdTimer.set(menuScrollSpeed); // wait for a set length of time (so the menu dosen't scroll by way to fast)
		
		input = true;
	}

	if (input) {
		lastUserInput = core1Time;
	}
} // checkMenuButtons()

void updateMenuBackup() {
	uint32_t index = 0;
	size_t maxAddress = menuDataBackup.size();

	for (uint8_t i = 0; i < menuLength; i++) { // for each menu item
		size_t numBytes = mainMenu[i]->getDataSize(); // store the size in bytes of the menu item's data
		uint32_t menuData = mainMenu[i]->getData();

		for (size_t j = 0; j < numBytes; j++) { // for each byte
			menuDataBackup[min(index, maxAddress)] = static_cast<uint8_t>(menuData & 0xFF); // store the 8 least significan bits (least significant byte) of the data in menuDataBackup
			menuData >>= 8; // shift the menu data 1 byte (8 bits) to the right
			index += 1; // add one to the index
		} // for (size_t j = 0; j < numBytes; j++)
	} // for (uint8_t i = 0; i < menuLength; i++)
}

bool periodicMenuBackup() {
	static timers::Timer menuBackupTimer;

	if (menuBackupTimer.isSet() && !menuBackupTimer.isDone()) { // if waiting to backup
		return false; // backup not updated
	}

	menuBackupTimer.set((backupInterval * 1000) - static_cast<uint16_t>(menuBackupTimer.overdone())); // wait to backup

	updateMenuBackup();

	return true; // backup updated
}
