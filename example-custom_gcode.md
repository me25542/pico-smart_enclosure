## example custom gcode:

(compatible with any PrusaSlicer-based slicer)
- Prusa Slicer
- Bambu Studio
- Orca Slicer
- Super Slicer

This start gcode is based off of commands compatible with v1.1.0

It should be compatible with any v1.x version later than this

### Start gcode:

(add at the *verry* start of your printer's start gcode)

```
; start of my custom stuff:

M260 A8 ; set adress to 8

M260 B3 ; set enclosure mode to printing

M260 B5 ; set print not done

{if filament_type[initial_tool]=="PLA"}

M260 B15 ; set target enclosure temp to 15deg C

{elsif filament_type[initial_tool]=="PETG"}

M260 B30 ; set target enclosure temp to 30deg C

M260 B102 ; set max enclosure fan speed to 50%

{elsif filament_type[initial_tool]=="ASA"||filament_type[initial_tool]=="ABS"}

M260 B50 ; set target enclosure temp to 50deg C

M260 B100 ; set max enclosure fan speed to 0%

{elsif filament_type[initial_tool]=="FLEX"}

M260 B25 ; set target enclosure temp to 25deg C

{elsif filament_type[initial_tool]=="PC"}

M260 B50 ; set target enclosure temp to 50deg C

M260 B100 ; set max enclosure fan speed to 0%

{elsif filament_type[initial_tool]=="PA"||filament_type[initial_tool]=="NYLON"}

M260 B50 ; set target enclosure temp to 50deg C

M260 B100 ; set max enclosure fan speed to 0%

{endif}

M260 S ; send the current buffer

; end of my custon stuff
```


### End gcode

(add at the *verry* end of your printer's nd gcode)

```
; start of my custom stuff:

M260 A8 ; set adress to 8

M260 B20 ; set target enclosure temp to 20deg C

M260 B2 ; set enclosure mode to cooldown

M260 B4 ; set print done

M260 S ; send the current buffer

; end of my custom stuff
```

### See also:

[RepRap - M260](https://reprap.org/wiki/G-code#M260:_i2c_Send_Data)

[Macros | Prusa knowledge base](https://help.prusa3d.com/article/macros_1775)
