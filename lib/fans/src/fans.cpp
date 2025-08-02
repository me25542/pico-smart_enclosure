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

#include "fans.hpp"
#include "pico/mutex.h"
#include <cstdint>

//**Fan****************************************************************************************************************************************************************************

fans::Fan::Fan(pin_size_t pin)
	: fan_pin(pin), fan_onState(HIGH), fan_disabledDutyCycle((!fan_onState) * 255), fan_enabledDutyCycle(fan_onState * 255) {
	fan_speed = fan_disabledDutyCycle;
	fan_oldSpeed = fan_disabledDutyCycle;
	fan_maxSpeed = UINT8_MAX;
	fan_minSpeed = 0;
	fan_enabled = true;
	fan_oldEnabled = false;
	fan_kickstartTime = 1000;
	recursive_mutex_init(&fan_mutex);
	analogWrite(fan_pin, fan_speed);
}

fans::Fan::Fan(pin_size_t pin, uint32_t kickstartTime)
	: fan_pin(pin), fan_onState(HIGH), fan_disabledDutyCycle((!fan_onState) * 255), fan_enabledDutyCycle(fan_onState * 255) {
	fan_speed = fan_disabledDutyCycle;
	fan_oldSpeed = fan_disabledDutyCycle;
	fan_maxSpeed = UINT8_MAX;
	fan_minSpeed = 0;
	fan_enabled = true;
	fan_oldEnabled = false;
	fan_kickstartTime = kickstartTime;
	recursive_mutex_init(&fan_mutex);
	analogWrite(fan_pin, fan_speed);
}

fans::Fan::Fan(pin_size_t pin, bool onState)
	: fan_pin(pin), fan_onState(onState), fan_disabledDutyCycle((!fan_onState) * 255), fan_enabledDutyCycle(fan_onState * 255) {
	fan_speed = fan_disabledDutyCycle;
	fan_oldSpeed = fan_disabledDutyCycle;
	fan_maxSpeed = UINT8_MAX;
	fan_minSpeed = 0;
	fan_enabled = true;
	fan_oldEnabled = false;
	fan_kickstartTime = 1000;
	recursive_mutex_init(&fan_mutex);
	analogWrite(fan_pin, fan_speed);
}

fans::Fan::Fan(pin_size_t pin, bool onState, uint32_t kickstartTime)
	: fan_pin(pin), fan_onState(onState), fan_disabledDutyCycle((!fan_onState) * 255), fan_enabledDutyCycle(fan_onState * 255) {
	fan_speed = fan_disabledDutyCycle;
	fan_oldSpeed = fan_disabledDutyCycle;
	fan_maxSpeed = UINT8_MAX;
	fan_minSpeed = 0;
	fan_enabled = true;
	fan_oldEnabled = false;
	fan_kickstartTime = kickstartTime;
	recursive_mutex_init(&fan_mutex);
	analogWrite(fan_pin, fan_speed);
}

fans::Fan::Fan(pin_size_t pin, bool onState, uint8_t dutyCycle)
	: fan_pin(pin), fan_onState(onState), fan_disabledDutyCycle((!fan_onState) * 255), fan_enabledDutyCycle(fan_onState * 255) {
	fan_speed = dutyCycle;
	fan_oldSpeed = fan_disabledDutyCycle;
	fan_maxSpeed = UINT8_MAX;
	fan_minSpeed = 0;
	fan_enabled = true;
	fan_oldEnabled = false;
	fan_kickstartTime = 1000;
	recursive_mutex_init(&fan_mutex);
	analogWrite(fan_pin, fan_speed);
}

fans::Fan::Fan(pin_size_t pin, bool onState, uint8_t dutyCycle, uint32_t kickstartTime)
	: fan_pin(pin), fan_onState(onState), fan_disabledDutyCycle((!fan_onState) * 255), fan_enabledDutyCycle(fan_onState * 255) {
	fan_speed = dutyCycle;
	fan_oldSpeed = fan_disabledDutyCycle;
	fan_maxSpeed = UINT8_MAX;
	fan_minSpeed = 0;
	fan_enabled = true;
	fan_oldEnabled = false;
	fan_kickstartTime = kickstartTime;
	recursive_mutex_init(&fan_mutex);
	analogWrite(fan_pin, fan_speed);
}

