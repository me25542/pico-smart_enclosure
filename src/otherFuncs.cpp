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

#include "otherFuncs.hpp"
#include "Adafruit_USBD_CDC.h"
#include "config.hpp"
#include "constants.hpp"
#include "vars.hpp"
#include <cstdint>

void setError(uint8_t origin, uint32_t info, bool recoverable) {
	mode = MODE_ERROR; // mode to error
	errorOrigin = origin; // record the origin of the error
	errorInfo = info; // record info about the error
	is_error_recoverable = recoverable; // record if error is recoverable
}

std::vector<uint8_t> base2ToBase4(uint8_t input) {
	std::vector<uint8_t> base4Digits(4); // std::vector to hold result

	for (uint8_t i = 0; i < 4; i++) { // for each base-4 digit | 4 is used because 4^4 == 256 == 2^8
		base4Digits[i] = input % 4; // find the remainder of the digits | should be optomized to ...base4Digits & 3
		input /= 4; // move to the next digit | should be optomized to base4Digits << 2
	}
	// base4Digits[0] is least significant digit

	return base4Digits;
}

void blinkLED() {
	static timers::Timer blinkTimer;
	static bool ledState = LOW;

	if (blinkTimer.isDone() || !blinkTimer.isSet()) {
		uint32_t duration;

		if (ledState) { // if the LED is on
			if (mode == MODE_STANDBY) { // if the mode is standby
				duration = LED_OFF_TIME_STANDBY; // set the LED to be off for 5 seconds (minus the 50 milliseconds)

			} else {
				duration = LED_OFF_TIME_OTHER; // set the LED to be off for 1 second (minus the 50 milliseconds)
			}

		} else { // if the LED is off
			duration = LED_ON_TIME; // set the LED to be on for 50 milliseconds
		}

		blinkTimer.set(duration); // set the time the LED will stay in the new state
		ledState = !ledState; // toggle LED state
		digitalWrite(LED_PIN, ledState); // set LED to new state
	}
}

void blinkErrorCode(uint8_t code) {
	std::vector<uint8_t> base4Digits = base2ToBase4(code); // convert to base 4 | base4Digits[0] will be the least significant digit

	for (uint8_t i = 4; i > 0; i--) { // for each digit (64s place, 16s place, etc.) | most significant to least significant digits
		for (uint8_t j = 0; j < base4Digits[i - 1]; j++) { // for each set of blinks (indicating the value of the digit)
			for (uint8_t k = 0; k < i; k++) { // for each blink in the set of blinks (indicating the place of the digit)
				digitalWrite(LED_BUILTIN, HIGH); // turn off the built-in LED
				delay(75); // wait for the light to blink
				digitalWrite(LED_BUILTIN, LOW); // turn on the built-in LED
				delay(200); // wait between blinks
			}

			delay(750); // wait between sets of blinks
		}

		delay(1000); // wait between digits
	}
}

bool useI2C(uint16_t timeoutId) {
	uint32_t i;
	for (i = 0; !mutex_try_enter(&I2C_mutex, &I2C_mutex_owner) && i < MAX_I2C_WAIT_TIME; i++) {
		delayMicroseconds(1);
	}

	if (i >= MAX_I2C_WAIT_TIME) {
		uint32_t info = timeoutId << 16;
		setError(13, info, true);
		return false; // timeout

	} else {
		return true; // got the mutex
	}
}

bool useI2C(uint16_t timeoutId, uint16_t timeout_info) {
	uint32_t i;
	for (i = 0; !mutex_try_enter(&I2C_mutex, &I2C_mutex_owner) && i < MAX_I2C_WAIT_TIME; i++) {
		delayMicroseconds(1);
	}

	if (i >= MAX_I2C_WAIT_TIME) {
		uint32_t info = timeoutId << 16;
		info += timeout_info;
		setError(13, info, true);
		return false; // timeout

	} else {
		return true; // got the mutex
	}
}

void doneWithI2C() {
	mutex_exit(&I2C_mutex);
}

