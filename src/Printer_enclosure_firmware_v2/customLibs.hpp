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

#include "config.hpp"
#include <Arduino.h>
#include <pico/mutex.h>
#include <atomic>
#include <type_traits>

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

namespace buffers {
  /**
  * @brief class to implament a 255-byte circular buffer
  * @note isn't technically quite multicore-safe. Great for single core use (even writing from an ISR on that core), but *could* have issues with multicore use
  */
  class CircularBuffer {
    public:
      CircularBuffer();

      /**
      * @brief reads the next byte from the buffer
      * @note returns 0 if buffer is empty (0 can still be valid data)
      */
      uint8_t read();

      /**
      * @brief writes a byte to the buffer. returns true on sucessfull write, false if buffer full
      */
      bool write(uint8_t data);

      /**
      * @brief returns the number of remaining bytes to be read in the buffer
      */
      uint8_t available();

      /**
      * @brief returns true if the buffer is full
      */
      inline bool isFull();

      /**
      * @brief return true if the buffer is empty
      */
      inline bool isEmpty();

    private:
      volatile uint8_t circularBuffer_buffer[256]; ///< the actual buffer, a 256-item array of uint8_t's

      std::atomic<uint8_t> circularBuffer_writePos; ///< where to write to next in the buffer

      std::atomic<uint8_t> circularBuffer_readPos; ///< where to read from next in the buffer
  };
}

namespace menu {
  // Generic template defaults to false
  template<typename T>
  struct is_atomic : std::false_type {};

  // Specialization: true if std::atomic<U>
  template<typename U>
  struct is_atomic<std::atomic<U>> : std::true_type {};

  // Convenience variable
  template<typename T>
  inline constexpr bool is_atomic_v = is_atomic<T>::value;

  /**
  * @brief base class to store items on the main menu
  */
  class baseMenuItem {
    public:
      virtual ~baseMenuItem() = default;

      virtual String getName() const = 0;

      virtual String getSubData(uint8_t index) const = 0; // depricated

      virtual bool getSubsUsed() const = 0;

      virtual uint8_t getIndexOffset() const = 0;

      virtual void setData(uint32_t data) = 0;
      virtual uint32_t getData() const = 0;
      virtual String getDataString() const = 0;

      virtual void setTempData(uint32_t data) = 0;
      virtual uint32_t getTempData() const = 0;
      virtual String getTempDataString() const = 0;
      virtual void incrementTempData() = 0;
      virtual void decrementTempData() = 0;

      virtual uint32_t getMaxVal() const = 0;

      virtual uint32_t getMinVal() const = 0;

      virtual bool isDataSigned() const = 0;

      virtual size_t getDataSize() const = 0;

      virtual void setIsEditable(bool isEditable) = 0;
      virtual bool getIsEditable() const = 0;
  };


  /**
  * @brief class to store items on the main menu
  */
  template<typename T>
  class menuItem : public baseMenuItem {
    public:
      /**
      * @brief initializes a menu item
      * @note only use this constructor for bools
      * @param data a pointer to some variable
      * @param name a String with the name to be displayed on the menu
      */
      menuItem(T* data, String name)
        : menuItem_data(data), menuItem_minVal(0), menuItem_maxVal(1), menuItem_name(name), menuItem_indexOffset(0), menuItem_subsUsed(false) {
        menuItem_isEditable.store(true);
      }

      /**
      * @brief initializes a menu item
      * @note you don't need to use this constructor for data of the type bool
      * @param data a pointer to some variable
      * @param minVal the smallest allowable value of the variable pointed to by dat
      * @param maxVal the largest allowable value of the variable pointed to by dat
      * @param name a String with the name to be displayed on the menu
      */
      menuItem(T* data, T minVal, T maxVal, String name)
        : menuItem_data(data), menuItem_minVal(minVal), menuItem_maxVal(maxVal), menuItem_name(name), menuItem_indexOffset(0), menuItem_subsUsed(false) {
        menuItem_isEditable.store(true);
      }

      /**
      * @brief initializes a menu item
      * @note you don't need to use this constructor for data of the type bool
      * @param data a pointer to some variable
      * @param minVal the smallest allowable value of the variable pointed to by dat
      * @param maxVal the largest allowable value of the variable pointed to by dat
      * @param name a String with the name to be displayed on the menu
      * @param valueSubs substitutions for numbers
      */
      menuItem(T* data, T minVal, T maxVal, String name, std::vector<String> valueSubs)
        : menuItem_data(data), menuItem_minVal(minVal), menuItem_maxVal(maxVal), menuItem_name(name), menuItem_valueSubs(valueSubs), menuItem_indexOffset(0), menuItem_subsUsed(true) {
        menuItem_isEditable.store(true);
      }

