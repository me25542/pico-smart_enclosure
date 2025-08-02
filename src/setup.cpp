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

#include "setup.hpp"
#include "config.hpp"
#include "otherFuncs.hpp"
#include "vars.hpp"


void varInit() {
	// initialize mutexes
	mutex_init(&I2C_mutex);
	mutex_init(&PSU_mutex);

	// setup the backup data for power loss
	menuBackupSetup();
}

bool tempSensorSetup() {
	#if DEBUG
	Serial.printf("tempSensorSetup() called.\n"); // print a debug message over USB
	#endif

	useI2C(1);

	Wire.setSDA(I2C0_SDA_PIN); // set the SDA of I2C0
	Wire.setSCL(I2C0_SCL_PIN); // set the SCL of I2C0
	Wire.setClock(400000); // set the bus to fast mode
	Wire.begin(); // Initialize the I2C0 bus as the master

	delay(1); // wait for a tiny bit

	// start I2C temp sensors and set mode to error if it fails:
	if (! heaterTempSensor.begin(HEATER_TEMP_SENSOR_ADDRESS)) { // if starting the heater temp sensor failed
		setError(3, 1, true); // set mode to error

		#if DEBUG
		Serial.printf("Starting the heater sensor failed.\n"); // print a debug message over USB
		#endif

		doneWithI2C();

		return false;
	}

	if (! inTempSensor.begin(IN_TEMP_SENSOR_ADDRESS)) { // if starting the in temp sensor failed
		setError(3, 2, true); // set mode to error

		#if DEBUG
		Serial.printf("Starting the in temp sensor failed.\n"); // print a debug message over USB
		#endif

		doneWithI2C();

		return false;
	}

	if (! outTempSensor.begin(OUT_TEMP_SENSOR_ADDRESS)) { // if starting the out temp sensor failed
		setError(3, 3, true); // set mode to error

		#if DEBUG
		Serial.printf("Starting the out temp sensor failed.\n"); // print a debug message over USB
		#endif

		doneWithI2C();

		return false;
	}

	// set I2C temp sensor resolution:
	heaterTempSensor.setResolution(i2cTempSensorRes);
	inTempSensor.setResolution(i2cTempSensorRes);
	outTempSensor.setResolution(i2cTempSensorRes);

	doneWithI2C();

	// check if the sensors are connected ant try to get the temperature from them
	if (areSensorsPresent() && getTemp()) {  //  if the sensors are preasant AND geting the temperature was sucessfull | these will both handel I2C availability themselves

		#if DEBUG
		Serial.printf("Sensor startup sucessful.\n"); // print a debug message over USB
		#endif

		return true;

	} else {
		#if DEBUG
		Serial.printf("Sensor startup failed.\n"); // print a debug message over USB
		#endif

		return false;
	}
}

void printerI2cSetup() {
	#if DEBUG
	Serial.printf("printerI2cSetup()called.\n"); // print a debug message over USB
	#endif

	// start I2C1 (the priter's bus):
	Wire1.setSDA(I2C1_SDA_PIN); // set the SDA of I2C1
	Wire1.setSCL(I2C1_SCL_PIN); // set the SCL of I2C1
	Wire1.begin(ENCLOSURE_ADDRESS); // Initialize the I2C1 slave address of the enclosure
	Wire1.onRequest(requestEvent); // set the function to call when the printer requests data via I2C
	Wire1.onReceive(I2cReceived); // set the function to call when the printer sends data via I2C

	#if DEBUG
	Serial.printf("Exiting printerI2cSetup().\n"); // print a debug message over USB
	#endif
}

