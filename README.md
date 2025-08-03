# pico-smart_enclosure
### Automatic active temperature control and lighting for a 3d printer enclosure.
Uses a raspberry Pi Pico connected to the printer via I2C to provide **automatic** active temperature control and full control of other enclosure functions via gcode (M260). 
Also gives full control via an OLed screen UI.

## Features
- **Automatic active temperature control**
- **Direct communication with the printer** (via I2C)
- **Smooth PWM lighting control**
- **Power loss recovery** (will save setting changes and state on power loss)
- **UI via OLed screen**
- Print name display while printing
- Fan kickstarting
- PWM fan speed control
- Printer (and user) controllable max fan speed
- Motorized fan flap control
- Print done notification (indicator light turns on)
- Uses the standby capabilities of ATX PSUs to save power
- Uses both cores in the pico for smoother real-time operation
- Offers both temperature and manual fan / heater control to printer (only via I2C for now)
- Automatically detects button / switch type used
- Highly customizable
- Other small things, like the light turns on when the door opens
- Control via:
    - UI - nearly full control, plus settings
    - M260 gcode commands (printer) - full control
    - USB connected computer - partial control (basics)
    - Two quick-acess buttons
        - changing light state
        - set mode to cooldown
- **Safety features**:
    - Fans and heaters turn off when door open
    - Sensor connection status is regularly checked
    - Sensor data is checked for plausibility (is it way to low, etc.)
    - Temperature exceeding set thresholds will cause shutdown (see below)
    - Will enter a safe "error" state if anything goes wrong (wiring looks incorrect [^1], sensor disconnected, processor core unresponsive, temperature exceeds set bounds, etc.)
        - Turns everything off
        - Stops communication with printer
        - Every 10 seconds, indefinitely:
            - Sends data about the error via USB
            - Blinks error code on the Pico's on-board LED

[^1]: *Cannot detect all (or even most) wiring mistakes, use caution*

## Usage
- **If you don't need to change anything in the code (if your hardware is the same as mine):**
    - Double check your wiring
        - Couldn't hurt, for what mine looks like see the wiring diagram below
        - Your switches can be either `NO` or `NC`, the code will detect this
    - **Ensure that whenever you upload a new version, the door is open (no switches are pressed)**
    - Plug the Raspberry Pi Pico into your computer while holding down the BOOTSEL button, copy [the latest `.uf2` file](https://github.com/me25542/pico-smart_enclosure/releases/latest) to the pico
    - Enjoy!
- **If you want to customize things in the code (building manually):**
    - The only officially supported option is [platformIO](https://platformio.org).
        - However, due to the core used, you will probably be able to rearrange things to work with Arduino IDE. This is **not** officially supported, and could become impossible in the future.
    - This project uses the excellent [Arduino-pico core](https://github.com/earlephilhower/arduino-pico) by [Earle F. Philhower](https://github.com/earlephilhower) and other contributors (released under the GNU Lesser General Public License v2.1)
        - Follow the [instructions](https://github.com/earlephilhower/arduino-pico/blob/master/docs/platformio.rst) found in it's repository to set up PlatformIO with it
    - Install all dependencies (see documentation)
    - Clone this repository and open it with the editor you installed PlatformIO in
    - Change any code as needed, depending on your setup
        - `\include\config.hpp` Is a good start for simple config changes
    - Double check your wiring (couldn't hurt, for what mine looks like see the wiring diagram below)
    - Upload the code to your Pico (see above)
    - Enjoy!

- **For wiring diagram, BOM, etc., see the documentation**

## Printer compatibility
**This design is compatible with any printer that supports [M260](https://reprap.org/wiki/G-code#M260:_i2c_Send_Data) gcode commands and has an open I2C bus that you can connect to.**

The only printers that I am *certain* will work with this are the Original Prusa MK4-series printers (I made this for my MK4), as their Xbuddy board has an I2C connector that is unused. (a female 4-pin Molex Click-Mate if you want to buy a cable to fit)

### Known compatible printers:
- Prusa CORE One
- Prusa MK4(s)
- Prusa MK3.9(s)
- Prusa MK3.5(s)

However, many other printers may have I2C connectors, so it's worth checking yours. If you find other printers that are compatible, please let me know!

A raspberry Pi might also work (they have I2C) if you are using one for Octoprint or Klipper already, with a bit of modification.

## Contribution
If you want to contribute in any way, feel free to do so!

Whether it's reporting bugs or writing code, your contributions will be helpful.
- *For code contributions, see `docs\CONTRIBUTING.md`*

### Ideas for features to add
- Thermal runaway protection
    - protect heaters from thermal runaway
- Audible notification for print done
    - Play a small chime or sound when the print is set to done
- Support for basic control through printer GPIO pins (for printers without I2C)
    - Have a small number of pins be able to, say, set the temperature to one of two states
    - Cut out any unneeded functionality (like setting the lights, fine temperature control, etc.)
- Air filter support
- Anything else you can think of!
    - If you can, implement it!
    - If you can't, raise an issue requesting the feature

Thank you for any and all contributions!

<br>

## About:
I couldn't find code for a smart printer enclosure that had all the features I wanted.
Mainly, I wanted something that was automatic; I didn't want to have to set fans, heaters, etc. *every single print*.
So, I spent a lot of time making this.
I believe it is the most fully featured code base for a 3d printer enclosure you could find, mainly because I couldn't find a more fully featured one.