      /**
      * @brief initializes a menu item
      * @note only use this constructor for bools
      * @param data a pointer to some variable
      * @param dataType the type of variable dat is
      * @param name a String with the name to be displayed on the menu
      * @param valueSubs substitutions for numbers
      */
      menuItem(T* data, String name, std::vector<String> valueSubs)
        : menuItem_data(data), menuItem_minVal(0), menuItem_maxVal(9999), menuItem_name(name), menuItem_valueSubs(valueSubs), menuItem_indexOffset(0), menuItem_subsUsed(true) {
        menuItem_isEditable.store(true);
      }
      
      /**
      * @brief initializes a menu item
      * @note you don't need to use this constructor for data of the type bool
      * @param data a pointer to some variable
      * @param minVal the smallest allowable value of the variable pointed to by dat
      * @param maxVal the largest allowable value of the variable pointed to by dat
      * @param name a String with the name to be displayed on the menu
      * @param valueSubs substitutions for numbers
      * @param indexOffset offset for provided indexes
      */
      menuItem(T* data, T minVal, T maxVal, String name, std::vector<String> valueSubs, uint8_t indexOffset)
        : menuItem_data(data), menuItem_minVal(minVal), menuItem_maxVal(maxVal), menuItem_name(name), menuItem_valueSubs(valueSubs), menuItem_indexOffset(indexOffset), menuItem_subsUsed(true) {
        menuItem_isEditable.store(true);
      }

      /**
      * @brief initializes a menu item
      * @note only use this constructor for bools
      * @param data a pointer to some variable
      * @param name a String with the name to be displayed on the menu
      * @param valueSubs substitutions for numbers
      * @param indexOffset offset for provided indexes
      */
      menuItem(T* data, String name, std::vector<String> valueSubs, uint8_t indexOffset)
        : menuItem_data(data), menuItem_minVal(0), menuItem_maxVal(9999), menuItem_name(name), menuItem_valueSubs(valueSubs), menuItem_indexOffset(indexOffset), menuItem_subsUsed(true) {
        menuItem_isEditable.store(true);
      }

      /**
      * @brief returns a String containing the name of the menu item
      */
      String getName() const override {
        return menuItem_name;
      }

      /**
      * @brief returns the substituted String for the value of the data
      */
      String getSubData(uint8_t index) const override {
        uint8_t offsetIndex = (index - menuItem_indexOffset);
        if (offsetIndex >= static_cast<uint8_t>(menuItem_valueSubs.size())) { // if the index provided is outside of the vector
          return String(index); // simply convert the vector to a String and return it
        }
        return menuItem_valueSubs[offsetIndex]; // return the String at the provided index in the vector
      }

      /**
      * @brief returns true if data substitution is used, false if otherwise
      */
      bool getSubsUsed() const override {
        return menuItem_subsUsed;
      }

      /**
      * @brief returns the index offset
      */
      uint8_t getIndexOffset() const override {
        return menuItem_indexOffset;
      }

      /**
      * @brief sets the value of the data variable pointed to by pointer passed in constructor
      */
      void setData(uint32_t data) override {
        if (!menuItem_isEditable.load()) return; // return without seting if menu item isn't editable

        using BaseType = std::remove_cv_t<std::remove_reference_t<decltype(*menuItem_data)>>;

        if constexpr (is_atomic_v<BaseType>) {
          menuItem_data->store(static_cast<typename BaseType::value_type>(data));
        } else {
          *menuItem_data = static_cast<BaseType>(data);
        }
      }

      /**
      * @brief returns the value of the variable pointed to by the internal menuItem_data pointer
      */
      uint32_t getData() const override {
        using BaseType = std::remove_cv_t<std::remove_reference_t<decltype(*menuItem_data)>>;

        if constexpr (is_atomic_v<BaseType>) {
          return static_cast<uint32_t>(menuItem_data->load());
        } else {
          return static_cast<uint32_t>(*menuItem_data);
        }
      }