bool isI2CDeviceConnected(uint8_t address) {
	#if DEBUG
	uint8_t core = rp2040.cpuid();
	Serial.printf("isI2CDeiceConnected(%u) called from core%u.\n", address, core);  //  print a debug message over the sellected debug port
	#endif

	uint16_t i;
	bool isNotError = false;

	
	useI2C(2);

	for (i = 0; !isNotError && i < 1000; i++) {
		Wire.beginTransmission(address);  //  starts a transmition to the specified device
		uint8_t I2CError = Wire.endTransmission(true);  //  record the response from ending the transmition and releasing the bus
		isNotError = I2CError == 0;
	}

	doneWithI2C();

	#if DEBUG
	Serial.printf("isI2CDeviceConnected returned: %d\n", isNotError);  //  print a debug message over the sellected debug port
	#endif

	return isNotError;  //  this will make the function return true if there were no errors, false otherwise
}

bool areSensorsPresent() {
	#if DEBUG
	uint8_t core = rp2040.cpuid();
	Serial.printf("areSensorsPreasent() called from core%u.\n", core);  //  print a debug message over the sellected debug port
	#endif

	bool sensorNotConnected = false;
	
	{
		uint8_t i;

		for (i = 0; !isI2CDeviceConnected(HEATER_TEMP_SENSOR_ADDRESS) && i < 255; i++) {  //  test up to 255 times if the sensor is conneceted
			delayMicroseconds(10);  //  wait a tiny bit between each test
		}

		if (i >= 255) {
			setError(6, 1, false);
			sensorNotConnected = true;
		}
	}

	{
		uint8_t i;

		for (i = 0; !isI2CDeviceConnected(IN_TEMP_SENSOR_ADDRESS) && i < 255; i++) {  //  test up to 255 times if the sensor is conneceted
			delayMicroseconds(10);  //  wait a tiny bit between each test
		}

		if (i >= 255) {
			setError(6, 2, false);
			sensorNotConnected = true;
		}
	}

	{
		uint8_t i;

		for (i = 0; !isI2CDeviceConnected(OUT_TEMP_SENSOR_ADDRESS) && i < 255; i++) {  //  test up to 255 times if the sensor is conneceted
			delayMicroseconds(10);  //  wait a tiny bit between each test
		}

		if (i >= 255) {
			setError(6, 3, false);
			sensorNotConnected = true;
		}
	}

	bool allSensorsConnected = ! sensorNotConnected;

	#if DEBUG
	Serial.printf("areSensorsPreasent() returned: %d\n", allSensorsConnected);  //  print a debug message over the sellected debug port
	#endif

	return allSensorsConnected;
}

bool isScreenConnected() {
	#if DEBUG
	uint8_t core = rp2040.cpuid();
	Serial.printf("isScreenConnected() called from core%u.\n", core);  //  print a debug message over the sellected debug port
	#endif

	{
		uint8_t i;

		for (i = 0; !isI2CDeviceConnected(SCREEN_ADDRESS) && i < 255; i++) {  //  test up to 255 times if the screen is conneceted
			delayMicroseconds(10);  //  wait a tiny bit between each test
		}

		if (i >= 255) {
			setError(12, SCREEN_ADDRESS, false);
			return false;

		} else {
			return true;
		}
	}
}

bool setPSU(bool state) {
	#if DEBUG
	uint8_t core = rp2040.cpuid();
	Serial.printf("setPSU(%d) called from core%u.\n", state, core);  //  print a debug message over the sellected debug port
	#endif

	mutex_enter_blocking(&PSU_mutex);  //  wait for any other instances of this function (the other core?) to be done

	digitalWrite(PSU_ON_PIN, state);  //  set the power state of the PSU

	if (state) {  //  if we are turning the PSU on
		uint16_t i;
		for (i = 0; ((!digitalRead(POWER_OK_PIN)) && (i < MAX_PSU_ON_TIME)); i++) {  //  while the PSU hasn't reported that the power state is the same as requested
			delay(1);  //  wait for one milisecond
			digitalWrite(PSU_ON_PIN, HIGH); // turn on the PSU
		}

		if (i < MAX_PSU_ON_TIME) {
			PSUIsOn = true;
			mutex_exit(&PSU_mutex); // let any other instance of this function (the other core?) know that we are done
			return true;

		} else {
			setError(9, state, false);
			mutex_exit(&PSU_mutex); // let any other instance of this function (the other core?) know that we are done
			return false;
		}

	} else {  //  if we are turning the PSU off
		PSUIsOn = false;
		mutex_exit(&PSU_mutex); // let any other instance of this function (the other core?) know that we are done
		return true;
	}
}

