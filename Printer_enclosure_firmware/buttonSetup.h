#ifndef BUTTONSETUP_H
#define BUTTONSETUP_H

    //  set switch pinModes
  door_switch.attach(doorSwitchPin, INPUT_PULLUP);
  light_switch.attach(lightSwitchPin, INPUT_PULLUP);
  coolDown_switch.attach(coolDownSwitchPin, INPUT_PULLUP);

  //  set switch debounce intervals
  door_switch.interval(100);
  light_switch.interval(100);
  coolDown_switch.interval(100);

  //  set switch pressed states
  door_switch.setPressedState(LOW);
  light_switch.setPressedState(LOW);
  coolDown_switch.setPressedState(LOW);

#endif