      /**
      * returns the value of the data variable, converted to a string, with substitutions used where aplicable
      */
      String getDataString() const override {
        uint32_t data = getData();

        if (menuItem_subsUsed) {
          uint8_t offsetIndex = (data - menuItem_indexOffset);

          if (offsetIndex < static_cast<uint8_t>(menuItem_valueSubs.size())) { // if the index provided is inside the vector
            return menuItem_valueSubs[offsetIndex]; // return the String at the provided index in the vector
          }
        }

        return String(data);
      }

      /**
      * @brief use to edit temporary data (not the actual variable pointed to as the data)
      * @note value is returned and stored as a uint32_t, you will have to cast it if negative
      */
      void setTempData(uint32_t data) override {
        menuItem_tempVal = data;
      }

      /**
      * @brief returns the value of the temporary data variable
      * @note value is returned and stored as a uint32_t, you will have to cast it if negative
      */
      uint32_t getTempData() const override {
        return menuItem_tempVal;
      }

      /**
      * @brief returns the value of the temporary data variable, converted to a string, with substitutions used where aplicable
      */
      String getTempDataString() const override {
        if (menuItem_subsUsed) {
          uint8_t offsetIndex = (menuItem_tempVal - menuItem_indexOffset);

          if (offsetIndex < static_cast<uint8_t>(menuItem_valueSubs.size())) { // if the index provided is inside the vector
            return menuItem_valueSubs[offsetIndex]; // return the String at the provided index in the vector
          }
        }

        return String(menuItem_tempVal);
      }

      /**
      * @brief adds one to the temporary data, clamping it within the min and max values
      */
      void incrementTempData() override {
        if (++menuItem_tempVal > menuItem_maxVal || menuItem_tempVal < menuItem_minVal) { // incrament temporary data and check if overflowed
          menuItem_tempVal = menuItem_minVal; // if it overflowed, set it to the minimum value
        }
      }

      /**
      * @brief subtracts one from the temporary data, clamping it within the min and max values
      */
      void decrementTempData() override {
        if (--menuItem_tempVal < menuItem_minVal || menuItem_tempVal > menuItem_maxVal) { // incrament temporary data and check if underflowed
          menuItem_tempVal = menuItem_maxVal; // if it underflowed, set it to the maximum value
        }
      }

      /**
      * @brief returns the maximum allowable value for whatever is pointed to by the menu item
      * @note value is returned and stored as a uint32_t, you will have to cast it if negative
      */
      uint32_t getMaxVal() const override {
        return menuItem_maxVal;
      }

      /**
      * @brief returns the minimum allowable value for whatever is pointed to by the menu item
      * @note value is returned and stored as a uint32_t, you will have to cast it if negative
      */
      uint32_t getMinVal() const override {
        return menuItem_minVal;
      }

      bool isDataSigned() const override {
        return std::is_signed<T>::value;
      }

      /**
      * @brief returns the size of the data in bytes
      */
      size_t getDataSize() const override {
        return sizeof(T);
      }

      /**
      * @brief returns a pointer to whatever is pointed to by the menu item
      */
      T* getDataVar() {
        return menuItem_data;
      }

      /**
      * @brief sets if the menu item is marked as editable
      * @note this mark is enforced; setData() won't do anything if the menu item isn't editable
      */
      void setIsEditable(bool isEditable) override {
        menuItem_isEditable.store(isEditable);
      }

      /**
      * @brief returns if the menu item is marked as editable
      * @note this mark is enforced; setData() won't do anything if the menu item isn't editable
      */
      bool getIsEditable() const override {
        return menuItem_isEditable.load();
      }
    
    private:
      T *menuItem_data; ///< a pointer to the specified data type

      std::atomic<bool> menuItem_isEditable; ///< tracks if the menu item is editable

      uint32_t menuItem_tempVal; ///< a variable to hold new values of data before they are commited
      
      uint32_t menuItem_minVal; ///< the minumun allowable value for the thing pointed to by data
      
      uint32_t menuItem_maxVal; ///< the maximum allowable value for the thing pointed to by data
      
      String menuItem_name; ///< the name for the menu item to be displayed
      
      std::vector<String> menuItem_valueSubs; ///< a vector of Strings to use when substituting strings for numbers

      const uint8_t menuItem_indexOffset; ///< the amount the vector index will be offset relative to the provided one
      
      const bool menuItem_subsUsed; ///< tracks if number-character substitution is used
  };
}

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
