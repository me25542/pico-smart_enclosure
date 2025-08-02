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

namespace relays {
	/**
	* @brief class to handle relays
	* @note NOT FULLY MULTICORE SAFE! DO NOT CALL FROM MULTIPLE CORES AT ONCE!
	*/
	class Relay {
		public:
			Relay(pin_size_t pin);

			Relay(pin_size_t pin, bool onState);

			Relay(pin_size_t pin, bool onState, bool state);

			/**
			* @brief sets the relay to a specified on / off state
			*/
			void setState(bool state);

			/**
			* @brief turns on the relay, equivilant to setstae(true)
			*/
			void turnOn();

			/**
			* @brief turns off the relay, equivilant to setstae(false)
			*/
			void turnOff();

			/**
			* @brief gets the current on / off state of the relay
			*/
			bool getState();

			/**
			* @brief toggles the current on / off state of the relay
			* @note not fully multicore safe
			*/
			void changeState();

		protected:
			/**
			* @brief actually sets the relay to the set on / off state
			*/
			virtual void relay_update();

			const pin_size_t relay_pin; ///< the pin the relay is connected to
			const bool relay_onState; ///< the HIGH / LOW state the relay pin should be at for the relay to be on
			std::atomic<bool> relay_state; ///< the current HIGH / LOW state of the relay pin
	};

	/**
	* @brief class to handle heaters, based on the Relay class
	* @note NOT FULLY MULTICORE SAFE! DO NOT CALL FROM MULTIPLE CORES AT ONCE!
	*/
	class Heater : public Relay {
		public:
			Heater(pin_size_t pin, bool onState);

			/**
			* @brief sets if the heater is enabled
			*/
			void setEnabled(bool enabled);

			/**
			* @brief enables the heater
			*/
			void enable();

			/**
			* @brief disables the heater
			*/
			void disable();

			/**
			* @brief returns if the heater is enabled or not
			*/
			bool getEnabled();

		protected:
			/**
			* @brief actually sets the heater to the set on / off state, or to off if disabled
			*/
			void relay_update() override;

			const bool heater_offState;
			std::atomic<bool> heater_enabled;
	};
}