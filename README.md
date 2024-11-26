# pico-smart_enclosure
### Automatic active temperature control and lighting for a 3d printer enclosure.
Uses a raspberry Pi Pico connected to the printer via I2C to provide **automatic** active temperature control and full control of other enclosure functions via gcode (M260).

### I am working on v2, this branch is a work in process intended to help me find bugs faster. Some documentation info may be outdated (I was to lazy to change it), and the code **will** have bugs.

## Features
- **Automatic** active temperature control
- **Direct** communication with the printer (via I2C)
- **Smooth** PWM lighting control (currently broken in a few ways, should be fixed by the stable release)
- **UI via OLed screen**
  - You can control pretty much everything using the menu
  - **Known issues**: The menu isn't verry responsive when in printing mode. Might get fixed (if I can workaround the root cause).
- Fan kickstarting
- PWM fan speed control
- Printer-controllable max fan speed
- Motorized fan flap control (optional)
- Print done notification (light turns on)
- Use of the standby capabilities on ATX PSUs to save power
- Uses both cores in the Pico for smoother real-time operation
- Highly customizable
- Prints lots of debugging data over USB
- Control via:
    - M260 gcode commands (printer) - full control
    - USB connected computer - full control
    - Two buttons - changing light state and setting mode to cooldown only
- **Safety** features:
    - Fans and heaters turn off when door open
    - Sensor connection status is regularly checked
    - Sensor data is checked for plausibility (is it way to low, etc.)
    - Temperature exceeding set thresholds will cause shutdown (see below)
    - Will enter a safe "error" state if anything is wrong (invalid printer command, sensor disconnected, temp exceeds set bounds, etc.)
        - Turns off heaters
        - Turns off fans
        - Turns off lights
        - Turns off the power supply
        - Stops communication with printer
        - Sends data about the error via USB every 30 seconds, indefinitely


## To use
- **If you don't need to change anything in the code (if your hardware is the same as mine):**
	- Double check your wiring (couldn't hurt, for what mine looks like see the wiring diagram below)
	- Plug the Raspberry Pi Pico into your computer while holding down the BOOTSEL button, copy [the latest .uf2 file](https://github.com/me25542/pico-smart_enclosure/releases) to the pico
	- Enjoy!
- **If you want to customize things in the code:**
	- I recommend Using Arduino IDE, as that is what I used and I know it works
	- Install [this](https://github.com/earlephilhower/arduino-pico) board in Arduino IDE
	    - Open Arduino IDE
	    - Go to file - preferences (or press CTRL+Comma)
	    - In the "Additional boards manager URLs" field paste the above URL (`https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json`)
	    - Hit "OK" to close the dialog
	    - Search something like "pico" in the boards manager, and install "Raspberry Pi Pico/RP2040/RP2035"
	- Install all dependencies (see below)
	- Download the files in "Printer_enclosure_firmware" and open them in Arduino IDE
	- Change any values as needed, depending on your setup
	- Double check your wiring (couldn't hurt, for what mine looks like see the wiring diagram below)
	- Upload the code to your Pico
	- Enjoy!

**This program has several dependencies you will need to install:**
  - [Adafruit MCP9808](https://github.com/adafruit/Adafruit_MCP9808_Library)
  - [Servo](https://github.com/arduino-libraries/Servo)
  - [Bounce2](https://github.com/thomasfredericks/Bounce2)
  - Some of these have dependencies of their own, just select "install all" when prompted in arduino IDE

### Wiring diagram:
![image](https://github.com/user-attachments/assets/5ea24fb2-6b94-49b1-925b-52193d9423ab)

## BOM

(needs updating)

(In no particular order):
- 1x 3d printer enclosure
- 1x ATX power supply (anything over ~250w will work)
- 1x Raspberry Pi Pico
- 1x heater
	- 2x 50w 3Ω power resistors
	- 1x old computer heatsink w. 12v fan
- 1x 5v relay module (at least one channel)
- 2x generic logic level converters (the "bad" kind)
- 2x 5v 9g servos
- 3x MCP9808 sensors
- 3x micro switches
- 4x BC337 transistors
- 4x 1kΩ resistors
- 4x 220Ω
- 4x white 5mm LEDs
- Some 12v white LED strips
- Some sort of connector to plug into your printer's control board (Molex Click-Mate for a MK4)
- Some printed parts ([this](https://www.printables.com/@_me_/collections/1713789) might be a good starting place)
- Probably some other little things that I forgot. Use common sense.

## Compatibility
**This design is compatible with any printer that supports [M260](https://reprap.org/wiki/G-code#M260:_i2c_Send_Data) gcode commands and has an I2C bus that you can connect to.**

The only printers that I am *certain* will work with this are the Original Prusa MK4-series printers (I made this for my MK4), as their Xbuddy board has an I2C connector that is unused. (a female 4-pin Molex Click-Mate if you want to buy a cable to fit)

**Known compatible printers:**
- Prusa MK4(s)
- Prusa MK3.9(s)
- Prusa MK3.5(s)

However, many other printers *might* have I2C connectors, so it's worth checking yours. If you find other printers that are compatible, please let me know!

A raspberry Pi might also work (they have I2C) if you are using one for Octoprint or Klipper already, with a bit of modification.

## Contribution
If you want to contribute in any way, feel free to do so!

Whether it's reporting bugs or writing code, your contributions will be helpful.

### Ideas for features to add
- Oled display + UI
    - Data display via a small I2C-connected screen
    - Navigation with buttons
- Thermal runaway protection
    - protect heaters from thermal runaway
- Smooth control of the print done light
    - Turn the print done light on and off smoothly
- Audible notification for print done
    - Play a small chime or sound when the print is set to done
- Support for basic control through printer GPIO pins (for printers without I2C)
    - Have a small number of pins be able to, say, set the temperature to one of two states
    - Cut out any unneeded functionality (like setting the lights, fine temperature control, etc.)
- Fixing my misuse of .h files
	- I, uh, might have misused the .h file format a bit
	- If you know how to fix this, please do! 
- Anything else you can think of!
    - If you can, implement it!
    - If you can't, raise an issue requesting the feature

Thank you for any and all contributions!

## Received I2C values, and what they do:

(*should* stil work, but dosen't seem to. Outdated; v2 uses a beter primary encoding scheem)

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

**Values 105-255 are reserved; as such they set the mode to error** (No longer; disregard)

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

If the max fan speed is not specified it will be set to 100%

### WARNING: values 9 and 105-255 set the mode to error (as they are reserved for future use), putting the enclosure into safe mode (No longer; disregard)
