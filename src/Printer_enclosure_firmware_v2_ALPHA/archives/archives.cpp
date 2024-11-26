/*  we don't need this anymore, we have a better one

//  the function called on data received:
void receiveEvent(int howMany) {
  #ifdef debug
  Serial.println("recieveEvent() called");  //  print a message over serial (USB)
  #endif
  while (Wire1.available()) {  //  while there are received bytes in the buffer:
    byte recVal = Wire1.read();  // set recVal ("it") to the first byte in the buffer
    Serial.print("byte recieved: ");  //  print a message over serial (USB)
    Serial.println(recVal);  //  print a message over serial (USB)

    if (recVal < 4) {  //  if it is 0-3:
      if (recVal == 0) {
        errorOrigin = 4;  //  record that the printer commanded the error
        errorInfo = mode;  //  record the old mode
      }
      mode = recVal;  //  set the mode

    } else if (recVal < 10) {  //  if it is 4-9:
      //  set other things:
      switch (recVal) {  //  switch... case statement; checks the value of recVal and does the coresponding action
        case 4:  //  if it is 4:
          printDone = true;  //  set print done
          break;

        case 5:  //  if it is 5:
          printDone = false;  //  set print not done
          break;

        case 6:  //  if it is 6:
          lightSetState = true;  //  remember to turn the lights on
          break;

        case 7:  //  if it is 7:
          lightSetState = false;  //  remember to turn the lights off
          break;

        case 8:
          changeLights = true;  //  remember to change the state of the lights
          break;
      }

    } else if (recVal < 100) {  //  if it is 10-99:
      globalSetTemp = recVal;  //  set the target temp

    } else if (recVal < 105) {  //  if it is 100-104:
      maxFanSpeed = byte(map(long(recVal), 100, 104, 0, 255));  //  set maxFanSpeed to a value from 0-255 based on the recieved value
    
    } else {
      mode = 0;
      errorOrigin = 5;
      errorInfo = recVal;
    }
  }
}
*/

/*
//  the function to set the value of the lights (turn the lights on or off)
void setLights(bool on) {
  #ifdef debug
  Serial.print("setLights(");  //  print a message over serial (USB)
  Serial.print(on);  //  print a message over serial (USB)
  Serial.println(") called");  //  print a message over serial (USB)
  #endif

  if (lightState != on) {  //  check if the state of the lights matches their target state. in this case, "on" is a variable of the type "bool"
    dontTurnOffThePSU = true;  //  make shure there will be power through the whole diming / brightening process

    if (on) {  //  if we are turning on the lights:
      setPSU(true);  //  turn on the PSU
    }

    smoothChange(lightsPin, on, dimingTime);  //  smoothly change the value of the lights pin to what it should be

    lightState = on;  //  remember the state of the lights
    dontTurnOffThePSU = false;  //  let the other core turn off the PSU again
  }
  
  lightSetState = lightState;  //  make shure the target state of the lights is updated if the light switch was pressed or the printer requested a change
}
*/

/*
//  a function to change the lights value (turn them off if on and vice versa). called on light switch relese
void lightChange() {
  #ifdef debug
  Serial.println("lightChange() called");  //  print a message over serial (USB)
  #endif

  setLights(! lightState);  //  print a message over serial (USB)
  changeLights = false;  //  disable the flag to change the light's state
}
*/