void buttonSetup() {
	#if DEBUG
	Serial.printf("buttonSetup() called.\n"); // print a debug message over USB
	#endif

	// set switch pinModes
	door_switch.attach(DOOR_SWITCH_PIN, INPUT_PULLUP);
	light_switch.attach(LIGHTSWITCH_PIN, INPUT_PULLUP);
	coolDown_switch.attach(COOLDOWN_SWITCH_PIN, INPUT_PULLUP);
	sell_switch.attach(SELL_SWITCH_PIN, INPUT_PULLUP);
	up_switch.attach(UP_SWITCH_PIN, INPUT_PULLUP);
	down_switch.attach(DOWN_SWITCH_PIN, INPUT_PULLUP);

	#if !USE_DEFAULT_DEBOUNCE_TIME
	// set switch debounce intervals
	door_switch.interval(DOOR_SWITCH_DEBOUNCE_TIME);
	light_switch.interval(LIGHT_SWITCH_DEBOUNCE_TIME);
	coolDown_switch.interval(COOLDOWN_SWITCH_DEBOUNCE_TIME);
	sell_switch.interval(SELL_SWITCH_DEBOUNCE_TIME);
	up_switch.interval(UP_SWITCH_DEBOUNCE_TIME);
	down_switch.interval(DOWN_SWITCH_DEBOUNCE_TIME);
	#endif

	#if !MANUAL_PRESSED_STATE
	if (findPressedState) {
		doorSw_ps = !digitalRead(DOOR_SWITCH_PIN);
		lightSw_ps = !digitalRead(LIGHTSWITCH_PIN);
		coolDownSw_ps = !digitalRead(COOLDOWN_SWITCH_PIN);
		sellSw_ps = !digitalRead(SELL_SWITCH_PIN);
		upSw_ps = !digitalRead(UP_SWITCH_PIN);
		downSw_ps = !digitalRead(DOWN_SWITCH_PIN);
	}
	#endif

	// set switch pressed states
	door_switch.setPressedState(doorSw_ps);
	light_switch.setPressedState(lightSw_ps);
	coolDown_switch.setPressedState(coolDownSw_ps);
	sell_switch.setPressedState(sellSw_ps);
	up_switch.setPressedState(upSw_ps);
	down_switch.setPressedState(downSw_ps);

	for (uint32_t i = 0; i < SWITCH_INITIALIZATION_UPDATES; i++) {
		delay(SWITCH_INITIALIZATION_UPDATE_INTERVAL);
		// update switches
		door_switch.update();
		light_switch.update();
		coolDown_switch.update();
		sell_switch.update();
		up_switch.update();
		down_switch.update();
	}

	door_switch.pressed();
	light_switch.pressed();
	coolDown_switch.pressed();
	sell_switch.pressed();
	up_switch.pressed();
	down_switch.pressed();

	doorOpen = !door_switch.isPressed(); // for startup after power loss, just going by state changes dosn't work

	#if DEBUG
	Serial.printf("Exiting buttonSetup().\n"); // print a debug message over USB
	#endif
}

void pinSetup() {
	#if DEBUG
	Serial.printf("pinSetup() called.\n"); // print a debug message over USB
	#endif
	
	// Pin definitions:
	// outputs, high pin# - low pin#
	pinMode(LED_PIN, OUTPUT);
	pinMode(ERROR_LIGHT_PIN, OUTPUT);
	pinMode(PSU_ON_PIN, OUTPUT);

	// inputs, high pin# - low pin#
	pinMode(POWER_OK_PIN, INPUT_PULLDOWN);

	// set initial output pin values, high pin# - low pin#:
	digitalWrite(LED_PIN, HIGH); // turn on the built-in LED
	digitalWrite(ERROR_LIGHT_PIN, !ERROR_LIGHT_ON); // turn off the error light
	digitalWrite(PSU_ON_PIN, LOW); // turn off the power supply

	#if DEBUG
	Serial.printf("Exiting pinSetup().\n"); // print a debug message over USB
	#endif
}

bool menuSetup() {
	#if DEBUG
	Serial.printf("menuSetup() called.\n"); // print a debug message over USB
	#endif

	useI2C(9);

	uint16_t i;
	for (i = 0; (i < MAX_SCREEN_STARTUP_TRIES) && !display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS); i++) { delay(1); }

	if (i >= MAX_SCREEN_STARTUP_TRIES) {
		mode = 0;
		errorOrigin = 10;

		#if DEBUG
		Serial.printf("Starting screen failed.\n"); // print a debug message over the sellected debug port
		#endif

		doneWithI2C();

		#if DEBUG
		Serial.printf("menuSetup() returned false\n"); // print a debug message over Serial
		#endif
	
		return false;

	} else {
		display.display();

		display.clearDisplay();
		display.cp437(true);  //  use normal ASCII text encoding
		display.setCursor(0, 0);
		display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
		display.setTextSize(2);
		display.print("Loading...");

		display.display();

		doneWithI2C();

		#if DEBUG
		Serial.printf("menuSetup() returned true\n"); // print a debug message over Serial
		#endif

		return true;
	}
}

