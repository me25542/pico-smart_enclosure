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

#ifndef TEMPSENSORSSETUP_H
#define TEMPSENSORSSETUP_H

  //  start I2C0 (the sensor bus):
  Wire.setSDA(I2C0_SDA);  //  set the SDA of I2C0
  Wire.setSCL(I2C0_SCL);  //  set the SCL of I2C0
  Wire.begin();  //  Initialize the I2C0 bus as the master

  delay(1);  //  wait for a tiny bit

  //  start I2C temp sensors and set mode to error if it fails:
  if (! heaterTempSensor.begin(heaterTempSensorAdress)) {  //  try to start the heater temp sensor and check if it worked
    mode = 0;  //  set mode to error if it didn't work
    errorOrigin = 3;  //  record where the error came from
    errorInfo = 1;  //  record which sensor is at fault
  }

  if (! inTempSensor.begin(inTempSensorAdress)) {  //  try to start the in temp sensor and check if it worked
    mode = 0;  //  set mode to error if it didn't work
    errorOrigin = 3;  //  record where the error came from
    errorInfo = 2;  //  record which sensor is at fault
  }

  if (! outTempSensor.begin(outTempSensorAdress)) {  //  try to start the out temp sensor and check if it worked
    mode = 0;  //  set mode to error if it didn't work
    errorOrigin = 3;  //  record where the error came from
    errorInfo = 3;  //  record which sensor is at fault
  }

  //  set I2C temp sensor resolution:
  heaterTempSensor.setResolution(i2cTempSensorRes);
  inTempSensor.setResolution(i2cTempSensorRes);
  outTempSensor.setResolution(i2cTempSensorRes);

  //  check if the sensors are connected:
  areSensorsPresent();

  //  get temps:
  getTemp();
  
#endif