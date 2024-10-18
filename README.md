# pico-smart_enclosure
### Automatic active temperature control and lighting for a 3d printer enclosure.
Uses a raspberry Pi Pico connected to the printer via I2C to provide **automatic** active temperature control and full control via gcode.


## Features
- **Automatic** active temperature control
- **Direct** communication with the printer (via I2C)
- **Smooth** lighting control
- Fan kickstarting
- Motorized fan flap control (optional)
- Print done notification (light turns on)
- Use of the standby capabilities on ATX PSUs to save power
- Uses both cores in the pico for smoother real-time operation
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
        - Sends data about the error via USB every 30 seconds, indefinitly


## To use
- Install [this](https://github.com/earlephilhower/arduino-pico) board in Arduino IDE
    - Open Arduino IDE
    - Go to file - preferences (or press CTRL+Comma)
    - In the "Additional boards manager URLs" field paste the above URL (https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json)
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
![Enclosure_wiring](https://github.com/user-attachments/assets/642e8f6c-49c5-4556-9cb7-0a7d82cefa94)

## Compatibility
This design is compatible with any printer that supports M260 gcode commands and has an I2C bus that you can connect to.

The only printers that I am *certain* will work with this are the Original Prusa MK4-series printers (I made this for my MK4), as their Xbuddy board has an I2C connector that is unused. (a female 4-pin Molex Clik-Mate if you want to buy a cable to fit)

However, many other printers *might* have I2C connectors, so it's worth checking yours. If you find other printers that are compatible, please let me know!

A raspberry Pi might also work (they have I2C) if you are using one for Octoprint or Klipper already, with a bit of modification.