void updateFan() {
	#if DEBUG
	uint8_t core = rp2040.cpuid();
	Serial.printf("updateFan() called from core%u.\n", core); // print a debug message over USB
	#endif

	static timers::Timer updateTimer;

	fan.setEnabled(!doorOpen.load());
	fan.update();

	if (updateTimer.isSet() && !updateTimer.isDone()) {
		return;
	}

	#if DEBUG
	Serial.printf("seting non-critical fan settings.\n");
	#endif

	updateTimer.set(FAN_UPDATE_INTERVAL);

	fan.setMaxSpeed(maxFanSpeed.load());
	fan.setMinSpeed(MIN_FAN_SPEED); // might change in the future
	fan.setServo1Speed(servo1Speed.load());
	fan.setServo1OpenPosition(servo1Open.load());
	fan.setServo1ClosedPosition(servo1Closed.load());
	fan.setServo2Speed(servo2Speed.load());
	fan.setServo2OpenPosition(servo2Open.load());
	fan.setServo2ClosedPosition(servo2Closed.load());
	fan.update();
}

void setFan(uint8_t dutyCycle) {
	#if DEBUG
	uint8_t core = rp2040.cpuid();
	Serial.printf("setFan(%u) called from core%u.\n", dutyCycle, core); // print a debug message over USB
	#endif

	fan.set(dutyCycle);
}

bool startSerial() {
	Serial.begin(SERIAL_SPEED); // start serial (over USB)
	return Serial; // send back to the calling function if serial is connected
}

uint32_t readTempSensor(Adafruit_MCP9808* sensor, uint8_t sensorID, uint8_t SCALE_MULT, uint8_t SCALE_SHIFT, uint16_t scaledMaxTemp) {
	const uint16_t scaled_minTemp = MIN_TEMP * SCALE_MULT;

	useI2C(4, sensorID);
	float floatTemp = sensor->readTempC(); // get the temperature from the sensor
	doneWithI2C();
	uint32_t scaledTemp = (floatTemp * SCALE_MULT); // convert from floating-point to fixed-point

	if (scaledTemp <= scaled_minTemp || scaledTemp >= scaledMaxTemp) { // if it is not within the acceptable range
		uint32_t info = sensorID << 16; // record the sensor that triggered the error
		info += static_cast<uint8_t>(scaledTemp >> SCALE_SHIFT); // record the temperature reading
		setError(1, info, false); // set mode to error
		return 0;
	}

	return scaledTemp;
}

