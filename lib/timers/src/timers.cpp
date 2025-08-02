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

#include "timers.hpp"

//** - Timer - ********************************************************************************************************************************************************************

timers::Timer::Timer()
	: timeDone(0), timerSet(TIMER_IS_NOT_SET) {
	mutex_init(&timerMutex);
}

void timers::Timer::set(uint32_t duration) {
	mutex_enter_blocking(&timerMutex); // wait for any other functions in this timer to finish

	uint32_t time = millis(); // find the time the timer was set
	timerSet = TIMER_IS_SET; // record that a timer is set
	timeDone = time + duration; // record when it will be done

	mutex_exit(&timerMutex); // release mutex — allow other functions to access this timer
}

uint32_t timers::Timer::stop() {
	mutex_enter_blocking(&timerMutex); // wait for any other functions in this timer to finish

	uint32_t toReturn = remaining_internal(); // find the time remaining
	timerSet = TIMER_IS_NOT_SET; // record that a timer is not set
	
	mutex_exit(&timerMutex); // release mutex — allow other functions to access this timer
	
	return toReturn; // return the time remaining
}

uint32_t timers::Timer::remaining() {
	mutex_enter_blocking(&timerMutex); // wait for any other functions in this timer to finish

	uint32_t toReturn;

	if (timerSet == TIMER_IS_NOT_SET) { // if there is no timer set
		toReturn = 0;

	} else {
		uint32_t time = millis(); // store the current time in `time`
		uint32_t timeRemaining_raw = timeDone - time; // find out the time remaining, not acounting for overflows (edge cases). | If `time` is too much larger than `timeDone`, this could overflow in a way that won't be caught

		if (timeRemaining_raw < UINT32_MAX / 2) { // if there wasn't an overflow of `timeRemaining_raw`
			toReturn = timeRemaining_raw; // we will return the value of `timeRemaining_raw`

		} else { // if `timeRemaining_raw` overflowed
			toReturn = 0; // we will return zero
		}
	}
	
	mutex_exit(&timerMutex); // release mutex — allow other functions to access this timer

	return toReturn;
}

bool timers::Timer::isDone() {
	mutex_enter_blocking(&timerMutex); // wait for any other functions in this timer to finish

	bool toReturn;

	if (timerSet == TIMER_IS_NOT_SET) { // if a timer is not set
		toReturn = TIMER_IS_NOT_DONE;

	} else { // if a timer is set
		uint32_t time = millis(); // find the time
		uint32_t timeRemaining_raw = timeDone - time; // find out the time remaining, not acounting for overflows. | If `time` is too much larger than `timeDone`, this could overflow in a way that won't be caught

		if (timeRemaining_raw < UINT32_MAX / 2) {
			toReturn = TIMER_IS_NOT_DONE; // still time left

		} else {
			timerSet = TIMER_IS_NOT_SET; // timer expired, clear it
			toReturn = TIMER_IS_DONE;
		}
	}

	mutex_exit(&timerMutex); // release mutex — allow other functions to access this timer

	return toReturn;
}

bool timers::Timer::isSet() {
	mutex_enter_blocking(&timerMutex); // wait for any other functions in this timer to finish

	bool toReturn = timerSet;

	mutex_exit(&timerMutex); // release mutex — allow other functions to access this timer

	return toReturn;
}

uint32_t timers::Timer::overdone() {
	mutex_enter_blocking(&timerMutex); // wait for any other functions in this timer to finish

	uint32_t toReturn;

	if (timerSet == TIMER_IS_NOT_SET) {
		toReturn = 0;

	} else {
		uint32_t time = millis(); // store the current time in `time`

		toReturn = time - timeDone;

		if (toReturn >= UINT32_MAX / 2) { // if we overflowed (timer not done, or this function called long after timer finished)
			toReturn = 0;
		}
	}

	mutex_exit(&timerMutex); // release mutex — allow other functions to access this timer

	return toReturn;
}

uint32_t timers::Timer::remaining_internal() {
	if (timerSet == TIMER_IS_NOT_SET) { // if there is no timer set
		return 0;
	}

	uint32_t toReturn;
	uint32_t time = millis(); // store the current time in `time`
	uint32_t timeRemaining_raw = timeDone - time; // find out the time remaining, not acounting for overflows (edge cases). | If `time` is too much larger than `timeDone`, this could overflow in a way that won't be caught

	if (timeRemaining_raw < UINT32_MAX / 2) { // if there wasn't an overflow of `timeRemaining_raw`
		toReturn = timeRemaining_raw; // we will return the value of `timeRemaining_raw`

	} else { // if `timeRemaining_raw` overflowed
		toReturn = 0; // we will return zero
	}

	return toReturn;
}

