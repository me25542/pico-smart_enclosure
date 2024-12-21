## v2 example custom gcode:

(compatible with any PrusaSlicer-based slicer)
- Prusa Slicer
- Bambu Studio
- Orca Slicer
- Super Slicer

**This example gcode is based off of commands compatible with v2.0.0_rc1 (internal, prerelease)**

List of compatible versions:
- v2.0.0_rc1 (internal, prerelease)
- v2.x

It should also be compatible with any v2.x versions in the future


### Start gcode:

(add at the *very* start of your printer's start gcode)

<details>

<summary> expand </summary>

```
; start of my custom stuff:
M260 A8 ; set address to 8
M260 B255 ; setting: mode
M260 B1 ; 1 byte
M260 B3 ; mode: printing
M260 B253 ; setting: print done
M260 B1 ; 1 byte
M260 B0 ; print done: false
{if filament_type[initial_tool]=="PLA"}
M260 B254 ; setting: enclosure temp
M260 B1 ; 1 byte
M260 B15 ; enclosure temp: 15deg
M260 S ; send the current buffer
{elsif filament_type[initial_tool]=="PETG"}
M260 B254 ; setting: enclosure temp
M260 B1 ; 1 byte
M260 B30 ; enclosure temp: 30 deg C
M260 B252 ; setting: max fan speed
M260 B1 ; 1 byte
M260 B127 ; max fan speed: 50%
M260 S ; send the current buffer
{elsif filament_type[initial_tool]=="ASA"||filament_type[initial_tool]=="ABS"}
M260 B254 ; setting: enclosure temp
M260 B1 ; 1 byte
M260 B50 ; enclosure temp: 50 deg C
M260 B252 ; setting: max fan speed
M260 B1 ; 1 byte
M260 B0 ; max fan speed: 0%
M260 S ; send the current buffer
{elsif filament_type[initial_tool]=="FLEX"}
M260 B254 ; setting: enclosure temp
M260 B1 ; 1 byte
M260 B25 ; enclosure temp: 25 deg C
M260 S ; send the current buffer
{elsif filament_type[initial_tool]=="PC"}
M260 B254 ; setting: enclosure temp
M260 B1 ; 1 byte
M260 B50 ; enclosure temp: 50 deg C
M260 B252 ; setting: max fan speed
M260 B1 ; 1 byte
M260 B0 ; max fan speed: 0%
M260 S ; send the current buffer
{elsif filament_type[initial_tool]=="PA"||filament_type[initial_tool]=="NYLON"}
M260 B254 ; setting: enclosure temp
M260 B1 ; 1 byte
M260 B50 ; enclosure temp: 50 deg C
M260 B252 ; setting: max fan speed
M260 B1 ; 1 byte
M260 B0 ; max fan speed: 0%
M260 S ; send the current buffer
{endif}
; end of my custom stuff
```

</details>

---

### End gcode

(add at the *very* end of your printer's end gcode)

<details>

<summary> expand </summary>

```
; start of my custom stuff:
M260 A8 ; set address to 8
M260 B254 ; set temp
M260 B1 ; 1 byte
M260 B20 ; 20 deg. c
M260 B255 ; mode
M260 B1 ; 1 byte
M260 B2 ; cooldown
M260 B253 ; print done
M260 B1 ; 1 byte
M260 B1 ; set print done
M260 S ; send the current buffer
; end of my custom stuff
```

</details>

---

### See also:

[RepRap - M260](https://reprap.org/wiki/G-code#M260:_i2c_Send_Data)

[Macros | Prusa knowledge base](https://help.prusa3d.com/article/macros_1775)

Make sure to read the documentation on command encoding.