bool getTemp() {
	constexpr uint8_t SCALE_MULT = 8; // used for multiplication
	constexpr uint8_t SCALE_SHIFT = 3; // used for bitshifting
	constexpr uint8_t SCALE_OFFSET = SCALE_MULT / 2;
	constexpr uint16_t scaled_maxHeaterTemp = MAX_HEATER_TEMP * SCALE_MULT;
	constexpr uint16_t scaled_maxInOutTemp = MAX_IN_OUT_TEMP * SCALE_MULT;
	constexpr uint16_t scaled_minTemp = MIN_TEMP * SCALE_MULT;

	#if DEBUG
	uint8_t core = rp2040.cpuid();
	Serial.printf("getTemp() called from core%u.\n", core); // print a debug message over USB
	#endif
	
	static volatile uint32_t lastReadTime = 120000;

	if (millis() - lastReadTime < (sensorReadInterval * 1000)) { // if it hasn't been long enough sience the last temp reading
		#if DEBUG
		Serial.printf("It hasn't been long enough between temp readings, not reading the temp.\n");
		#endif

		return true;
	}

	lastReadTime = millis(); // update the time

	#if DEBUG
	Serial.printf("It has been long enough between temp readings, reading the temp...\n");
	#endif

	if (!areSensorsPresent()) { // check if any sensors were disconnected (We don't want to waste time if sensors are disconnected)
		return false; // go back. areSensorsPreasant() should have set the mode to error
	}

	// make some temporary variables to store the sum of all mesured temps (for avereging)
	int32_t tempHeaterTemp = 0;
	int32_t tempInTemp = 0;
	int32_t tempOutTemp = 0;

	for (uint8_t i = 0; i < sensorReads; i++) { // do the stuff in this loop the number of times set by sensorReads
		tempHeaterTemp += readTempSensor(&heaterTempSensor, 1, SCALE_MULT, SCALE_SHIFT, scaled_maxHeaterTemp);
		tempInTemp += readTempSensor(&inTempSensor, 2, SCALE_MULT, SCALE_SHIFT, scaled_maxInOutTemp);
		tempOutTemp += readTempSensor(&outTempSensor, 3, SCALE_MULT, SCALE_SHIFT, scaled_maxInOutTemp);
	}

	// logic is: find average reading, add 0.5 (SCALE_OFFSET) because later bitshifting wil truncate things, and then convert back to regular integers from fixed-point by bitshifting
	heaterTemp = ((tempHeaterTemp / sensorReads) + SCALE_OFFSET) >> SCALE_SHIFT;  //  set the temperature to the temporary variable devided by the number of times the temperature was read
	inTemp = ((tempInTemp / sensorReads) + SCALE_OFFSET) >> SCALE_SHIFT;  //  set the temperature to the temporary variable devided by the number of times the temperature was read
	outTemp = ((tempOutTemp / sensorReads) + SCALE_OFFSET) >> SCALE_SHIFT;  //  set the temperature to the temporary variable devided by the number of times the temperature was read

	#if DEBUG
	Serial.printf("Heater temp: %d. | In temp: %d. | Out temp: %d.\n", heaterTemp, inTemp, outTemp);  //  print a debug message over the sellected debug port
	#endif

	return true;
} // bool getTemp()

void lightswitchPressed() {
	#if DEBUG
	uint8_t core = rp2040.cpuid();
	Serial.printf("lightswitchPressed() called from core%u.\n", core);  //  print a debug message over the sellected debug port
	#endif

	turnLightOff = false;

	lightSetState = !lightSetState;  //  change the state of the lights in logic

	mainLight.setState(lightSetState);
}

void manualCooldown() {
	#if DEBUG
	uint8_t core = rp2040.cpuid();
	Serial.printf("manualCooldown() called from core%u.\n", core);  //  print a debug message over the sellected debug port
	#endif

	mode = MODE_COOLDOWN;  //  set the mode to cooldown
}

void doorOpening() {
	#if DEBUG
	uint8_t core = rp2040.cpuid();
	Serial.printf("doorOpening() called from core%u.\n", core);  //  print a debug message over the sellected debug port
	#endif

	doorOpen = true; // remember the door is open
	printDone = false; // the print might have been removed; remember that the print is not done

	if (lights_On_On_Door_Open && !mainLight.getState()) { // if the lights should change on door open
		turnLightOff = true;
		lightSetState = true;
		mainLight.setState(true); // turn on the lights
	}

	if (mode != MODE_PRINTING && printName[0] != 0) { // if the mode is not printing and the print name isn't empty (well, technically if the first character of the print name isn'y NULL)
		#if DEBUG
		Serial.printf("clearing the print name.\n"); // print a debug message over the sellected debug port
		#endif

		for (uint16_t i = 0; i < 256; i++) { // go through all the characters in the print name
			printName[i] = 0; // and set them to NULL
		} // for (uint8_t i = 0; i < 256; i++)
	} // if (mode != MODE_PRINTING)

	heater.disable(); // disable heater
	fan.disable(); // disable the fan
	fan.update();

	#if DEBUG
	Serial.printf("Door open.\n"); // print a debug message over the sellected debug port
	#endif
}