void menuBackupSetup() {
	for (uint8_t i = 0; i < menuLength; i++) { // for each menu item
		size_t numBytes = mainMenu[i]->getDataSize(); // store the size in bytes of the menu item's data
		uint32_t mask = 0xFF; // a variable to act as a bitmask

		for (size_t j = 0; j < numBytes; j++) { // for each byte
			menuDataBackup.push_back(static_cast<uint8_t>(mainMenu[i]->getData() & mask)); // store the 8 least significan bits (least significant byte) of the data in menuDataBackup
			mask <<= 8; // shift the mask 1 byte (8 bits) to the left
		} // for (size_t j = 0; j < numBytes; j++)
	} // for (uint8_t i = 0; i < menuLength; i++)

	menuDataBackup.shrink_to_fit(); // remove unused memmory allocated, it will never be used
}

bool checkVersion() {
	//  get the stored version info (the version info of the code that stored the data)
	uint8_t stored_majorVersion = EEPROM.read(1);
	uint8_t stored_minorVersion = EEPROM.read(2);
	uint8_t stored_bugFixVersion = EEPROM.read(3);
	uint8_t stored_buildVersion = EEPROM.read(4);

	return ((stored_majorVersion == majorVersion) && (stored_minorVersion == minorVersion) && (stored_bugFixVersion == bugFixVersion) && (stored_buildVersion == buildVersion));
		//  a bit long here, but basically return true if the version is the same as the stored one, false otherwise
}

#if !manualPressedState
bool buttonRecovery() {
	bool useButtonData = !static_cast<bool>(EEPROM.read(9));  //  use the button data only if address 9 contains a 0

	if (useButtonData) {
		findPressedState = false;

		doorSw_ps = static_cast<bool>(EEPROM.read(10));
		lightSw_ps = static_cast<bool>(EEPROM.read(11));
		coolDownSw_ps = static_cast<bool>(EEPROM.read(12));
		sellSw_ps = static_cast<bool>(EEPROM.read(13));
		upSw_ps = static_cast<bool>(EEPROM.read(14));
		downSw_ps = static_cast<bool>(EEPROM.read(15));

	} else {
		findPressedState = true;
	}

	return useButtonData;
}
#endif

inline bool checkError() {
	return EEPROM.read(5) == 255; // return true only if the stored value is 255
}

bool menuRecovery() {
	bool useMenuData = static_cast<bool>(EEPROM.read(0)); // recover data about weather or not to use menu item data

	if (!useMenuData) {
		return false; // data not recovered
	}

	uint16_t memAdr = 1023;  //  start with the adress at the start of the second kb of memory

	for (uint8_t i = 0; i < menuLength; i++) { // for each menu item
		size_t numBytes = mainMenu[i]->getDataSize(); // store the size in bytes of the menu item's data
		uint32_t recoveredData = 0; // temporary data stored in each menu item *could* be used instead of this, but memmory isn't at enough of a premium

		for (size_t j = 0; j < numBytes; j++) { // for each byte of data for the menu item
			recoveredData |= static_cast<uint32_t>(EEPROM.read(memAdr++)) << (8 * j);
		}

		mainMenu[i]->setData(recoveredData);
	}

	return true; // data recovered
}  //  menuRecovery()

void backupRecovery() {
	#if DEBUG
	Serial.printf("backupRecovery() called.\n"); // print a debug message over USB
	#endif

	EEPROM.begin(2048);  //  2048 because we have 1kb (1024b) for data, and I wanted a nice number of bytes to be used (we use like 10 of the remaining 1024 for version info and if to use the data)

	if (checkVersion()) {  //  if the version is the same as the stored one. We do this so that if data compatability is broken by some future update we don't break things
		#if !manualPressedState
		buttonRecovery();
		#endif

		if (checkError()) { // if the mode wasn't error on backup
			menuRecovery(); // recover the stored menu data
		}
	}  //  if (checkVersion)
}  //  void backupRecovery()

void serialSetup() {
	for (uint16_t serialStartupTries = 0; serialStartupTries < MAX_SERIAL_STARTUP_TRIES && !startSerial(); ++ serialStartupTries); // start serial (USB) comunication (and wait for up to one second for a computer to be connected)
	Serial.setTimeout(SERIAL_TIMOUT); // set the serial timout time

	#if DEBUG
	Serial.printf("Exiting serialSetup().\n"); // print a debug message over USB
	#endif
}