void fans::Fan::update() {
	recursive_mutex_enter_blocking(&fan_mutex);

	if (fan_enabled && !fan_kickstartTimer.isSet() && ((!fan_oldEnabled || (fan_oldSpeed == fan_disabledDutyCycle)) && (fan_speed != fan_disabledDutyCycle))) {
		fan_kickstartTimer.set(fan_kickstartTime);
		analogWrite(fan_pin, fan_enabledDutyCycle);

	} else if (fan_enabled && fan_kickstartTimer.isSet() && !fan_kickstartTimer.isDone() && (fan_speed != fan_disabledDutyCycle)) {
		analogWrite(fan_pin, fan_enabledDutyCycle);

	} else if (!fan_enabled) {
		if (fan_kickstartTimer.isSet()) fan_kickstartTimer.stop();

		analogWrite(fan_pin, fan_disabledDutyCycle);

	} else {
		analogWrite(fan_pin, fan_speed);
	}

	fan_oldSpeed = fan_speed;
	fan_oldEnabled = fan_enabled;

	recursive_mutex_exit(&fan_mutex);
}

void fans::Fan::set(uint8_t dutyCycle) {
	recursive_mutex_enter_blocking(&fan_mutex);
	fan_setDutyCycle(dutyCycle);
	recursive_mutex_exit(&fan_mutex);
}

uint8_t fans::Fan::getDutyCycle() {
	recursive_mutex_enter_blocking(&fan_mutex);
	uint8_t returnValue = fan_speed;
	recursive_mutex_exit(&fan_mutex);
	return returnValue;
}

void fans::Fan::setEnabled(bool enabled) {
	recursive_mutex_enter_blocking(&fan_mutex);
	fan_setEnabled(enabled);
	recursive_mutex_exit(&fan_mutex);
}

bool fans::Fan::getEnabled() {
	recursive_mutex_enter_blocking(&fan_mutex);
	bool returnValue = fan_enabled;
	recursive_mutex_exit(&fan_mutex);
	return returnValue;
}

void fans::Fan::disable() {
	setEnabled(false);
}

void fans::Fan::enable() {
	setEnabled(true);
}

bool fans::Fan::needsPower() {
	recursive_mutex_enter_blocking(&fan_mutex);
	bool returnValue = fan_enabled && (fan_speed != fan_disabledDutyCycle);
	recursive_mutex_exit(&fan_mutex);
	return returnValue;
}

void fans::Fan::setMaxSpeed(uint8_t dutyCycle) {
	recursive_mutex_enter_blocking(&fan_mutex);
	fan_maxSpeed = dutyCycle;
	fan_setDutyCycle(fan_onState ? fan_speed : ~fan_speed);
	recursive_mutex_exit(&fan_mutex);
}

uint8_t fans::Fan::getMaxSpeed() {
	recursive_mutex_enter_blocking(&fan_mutex);
	uint8_t returnValue = fan_maxSpeed;
	recursive_mutex_exit(&fan_mutex);
	return returnValue;
}

void fans::Fan::setMinSpeed(uint8_t dutyCycle) {
	recursive_mutex_enter_blocking(&fan_mutex);
	fan_minSpeed = dutyCycle;
	recursive_mutex_exit(&fan_mutex);
}

uint8_t fans::Fan::getMinSpeed() {
	recursive_mutex_enter_blocking(&fan_mutex);
	uint8_t returnValue = fan_minSpeed;
	recursive_mutex_exit(&fan_mutex);
	return returnValue;
}

