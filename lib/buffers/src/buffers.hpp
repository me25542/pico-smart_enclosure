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
