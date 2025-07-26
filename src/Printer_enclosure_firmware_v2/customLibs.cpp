#include <atomic>
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

#include "customLibs.hpp"

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

//** - CircularBuffer - ***********************************************************************************************************************************************************

buffers::CircularBuffer::CircularBuffer() {
  circularBuffer_readPos = 0;
  circularBuffer_writePos = 0;

  // write each byte in the buffer to NULL (0)
  for (uint8_t i = 0; i != 255; i++) {
    circularBuffer_buffer[i] = 0;
  }
}

uint8_t buffers::CircularBuffer::read() {
  if (!isEmpty()) { // if the buffer isn't empty
    return circularBuffer_buffer[circularBuffer_readPos++]; // return a byte from the buffer, move where to read from next
    // remember, circularBuffer_readPos is a uint8_t and will wrap around on its own

  } else {
    return 0; // return NULL (0) and don't incrament the read position to avoid returning garbage data or geting the pointer out of sync
  }
}

bool buffers::CircularBuffer::write(uint8_t data) {
  if (isFull()) { // if the buffer is full
    return false; // the buffer is full; return with error
  }

  circularBuffer_buffer[circularBuffer_writePos++] = data; // write a byte to the buffer, move where to write to next
    // remember, circularBuffer_writePos is a uint8_t and will wrap around on its own

  return true;
}

uint8_t buffers::CircularBuffer::available() {
  return circularBuffer_writePos - circularBuffer_readPos; // return the number of bytes still to be read
    // Relies on uint8_t wrapping behavior
}

inline bool buffers::CircularBuffer::isFull() {
  return static_cast<uint8_t>(circularBuffer_writePos + 1) == circularBuffer_readPos; // return true if the write position is one away from the read position
}

inline bool buffers::CircularBuffer::isEmpty() {
  return circularBuffer_writePos == circularBuffer_readPos; // return true if the read and write positions are the same (the buffer is empty)
}

//** - Light - ********************************************************************************************************************************************************************

lights::Light::Light(uint8_t pin, uint8_t speed, bool onState, bool state, std::atomic<bool> *PSUVar)
  : light_pin(pin), light_speed(speed), light_onState(onState), light_state(state), light_PSUVar(PSUVar) {
  pinMode(light_pin, OUTPUT);
  digitalWrite(light_pin, light_state);
  light_time = millis();
}

uint8_t lights::Light::getPin() {
  return light_pin;
}

bool lights::Light::needsPower() {
  if (light_onState) {
    return (light_state || light_changing);

  } else {
    return (!light_state || light_changing);
  }
}

void lights::Light::setSpeed(uint8_t speed) {
  light_speed = speed;
}

uint8_t lights::Light::getSpeed() {
  return light_speed;
}

void lights::Light::setState(bool state) {
  //  this if statemet is to reverse the set state if the lights are on when LOW, as the internal state is equal to HIGH or LOW not on and off
  if (light_onState) {
    if (state != light_state) {
      light_toChange = true;
      light_changing = true;
    }

    light_state = state;

  } else {
    if (state == light_state) {
      light_toChange = true;
      light_changing = true;
    }

    light_state = !state;
  }
}

void lights::Light::changeState() {
  light_state = !light_state;
  light_toChange = true;
  light_changing = true;
}

bool lights::Light::getState() {
  //  this if statement flips the returned state if the lights are on when LOW, as the internal state is equal to HIGH or LOW not on and off
  if (light_onState) {
    return light_state;

  } else {
    return !light_state;
  }
}

void lights::Light::tick() {
  if (!light_changing) { // If we are not in the process of changing the light state
    return; // return without doing anything
  }

  if (light_toChange) { // initialization for if we haven't started changing yet
    light_toChange = false;

    if (light_state) { // if we are going to HIGH
      light_i = 0; // Start from fully LOW

    } else { // if we are going to LOW
      light_i = 255; // Start from fully HIGH
    }

    while (!light_PSUVar->load()); // wait for the PSU to be on

    light_time = millis(); // reset the last updtate time so that the lights don't turn on all at once
  }

  uint32_t currentTime = millis();
  uint32_t elapsedTime = currentTime - light_time;

  if (!(elapsedTime >= light_speed)) { // if it's not time to update the light brightness
    return; // return without doing anything
  }

  light_time = currentTime; // Update the last update time

  if (light_state) { // Turning the light HIGH
    // Calculate the increment to apply this tick
    uint8_t increment = min(uint8_t(elapsedTime / light_speed), uint8_t(255 - light_i));
    light_i += increment;
    analogWrite(light_pin, light_i);

    if (light_i >= 255) { // If we've reached fully HIGH
      light_i = 255; // Clamp to max value
      light_changing = false; // Stop changing the light
    }

  } else { // Turning the light to LOW
    // Calculate the decrement to apply this tick
    uint8_t decrement = min(uint8_t(elapsedTime / light_speed), light_i);
    light_i -= decrement;
    analogWrite(light_pin, light_i);

    if (light_i <= 0) { // If we've reached fully LOW
      light_i = 0; // Clamp to min value
      light_changing = false; // Stop changing the light
      
    } // if (light_i <= 0)
  } // else (  if (light_state)  )
} // lights::Light::tick()

void lights::Light::blip(uint32_t microseconds) {
  analogWrite(light_pin, (light_state * 255) ^ 4); // set the lights to 4/255 of set to LOW, 251/255 if set to HIGH
  delayMicroseconds(microseconds);
  analogWrite(light_pin, (light_state * 255));
}