void doorClosing() {
	#if DEBUG
	uint8_t core = rp2040.cpuid();
	Serial.printf("doorClosing() called from core%u.\n", core); // print a debug message over the sellected debug port
	#endif

	if (turnLightOff) { // if the lights should turn off
		lightSetState = false;
		mainLight.setState(false);
	}

	heater.enable(); // enable the heater
	fan.enable(); // enable the fan

	doorOpen = false; // remember that the door is closed
	printDone = false; // the print might have been removed; remember that the print is not done

	#if DEBUG
	Serial.printf("Door closed.\n"); // print a debug message over the sellected debug port
	#endif
}

void checkButtons() {
	#if DEBUG
	uint8_t core = rp2040.cpuid();
	Serial.printf("checkButtons() called from core%u.\n", core); // print a debug message over the sellected debug port
	#endif

	bool input = false; // tracks if user input was detected

	// update the buttons
	door_switch.update();
	light_switch.update();
	coolDown_switch.update();

	if (door_switch.pressed()) {
		doorClosing();
		input = true;

	} else if (door_switch.released()) {
		doorOpening();
		input = true;
	}

	if (light_switch.released()) {
		#if DEBUG
		Serial.printf("Light switch pressed.\n");  //  print a debug message over the sellected debug port
		#endif

		lightswitchPressed();

		input = true;
	}

	static bool cdSwitchWasPressed = false;  //  this variable makes it so that cooldown is triggered only once per button press and hold

	if (coolDown_switch.isPressed()) {  //  if the cooldown switch was just relesed and was previusly held for over a set length of time:
		if ((coolDown_switch.currentDuration() >= COOLDOWN_SWITCH_HOLD_TIME) && !cdSwitchWasPressed) {
			#if DEBUG
			Serial.printf("Cooldown switch pressed.\n");  //  print a debug message over the sellected debug port
			#endif

			cdSwitchWasPressed = true;

			manualCooldown();  //  set mode to cooldown
		}

		input = true;

	} else {
		cdSwitchWasPressed = false;
	}

	if (input) {
		lastUserInput = core1Time;
	}
}

void gpioDisable() {
	// set nearely all pins (all used except for error light and built-in LED) to a disconnected, high-impedance state
	gpio_set_function(FAN_PIN, GPIO_FUNC_NULL); // fan
	gpio_disable_pulls(FAN_PIN); // fan
	gpio_set_function(PSU_ON_PIN, GPIO_FUNC_NULL); // power supply
	gpio_disable_pulls(PSU_ON_PIN); // power supply
	gpio_set_function(HEATER_PIN, GPIO_FUNC_NULL); // heater
	gpio_disable_pulls(HEATER_PIN); // heater
	gpio_set_function(SERVO_2_PIN, GPIO_FUNC_NULL); // servo2
	gpio_disable_pulls(SERVO_2_PIN); // servo2
	gpio_set_function(SERVO_1_PIN, GPIO_FUNC_NULL); // servo1
	gpio_disable_pulls(SERVO_1_PIN); // servo1
	gpio_set_function(I2C1_SCL_PIN, GPIO_FUNC_NULL); // I2C1 SCL
	gpio_disable_pulls(I2C1_SCL_PIN); // I2C1 SCL
	gpio_set_function(I2C1_SDA_PIN, GPIO_FUNC_NULL); // I2C1 SDA
	gpio_disable_pulls(I2C1_SDA_PIN); // I2C1 SDA
	gpio_set_function(I2C0_SCL_PIN, GPIO_FUNC_NULL); // I2C0 SCL
	gpio_disable_pulls(I2C0_SCL_PIN); // I2C0 SCL
	gpio_set_function(I2C0_SDA_PIN, GPIO_FUNC_NULL); // I2C0 SDA
	gpio_disable_pulls(I2C0_SDA_PIN); // I2C0 SDA
	gpio_set_function(LIGHTS_PIN, GPIO_FUNC_NULL); // lights
	gpio_disable_pulls(LIGHTS_PIN); // lights
	gpio_set_function(PRINT_DONE_LIGHT_PIN, GPIO_FUNC_NULL); // print done light
	gpio_disable_pulls(PRINT_DONE_LIGHT_PIN); // print done light
	gpio_set_function(POWER_OK_PIN, GPIO_FUNC_NULL); // power OK
	gpio_disable_pulls(POWER_OK_PIN); // power OK
	gpio_set_function(DOWN_SWITCH_PIN, GPIO_FUNC_NULL); // down button
	gpio_disable_pulls(DOWN_SWITCH_PIN); // down button
	gpio_set_function(UP_SWITCH_PIN, GPIO_FUNC_NULL); // up button
	gpio_disable_pulls(UP_SWITCH_PIN); // up button
	gpio_set_function(SELL_SWITCH_PIN, GPIO_FUNC_NULL); // sellect button
	gpio_disable_pulls(SELL_SWITCH_PIN); // sellect button
	gpio_set_function(COOLDOWN_SWITCH_PIN, GPIO_FUNC_NULL); // cooldown button
	gpio_disable_pulls(COOLDOWN_SWITCH_PIN); // cooldown button
	gpio_set_function(LIGHTSWITCH_PIN, GPIO_FUNC_NULL); // light button
	gpio_disable_pulls(LIGHTSWITCH_PIN); // light button
	gpio_set_function(DOOR_SWITCH_PIN, GPIO_FUNC_NULL); // door switch
	gpio_disable_pulls(DOOR_SWITCH_PIN); // door switch
}