void fans::Fan::fan_setDutyCycle(uint8_t dutyCycle) {
	uint8_t clampedDutyCycle = (dutyCycle == 0) ? 0 : std::clamp(dutyCycle, fan_minSpeed, fan_maxSpeed); // clamp the duty cycle between min and max, or set it to zero
	fan_speed = fan_onState ? clampedDutyCycle : ~clampedDutyCycle; // store the clamped value, or its inverse
}

void fans::Fan::fan_setEnabled(bool enabled) {
	fan_enabled = enabled;
}

//**TwoServoFan********************************************************************************************************************************************************************

fans::TwoServoFan::TwoServoFan(pin_size_t fanPin, pin_size_t servo1Pin, pin_size_t servo2Pin, bool onState, uint32_t kickstartTime, uint8_t servoOpenPosition, uint8_t servoClosedPosition, uint8_t servoSpeed)
	: Fan(fanPin, onState, kickstartTime), twoServoFan_servo1Pin(servo1Pin), twoServoFan_servo2Pin(servo2Pin) {
	twoServoFan_servo1OpenPosition = servoOpenPosition;
	twoServoFan_servo2OpenPosition = servoOpenPosition;
	twoServoFan_servo1ClosedPosition = servoClosedPosition;
	twoServoFan_servo2ClosedPosition = servoClosedPosition;
	twoServoFan_servo1Speed = servoSpeed;
	twoServoFan_servo2Speed = servoSpeed;

	twoServoFan_servo1.attach(twoServoFan_servo1Pin);
	twoServoFan_servo2.attach(twoServoFan_servo2Pin);
}

void fans::TwoServoFan::update() {
	recursive_mutex_enter_blocking(&fan_mutex);

	if (fan_enabled && !fan_kickstartTimer.isSet() && ((!fan_oldEnabled || (fan_oldSpeed == fan_disabledDutyCycle)) && (fan_speed != fan_disabledDutyCycle))) {
		fan_kickstartTimer.set(fan_kickstartTime);
		analogWrite(fan_pin, fan_enabledDutyCycle);

	} else if (fan_enabled && fan_kickstartTimer.isSet() && !fan_kickstartTimer.isDone() && (fan_speed != fan_disabledDutyCycle)) {
		analogWrite(fan_pin, fan_enabledDutyCycle);

	} else if (!fan_enabled) {
		if (fan_kickstartTimer.isSet()) fan_kickstartTimer.stop();

		analogWrite(fan_pin, fan_disabledDutyCycle);

	} else {
		analogWrite(fan_pin, fan_speed);
	}

	if (fan_enabled && (fan_speed != fan_disabledDutyCycle)) {
		twoServoFan_openServos();

	} else {
		twoServoFan_closeServos();
	}

	fan_oldSpeed = fan_speed;
	fan_oldEnabled = fan_enabled;

	recursive_mutex_exit(&fan_mutex);
}

bool fans::TwoServoFan::needsPower() {
	recursive_mutex_enter_blocking(&fan_mutex);
	bool returnValue = (fan_enabled && (fan_speed != fan_disabledDutyCycle)) || (twoServoFan_powerTimer.isSet() && !twoServoFan_powerTimer.isDone());
	recursive_mutex_exit(&fan_mutex);
	return returnValue;
}

void fans::TwoServoFan::setServo1Speed(uint8_t speed) {
	recursive_mutex_enter_blocking(&fan_mutex);
	twoServoFan_servo1Speed = speed;
	recursive_mutex_exit(&fan_mutex);
}

uint8_t fans::TwoServoFan::getServo1Speed() {
	recursive_mutex_enter_blocking(&fan_mutex);
	uint8_t returnValue = twoServoFan_servo1Speed;
	recursive_mutex_exit(&fan_mutex);
	return returnValue;
}

void fans::TwoServoFan::setServo1OpenPosition(uint8_t position) {
	recursive_mutex_enter_blocking(&fan_mutex);
	twoServoFan_servo1OpenPosition = position;
	recursive_mutex_exit(&fan_mutex);
}

uint8_t fans::TwoServoFan::getServo1OpenPosition() {
	recursive_mutex_enter_blocking(&fan_mutex);
	uint8_t returnValue = twoServoFan_servo1OpenPosition;
	recursive_mutex_exit(&fan_mutex);
	return returnValue;
}