/*
//  the function to set the state of the mode indicator lights
void setIndicatorLights(bool errorLight_On, bool printingLight_On, bool cooldownLight_On, bool standbyLight_On) {
  #ifdef debug
  Serial.print("setIndicatorLights(");  //  print a message over serial (USB)
  Serial.print(errorLight_On);  //  print a message over serial (USB)
  Serial.print(", ");  //  print a message over serial (USB)
  Serial.print(printingLight_On);  //  print a message over serial (USB)
  Serial.print(", ");  //  print a message over serial (USB)
  Serial.print(cooldownLight_On);  //  print a message over serial (USB)
  Serial.print(", ");  //  print a message over serial (USB)
  Serial.print(standbyLight_On);  //  print a message over serial (USB)
  Serial.println(") called");  //  print a message over serial (USB)
  #endif

  //  these will only be created the first time this function is called
  static bool el_State = false;  //  tracks the state of the error light
  static bool pl_State = false;  //  tracks the state of the printing light
  static bool cl_State = false;  //  tracks the state of the cooldown light
  static bool sl_State = false;  //  tracks the state of the standby light

  //  turn off the lights being turned off:
  if ((! errorLight_On) && (el_State != errorLight_On)) {  //  if we are turning the light off AND we will be changing its state:
    smoothChange(errorLightPin, errorLight_On, il_DimingTime);  //  smoothly change the light to it's new value
  }
  if ((! printingLight_On) && (pl_State != printingLight_On)) {  //  if we are turning the light off AND we will be changing its state:
    smoothChange(printingLightPin, printingLight_On, il_DimingTime);  //  smoothly change the light to it's new value
  }
  if ((! cooldownLight_On) && (cl_State != cooldownLight_On)) {  //  if we are turning the light off AND we will be changing its state:
    smoothChange(cooldownLightPin, cooldownLight_On, il_DimingTime);  //  smoothly change the light to it's new value
  }
  if ((! standbyLight_On) && (sl_State != standbyLight_On)) {  //  if we are turning the light off AND we will be changing its state:
    smoothChange(standbyLightPin, standbyLight_On, il_DimingTime);  //  smoothly change the light to it's new value
  }

  //  turn on the lights being turned on:
  if ((errorLight_On) && (el_State != errorLight_On)) {  //  if we are turning the light on AND we will be changing its state:
    smoothChange(errorLightPin, errorLight_On, il_DimingTime);  //  smoothly change the light to it's new value
  }
  if ((printingLight_On) && (pl_State != printingLight_On)) {  //  if we are turning the light on AND we will be changing its state:
    smoothChange(printingLightPin, printingLight_On, il_DimingTime);  //  smoothly change the light to it's new value
  }
  if ((cooldownLight_On) && (cl_State != cooldownLight_On)) {  //  if we are turning the light on AND we will be changing its state:
    smoothChange(cooldownLightPin, cooldownLight_On, il_DimingTime);  //  smoothly change the light to it's new value
  }
  if ((standbyLight_On) && (sl_State != standbyLight_On)) {  //  if we are turning the light on AND we will be changing its state:
    smoothChange(standbyLightPin, standbyLight_On, il_DimingTime);  //  smoothly change the light to it's new value
  }

  //  remember the state of the indicator lights:
  el_State = errorLight_On;
  pl_State = printingLight_On;
  cl_State = cooldownLight_On;
  sl_State = standbyLight_On;
}
*/

/*
//  the function to set the print done light
void setPrintDoneLight(bool pdl_State) {
  #ifdef debug
  Serial.print("setPrintDoneLight(");  //  print a message over serial (USB)
  Serial.print(pdl_State);  //  print a message over serial (USB)
  Serial.println(") called");  //  print a message over serial (USB)
  #endif

  //  this will only be created the first time this function is called:
  static bool printDoneLight_On = false;  //  tracks the state of the print done light

  if (printDoneLight_On != pdl_State) {  //  if we are actually going to change the state of the print done light:
    smoothChange(printDoneLightPin, pdl_State, pdl_DimingTime);  //  smoothly change the state of the print done light
  }
  printDoneLight_On = pdl_State;  //  remember the state of the print done light
}
*/

/*
//  a function to smoothly turn on or off a pin using PWM
void smoothChange(uint8_t pin, bool setState, int32_t time) {
  #ifdef debug
  Serial.print("smoothChange(");  //  print a message over serial (USB)
  Serial.print(pin);  //  print a message over serial (USB)
  Serial.print(",");  //  print a message over serial (USB)
  Serial.print(setState);  //  print a message over serial (USB)
  Serial.print(",");  //  print a message over serial (USB)
  Serial.print(time);  //  print a message over serial (USB)
  Serial.println(") called");  //  print a message over serial (USB)
  #endif

  if (setState) {  //  if we want to turn the pin on:
    for (uint8_t i = 0; i < 255; i++) {  //  while "i" (which starts at 0) is less than 255, run the enclosed code and add one to "i"
      analogWrite(pin, i);  //  write the value contained in i to the pin
      delay(time);  //  wait for the set amount of time
    }

  } else {  //  if we want to turn the pin off:
    for (uint8_t i = 255; i > 0; i--) {  //  while "i" (which starts at 255) is greater than 0, rin the enclosed code and subtract one from "i"
      analogWrite(pin, i);  //  write the value contained in i to the pin
      delay(time);  //  wait for the set amount of time
    }
  }
  digitalWrite(pin, setState);
}
*/
