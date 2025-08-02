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
#include <atomic>

namespace lights {
	/**
	* @brief class to smoothly controll a light's state without blocking other code
	*/
	class Light {
		public:
			/**
			* @brief initializes the light with it's inital state and the speed at which to change it
			* @param pin the pin the light is connected to
			* @param speed the speed the light will change at (lower is faster)
			* @param state the initial state of the light
			* @param PSUVar a pointer to a variable containing the state of the PSU
			*/
			Light(uint8_t pin, uint8_t speed, bool onState, bool state, std::atomic<bool> *PSUVar);

			/**
			* @brief returns the pin number ascosiated with the light
			*/
			uint8_t getPin();

			/**
			* @brief returns true if the light is changing OR on, false otherwise (if it is off AND not changing)
			*/
			bool needsPower();

			/**
			* @brief sets the speed to change the light at (lower is faster)
			*/
			void setSpeed(uint8_t speed);

			/**
			* @brief returns the diming speed of the light, in ms per 1/255 full brightness
			*/
			uint8_t getSpeed();

			/**
			* @brief sets the state of the light
			*/
			void setState(bool state);

			/**
			* @brief changes the state of the light
			*/
			void changeState();

			/**
			* @brief returns the state of the light; true for on, false for off
			*/
			bool getState();

			/**
			* @brief call as often as possible, handles all actual setting of the light (diming, etc.)
			*/
			void tick();

			/**
			* @brief breifly changes the state of the light, then revets to what it was again. Used to fix a hardware issue | EXPERIMENTAL
			* @param microseconds the amount of time that the light's state will be inverted for
			*/
			void blip(uint32_t microseconds);
		
		private:
			const uint8_t light_pin; ///< the pin number the light is attached to
			volatile uint8_t light_speed; ///< the speed at which the light will dim / brighten
			const bool light_onState; ///< the HIGH / LOW state that the light pin is at when ON
			volatile bool light_state; ///< the target state of the light
			volatile bool light_changing; ///< true if the light is changing to a new state
			volatile bool light_toChange; ///< true if the light is set to begin changing to a new state, but has not yet began
			volatile uint8_t light_i; ///< the current PWM value of the light pin. | Used to incrament / decrement brightness
			volatile uint32_t light_time; ///< the current time from startup in milliseconds
			std::atomic<bool> *light_PSUVar; ///< a pointer to a (external) variable that contains the state of the PSU
	};
}
