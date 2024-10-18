/*
 * Copyright © 2024 Dalen Hardy
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * 
 * Except as contained in this notice, the name of Dalen Hardy shall not be used in advertising or otherwise
 * to promote the sale, use or other dealings in this Software without prior written authorization from Dalen Hardy.
*/

#ifndef PRINTERI2CSETUP_H
#define PRINTERI2CSETUP_H


  //  start I2C1 (the priter's bus):
  Wire1.setSDA(I2C1_SDA);  //  set the SDA of I2C1
  Wire1.setSCL(I2C1_SCL);  //  set the SCL of I2C1
  Wire1.begin(0x08);  // Initialize the I2C1 bus with the address 0x08 (8)
  Wire1.onRequest(requestEvent);  //  set the function to call when the printer requests data via I2C
  Wire1.onReceive(receiveEvent);  //  set the function to call when the printer sends data via I2C

#endif