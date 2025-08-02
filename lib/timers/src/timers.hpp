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
#include <pico/mutex.h>

namespace timers {
	constexpr bool TIMER_IS_NOT_SET = false;
	constexpr bool TIMER_IS_SET = true;
	constexpr bool TIMER_IS_NOT_DONE = false;
	constexpr bool TIMER_IS_DONE = true;

	/**
	* @brief provides a multicore-safe timer that expires a set duration after being started
	*/
	class Timer {
		public:
			Timer();

			/**
			* @brief Sets a new timer for the specified duration in milliseconds
			* @param duration the timer duration in milliseconds
			*/
			void set(uint32_t duration);

			/**
			* @brief Stops the timer if one is running.
			* @return The time remaining (in milliseconds) if a timer was running, or 0 if no timer was active
			*/
			uint32_t stop();

			/**
			* @brief returns the number of milliseconds left on the timer, or zero if no timer is set
			*/
			uint32_t remaining();

			/**
			* @brief returns true if the timer is done, false otherwise. If the timer is done, the first time this is called it will be reset. Returns false if no timer is set
			*/
			bool isDone();

			/**
			* @brief returns true if there is a timer set, false otherwise
			*/
			bool isSet();

			/**
			* @brief returns the time since the timer finished (zero if the timer is not done)
			*/
			uint32_t overdone();

		private:
			mutex_t timerMutex; ///< Mutex for multicore-safe operations

			volatile uint32_t timeDone; ///< Time (ms) when the timer will expire

			volatile bool timerSet; ///< True if timer is active, false otherwise

			/**
			* @brief returns the number of milliseconds left on the timer, or zero if no timer is set | DOES NOT AQUIRE MUTEX! NOT MULTICORE SAFE!
			*/
			uint32_t remaining_internal();
	};

	/**
	* @brief provides a multicore-safe timer that expires a set duration (in microseconds) after being started
	*/
	class Timer_us {
		public:
			Timer_us();

			/**
			* @brief Sets a new timer for the specified duration in microseconds
			* @param duration the timer duration in microseconds
			*/
			void set(uint32_t duration);

			/**
			* @brief Stops the timer if one is running.
			* @return The time remaining (in microseconds) if a timer was running, or 0 if no timer was active
			*/
			uint32_t stop();

			/**
			* @brief returns the number of microseconds left on the timer, or zero if no timer is set
			*/
			uint32_t remaining();

			/**
			* @brief returns true if the timer is done, false otherwise. If the timer is done, the first time this is called it will be reset. Returns false if no timer is set
			*/
			bool isDone();

			/**
			* @brief returns true if there is a timer set, false otherwise
			*/
			bool isSet();

			/**
			* @brief returns the time (µs) since the timer finished (zero if the timer is not done)
			*/
			uint32_t overdone();

		private:
			mutex_t timerMutex; ///< Mutex for multicore-safe operations

			volatile uint32_t timeDone; ///< Time (µs) when the timer will expire

			volatile bool timerSet; ///< True if timer is active, false otherwise

			/**
			* @brief returns the number of microseconds left on the timer, or zero if no timer is set | DOES NOT AQUIRE MUTEX! NOT MULTICORE SAFE!
			*/
			uint32_t remaining_internal();
	};
}
