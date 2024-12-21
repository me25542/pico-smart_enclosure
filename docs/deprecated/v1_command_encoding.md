# v1 commands are deprecated, use v2 instead

**This is only included for archival reasons**

---

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

**Values 100-104 set max fan speed:**
- 100 = max fan speed of 0% (vents will still be opened)
- 101 = max fan speed of 25%
- 102 = max fan speed of 50%
- 103 = max fan speed of 75%
- 104 = max fan speed of 100%