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