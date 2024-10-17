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

#ifndef SERVOSSETUP_H
#define SERVOSSETUP_H


  //  start servos:
  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);

  //  move servos to home position (closed, or 0deg):
  servo1.write(servo1Closed);
  servo2.write(servo2Closed);

#endif