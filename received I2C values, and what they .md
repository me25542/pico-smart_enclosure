## Received I2C values, and what they do:

**Values 0-3 set the enclosure mode:**
- 0 = error (shuts everything down and enters a safe state)
- 1 = standby (doesn't really do anything, turns off the PSU to save power when the lights are off)
- 2 = cooldown (turns on the vent fan until the enclosure temp is close enough to the outside temp)
- 3 = printing (maintains the target temperature by heating or cooling as is required)

**Values 4-9 set other things:**
- 4 = print done
- 5 = print not done
- 6 = turns enclosure lights on
- 7 = turn enclosure lights off
- 8 = changes enclosure lights state (from off to on and vice versa)
- 9 = reserved for future use (sets mode to error)
	
**Values 10-99 set target temp:**
- 10 = target temp of 10 °C
- 11 = target temp of 11 °C
- ...
- 98 = target temp of 98 °C
- 99 = target temp of 99 °C

**Values 100-255 are reserved; as such they set the mode to error**

### Notes:

If the enclosure lights state are set, and are changed, in the same transmission to the enclosure:
- Except in very rare cases, the lights will be set to the specified value and *then* changed.
- However, this is not guaranteed, and it is not recommended to set the lights *and* change them in the same transmission

If the lights are set to one value, and then to another, in the same transmission to the enclosure:
- The last sent value will be used

If the set temperature is set to one value (e.g 10 °C), and then to another (e.g. 40 °C), in the same transmission to the enclosure:
- The last sent value will be used

If the print is set to one value (e.g. done) and then to another (e.g. not done) in the same transmission to the enclosure:
- The last sent value will be used

If the mode is set to one value (e.g. printing) and then to another (e.g. standby) in the same transmission to the enclosure:
- The last sent value will be used

If the set temperature value is not specified at the start of a print, the last received set temperature value is used

### WARNING: values 9 and 100-255 set the mode to error (as they are reserved for feature use), putting the enclosure into safe mode
