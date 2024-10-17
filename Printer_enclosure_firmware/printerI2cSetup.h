/*
 * Writen by Dalen Hardy 2024
 * 
 * This code is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * In no event shall the author be liable for any claim, damages, or other liability, 
 * whether in an action of contract, tort, or otherwise, arising from, out of, or in connection with the code 
 * or the use or other dealings in the code.
 * 
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