void heatingLogic_temp() {
	// ensure the fan, heaters, and servos have power:
	setPSU(true); // turn on the PSU

	// make it so that the logic can't be messed up by the printer setting the set temp at an unconviniant time
	uint8_t setTemp = globalSetTemp;

	if (oldMode != MODE_PRINTING) { // if printing() wasn't called last loop:
		heatingMode = !(inTemp > setTemp); // set if we are heating or cooling
		hysteresisTriggered = false;
	}

	if (heatingMode) { // if we think that we should be heating the enclosure:
		if ((inTemp + hysteresis) < setTemp) { // if the inside temp is at least <hysteresis> less than it should be:
			heater.turnOn(); // turn on the heater
			setFan(fanOffVal);

		} else if ((inTemp - bigDiff) > setTemp) { // if the inside temp is much higher than it shuld be:
			heatingMode = false;
			heater.turnOff(); // turn off heater
			setFan(fanMidVal);

		} else if (inTemp > setTemp) { // if the inside temp is higher than it should be:
			heater.turnOff(); // turn off heater
			setFan(fanOffVal);
		}

	} else { // if we think we should be cooling the enclosure:
		if (inTemp > setTemp) { // if the inside temp is hotter than itshould be
			heater.turnOff(); // turn off heater

			if (inTemp > (setTemp + hysteresis)) { // if the hysteresis were just triggered (if the inside temp is too hot by enough)
				hysteresisTriggered = true;
			}

			if (hysteresisTriggered) {
				if (inTemp > (setTemp + bigDiff)) { // if the inside temp is too high by a large enough margin that it would brake the maping logic
					setFan(fanOnVal); // turn on the fan

				} else { // if the maping logic won't be broken
					setFan(map(inTemp, setTemp, (setTemp + bigDiff), fanOffVal, fanOnVal)); // set the fan to a value coresponding to how different the temp is from the target
				}

			} else {
				setFan(fanOffVal); // turn off the fan
			}

		} else if ((inTemp + bigDiff) < setTemp) { // if the inside temp is much less than it should be
			heatingMode = true; // we should be heating, not cooling
			hysteresisTriggered = false;
			heater.turnOn(); // turn on heater
			setFan(fanOffVal); // turn off the fan

		} else { // if the inside temp is just a bit less than it should be, or what it should be
			hysteresisTriggered = false;
			heater.turnOff(); // turn off heater
			setFan(fanOffVal);
		}
	}
}

void heatingLogic_manual() {
	// make it so that the logic can't be messed up by the printer setting the set temp at an unconviniant time
	uint8_t setTemp = globalSetTemp;

	bool heaterSetState = setTemp & 0x01; // the least significant bit of setTemp will set the heater state
	uint8_t fanSetVal = setTemp & 0xFE; // the 7 most significant bits of setTemp will set the fan value

	if (heaterSetState || fanSetVal) {
		setPSU(true); // turn on the power suply
	}

	heater.setState(heaterSetState); // set heater
	setFan(fanSetVal); // set the fan
}
