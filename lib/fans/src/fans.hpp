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

#pragma once

#include <Arduino.h>
#include <Servo.h>
#include <atomic>
#include <algorithm>
#include <pico/mutex.h>
#include <timers.hpp>

namespace fans {
	/**
	* @brief a multicore-safe class for handling fans
	*/
	class Fan {
		public:
			Fan(pin_size_t pin);

			Fan(pin_size_t pin, uint32_t kickstartTime);

			Fan(pin_size_t pin, bool onState);

			Fan(pin_size_t pin, bool onState, uint32_t kickstartTime);

			Fan(pin_size_t pin, bool onState, uint8_t dutyCycle);

			Fan(pin_size_t pin, bool onState, uint8_t dutyCycle, uint32_t kickstartTime);

			/**
			* @brief updates the fan, call every loop
			*/
			virtual void update();

			/**
			* @brief sets the target duty cycle of the fan
			* @note only takes efect after a call to update()
			*/
			void set(uint8_t dutyCycle);

			/**
			* @brief returns the target duty cycle of the fan
			*/
			uint8_t getDutyCycle();

			/**
			* @brief sets if the fan is enabled
			* @note only takes efect after a call to update()
			*/
			void setEnabled(bool enabled);

			/**
			* @brief returns weather or not the fan is enabled
			*/
			bool getEnabled();

			/**
			* @brief enables the fan
			* @note only takes efect after a call to update()
			*/
			void enable();

			/**
			* @brief disables the fan
			* @note only takes efect after a call to update()
			*/
			void disable();

			/**
			* @brief returns weather or not the fan needs the PSU to be on
			*/
			virtual bool needsPower();

			/**
			* @brief sets the maximum speed of the fan
			* @note only takes efect after a call to update()
			*/
			void setMaxSpeed(uint8_t dutyCycle);

			/**
			* @brief returns the maximum speed of the fan
			*/
			uint8_t getMaxSpeed();

			/**
			* @brief sets the minimum speed of the fan
			* @note only takes efect after a call to update()
			*/
			void setMinSpeed(uint8_t dutyCycle);

			/**
			* @brief returns the minimum speed of the fan
			*/
			uint8_t getMinSpeed();

		protected:
			/**
			* @brief sets the duty cycle of the fan
			* @note only takes efect after a call to update()
			*/
			void fan_setDutyCycle(uint8_t dutyCycle);

			/**
			* @brief sets if the fan is enabled
			* @note only takes efect after a call to update()
			*/
			void fan_setEnabled(bool enabled);

			const pin_size_t fan_pin;
			const bool fan_onState;
			const uint8_t fan_disabledDutyCycle;
			const uint8_t fan_enabledDutyCycle;
			uint8_t fan_speed;
			uint8_t fan_oldSpeed;
			uint8_t fan_maxSpeed;
			uint8_t fan_minSpeed;
			bool fan_enabled;
			bool fan_oldEnabled;
			uint32_t fan_kickstartTime;
			timers::Timer fan_kickstartTimer;
			recursive_mutex_t fan_mutex;
	};

	/**
	* @brief a multicore-safe extension of the Fan class, adding control of two servos
	*/
	class TwoServoFan : public Fan {
		public:
			TwoServoFan(pin_size_t fanPin, pin_size_t servo1Pin, pin_size_t servo2Pin, bool onState, uint32_t kickstartTime, uint8_t servoOpenPosition, uint8_t servoClosedPosition, uint8_t servoSpeed);

			/**
			* @brief updates the fan, call every loop
			*/
			void update() override;

			/**
			* @brief returns weather or not the fan needs the PSU to be on
			*/
			bool needsPower() override;

			/**
			* @brief sets the speed of servo 1 (higher is slower)
			*/
			void setServo1Speed(uint8_t speed);

			/**
			* @brief returns the speed of servo 1 (higher is slower)
			*/
			uint8_t getServo1Speed();

			/**
			* @brief sets the "open" position of servo 1
			*/
			void setServo1OpenPosition(uint8_t position);

			/**
			* @brief returns the "open" position of servo 1
			*/
			uint8_t getServo1OpenPosition();

			/**
			* @brief sets the "closed" position of servo 1
			*/
			void setServo1ClosedPosition(uint8_t position);

			/**
			* @brief returns the "closed" position of servo 1
			*/
			uint8_t getServo1ClosedPosition();

			/**
			* @brief sets the speed of servo 1 (higher is slower)
			*/
			void setServo2Speed(uint8_t speed);

			/**
			* @brief returns the speed of servo 1 (higher is slower)
			*/
			uint8_t getServo2Speed();

			/**
			* @brief sets the "open" position of servo 2
			*/
			void setServo2OpenPosition(uint8_t position);

			/**
			* @brief returns the "open" position of servo 2
			*/
			uint8_t getServo2OpenPosition();

			/**
			* @brief sets the "closed" position of servo 2
			*/
			void setServo2ClosedPosition(uint8_t position);

			/**
			* @brief returns the "closed" position of servo 2
			*/
			uint8_t getServo2ClosedPosition();

		protected:
			/**
			* @brief moves servo 1
			* @return false if the servo was already in the specified position, true otherwise
			*/
			bool twoServoFan_moveServo1(uint8_t position);

			/**
			* @brief moves servo 2
			* @return false if the servo was already in the specified position, true otherwise
			*/
			bool twoServoFan_moveServo2(uint8_t position);

			/**
			* @brief opens both servos
			* @return false if the servos were already open, true otherwise
			*/
			bool twoServoFan_openServos();

			/**
			* @brief closes both servos
			* @return false if the servos were already closed, true otherwise
			*/
			bool twoServoFan_closeServos();

			const pin_size_t twoServoFan_servo1Pin;
			const pin_size_t twoServoFan_servo2Pin;
			uint8_t twoServoFan_servo1Position;
			uint8_t twoServoFan_servo1Speed;
			uint8_t twoServoFan_servo1ClosedPosition;
			uint8_t twoServoFan_servo1OpenPosition;
			uint8_t twoServoFan_servo2Position;
			uint8_t twoServoFan_servo2Speed;
			uint8_t twoServoFan_servo2ClosedPosition;
			uint8_t twoServoFan_servo2OpenPosition;
			timers::Timer twoServoFan_powerTimer;
			Servo twoServoFan_servo1;
			Servo twoServoFan_servo2;
	};
}