void fans::TwoServoFan::setServo1ClosedPosition(uint8_t position) {
	recursive_mutex_enter_blocking(&fan_mutex);
	twoServoFan_servo1ClosedPosition = position;
	recursive_mutex_exit(&fan_mutex);
}

uint8_t fans::TwoServoFan::getServo1ClosedPosition() {
	recursive_mutex_enter_blocking(&fan_mutex);
	uint8_t returnValue = twoServoFan_servo1ClosedPosition;
	recursive_mutex_exit(&fan_mutex);
	return returnValue;
}

void fans::TwoServoFan::setServo2Speed(uint8_t speed) {
	recursive_mutex_enter_blocking(&fan_mutex);
	twoServoFan_servo2Speed = speed;
	recursive_mutex_exit(&fan_mutex);
}

uint8_t fans::TwoServoFan::getServo2Speed() {
	recursive_mutex_enter_blocking(&fan_mutex);
	uint8_t returnValue = twoServoFan_servo2Speed;
	recursive_mutex_exit(&fan_mutex);
	return returnValue;
}

void fans::TwoServoFan::setServo2OpenPosition(uint8_t position) {
	recursive_mutex_enter_blocking(&fan_mutex);
	twoServoFan_servo2OpenPosition = position;
	recursive_mutex_exit(&fan_mutex);
}

uint8_t fans::TwoServoFan::getServo2OpenPosition() {
	recursive_mutex_enter_blocking(&fan_mutex);
	uint8_t returnValue = twoServoFan_servo2OpenPosition;
	recursive_mutex_exit(&fan_mutex);
	return returnValue;
}

void fans::TwoServoFan::setServo2ClosedPosition(uint8_t position) {
	recursive_mutex_enter_blocking(&fan_mutex);
	twoServoFan_servo2ClosedPosition = position;
	recursive_mutex_exit(&fan_mutex);
}

uint8_t fans::TwoServoFan::getServo2ClosedPosition() {
	recursive_mutex_enter_blocking(&fan_mutex);
	uint8_t returnValue = twoServoFan_servo2ClosedPosition;
	recursive_mutex_exit(&fan_mutex);
	return returnValue;
}

bool fans::TwoServoFan::twoServoFan_moveServo1(uint8_t position) {
	if (twoServoFan_servo1Position == position) {
		return false;
	}

	for (; twoServoFan_servo1Position != position; (position > twoServoFan_servo1Position) ? twoServoFan_servo1Position++ : twoServoFan_servo1Position--) {
		twoServoFan_servo1.write(twoServoFan_servo1Position);
		delay(twoServoFan_servo1Speed);
	}

	twoServoFan_powerTimer.set(1000);

	return true;
}

bool fans::TwoServoFan::twoServoFan_moveServo2(uint8_t position) {
	if (twoServoFan_servo2Position == position) {
		return false;
	}

	for (; twoServoFan_servo2Position != position; (position > twoServoFan_servo2Position) ? twoServoFan_servo2Position++ : twoServoFan_servo2Position--) {
		twoServoFan_servo2.write(twoServoFan_servo2Position);
		delay(twoServoFan_servo2Speed);
	}

	twoServoFan_powerTimer.set(1000);

	return true;
}

bool fans::TwoServoFan::twoServoFan_openServos() {
	bool servo1Moved = twoServoFan_moveServo1(twoServoFan_servo1OpenPosition);
	bool servo2Moved = twoServoFan_moveServo2(twoServoFan_servo2OpenPosition);
	return servo1Moved || servo2Moved;
}

bool fans::TwoServoFan::twoServoFan_closeServos() {
	bool servo1Moved = twoServoFan_moveServo1(twoServoFan_servo1ClosedPosition);
	bool servo2Moved = twoServoFan_moveServo2(twoServoFan_servo2ClosedPosition);
	return servo1Moved || servo2Moved;
}
