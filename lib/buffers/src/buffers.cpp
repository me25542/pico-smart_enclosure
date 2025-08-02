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

#include "buffers.hpp"

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