//** - Timer_us - *****************************************************************************************************************************************************************

timers::Timer_us::Timer_us()
	: timeDone(0), timerSet(TIMER_IS_NOT_SET) {
	mutex_init(&timerMutex);
}

void timers::Timer_us::set(uint32_t duration) {
	mutex_enter_blocking(&timerMutex); // wait for any other functions in this timer to finish

	uint32_t time = micros(); // find the time the timer was set
	timerSet = TIMER_IS_SET; // record that a timer is set
	timeDone = time + duration; // record when it will be done

	mutex_exit(&timerMutex); // release mutex — allow other functions to access this timer
}

uint32_t timers::Timer_us::stop() {
	mutex_enter_blocking(&timerMutex); // wait for any other functions in this timer to finish

	uint32_t toReturn = remaining_internal(); // find the time remaining
	timerSet = TIMER_IS_NOT_SET; // record that a timer is not set
	
	mutex_exit(&timerMutex); // release mutex — allow other functions to access this timer
	
	return toReturn; // return the time remaining
}

uint32_t timers::Timer_us::remaining() {
	mutex_enter_blocking(&timerMutex); // wait for any other functions in this timer to finish

	uint32_t toReturn;

	if (timerSet == TIMER_IS_NOT_SET) { // if there is no timer set
		toReturn = 0;

	} else {
		uint32_t time = micros(); // store the current time in `time`
		uint32_t timeRemaining_raw = timeDone - time; // find out the time remaining, not acounting for overflows (edge cases). | If `time` is too much larger than `timeDone`, this could overflow in a way that won't be caught

		if (timeRemaining_raw < UINT32_MAX / 2) { // if there wasn't an overflow of `timeRemaining_raw`
			toReturn = timeRemaining_raw; // we will return the value of `timeRemaining_raw`

		} else { // if `timeRemaining_raw` overflowed
			toReturn = 0; // we will return zero
		}
	}
	
	mutex_exit(&timerMutex); // release mutex — allow other functions to access this timer

	return toReturn;
}

bool timers::Timer_us::isDone() {
	mutex_enter_blocking(&timerMutex); // wait for any other functions in this timer to finish

	bool toReturn;

	if (timerSet == TIMER_IS_NOT_SET) { // if a timer is not set
		toReturn = TIMER_IS_NOT_DONE;

	} else { // if a timer is set
		uint32_t time = micros(); // find the time
		uint32_t timeRemaining_raw = timeDone - time; // find out the time remaining, not acounting for overflows. | If `time` is too much larger than `timeDone`, this could overflow in a way that won't be caught

		if (timeRemaining_raw < UINT32_MAX / 2) {
			toReturn = TIMER_IS_NOT_DONE; // still time left

		} else {
			timerSet = TIMER_IS_NOT_SET; // timer expired, clear it
			toReturn = TIMER_IS_DONE;
		}
	}

	mutex_exit(&timerMutex); // release mutex — allow other functions to access this timer

	return toReturn;
}

bool timers::Timer_us::isSet() {
	mutex_enter_blocking(&timerMutex); // wait for any other functions in this timer to finish

	bool toReturn = timerSet;

	mutex_exit(&timerMutex); // release mutex — allow other functions to access this timer

	return toReturn;
}

uint32_t timers::Timer_us::overdone() {
	mutex_enter_blocking(&timerMutex); // wait for any other functions in this timer to finish

	uint32_t toReturn;

	if (timerSet == TIMER_IS_NOT_SET) {
		toReturn = 0;

	} else {
		uint32_t time = micros(); // store the current time in `time`

		toReturn = time - timeDone;

		if (toReturn >= UINT32_MAX / 2) { // if we overflowed (timer not done, or this function called long after timer finished)
			toReturn = 0;
		}
	}

	mutex_exit(&timerMutex); // release mutex — allow other functions to access this timer

	return toReturn;
}

uint32_t timers::Timer_us::remaining_internal() {
	if (timerSet == TIMER_IS_NOT_SET) { // if there is no timer set
		return 0;
	}

	uint32_t toReturn;
	uint32_t time = micros(); // store the current time in `time`
	uint32_t timeRemaining_raw = timeDone - time; // find out the time remaining, not acounting for overflows (edge cases). | If `time` is too much larger than `timeDone`, this could overflow in a way that won't be caught

	if (timeRemaining_raw < UINT32_MAX / 2) { // if there wasn't an overflow of `timeRemaining_raw`
		toReturn = timeRemaining_raw; // we will return the value of `timeRemaining_raw`

	} else { // if `timeRemaining_raw` overflowed
		toReturn = 0; // we will return zero
	}

	return toReturn;
}
