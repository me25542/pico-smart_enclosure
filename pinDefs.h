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

#ifndef PINDEFS_H
#define PINDEFS_H


  //  Pin definitions:
  //  outputs, high pin# - low pin#
  pinMode(ledPin, OUTPUT);
  pinMode(printDoneLightPin, OUTPUT);
  pinMode(printingLightPin, OUTPUT);
  pinMode(cooldownLightPin, OUTPUT);
  pinMode(standbyLightPin, OUTPUT);
  pinMode(errorLightPin, OUTPUT);
  pinMode(lightsPin, OUTPUT);
  pinMode(heater2Pin, OUTPUT);
  pinMode(heater1Pin, OUTPUT);
  pinMode(psPin, OUTPUT);
  pinMode(fanPin, OUTPUT);

  //  inputs, high pin# - low pin#
  pinMode(pokPin, INPUT_PULLDOWN);

  // set initial output pin values, high pin# - low pin#:
  digitalWrite(ledPin, LOW);  //  turn off the built-in LED
  digitalWrite(printDoneLightPin, LOW);  //  turn off the "print done" light
  digitalWrite(printingLightPin, LOW);  //  turn off the "printing" light
  digitalWrite(cooldownLightPin, LOW);  //  turn off the "cooldown" light
  digitalWrite(standbyLightPin, LOW);  //  turn off the "standby" light
  digitalWrite(errorLightPin, LOW);  //  turn off the "error" light
  digitalWrite(lightsPin, LOW);  //  turn off the enclosure lights
  digitalWrite(heater1Pin, HIGH);  //  turn off heater 1
  digitalWrite(heater2Pin, HIGH);  //  turn off heater 2
  digitalWrite(fanPin, LOW);  //  turn off the fan
  digitalWrite(psPin, HIGH);  //  turn on the power supply

#endif