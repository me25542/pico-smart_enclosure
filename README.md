# pico-smart_enclosure
### Automatic active temperature control and lighting for a 3d printer enclosure.
Uses a raspberry Pi Pico connected to the printer via I2C to provide **automatic** active temperature control and full controll via gcode.


**Features:**
- **Automatic** active temperature control
- **Direct** communication with the printer (via I2C)
- **Smooth** lighting control
- Fan kickstarting
- Motorized fan flap control (optional)
- Use of the standby capabilities on ATX PSUs to save power
- Control via:
    - M260 gcode commands (printer) - full control
    - USB connected computer - full control
    - Two buttons - changing light state and seting mode to cooldown only
- **Safety** features:
    - Fans and heaters turn off when door open
    - Sensor connection status is regularly checked
    - Sensor data is checked for plausability (is it way to low, etc.)
    - Tempurature exceeding set threashholds will cause shutdown (see below)
    - Will enter a safe "error" state if anything is wrong (invlid printer command, sensor diconnected, temp exceeds set bounds, etc.)


### To use:

Download the files in "Printer_enclosure_firmware" and open them in arduin IDE

**This program has serveral dependencies you will need to install:**
  - [Adafruit MCP9808](https://github.com/adafruit/Adafruit_MCP9808_Library)
  - [Servo](https://github.com/arduino-libraries/Servo)
  - [Bounce2](https://github.com/thomasfredericks/Bounce2)
