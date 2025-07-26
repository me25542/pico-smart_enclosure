# I2C command encoding

The encoding of commands to the enclosure into bytes sent via I2C is a bit more complicated in v2 than in v1. 
Instead of a rudimentary system where each possible byte is mapped to a command (e.g. `46` sets the target enclosure temp to 46 degrees), 
commands are encoded as a series of bytes. Each v2 command consists of at least 3 bytes: the command type, the number of bytes, and the data. 
The command structure is like this (first sent bytes on the left):

`command type`  |`number of data bytes` |`data bytes`
---             |---                    |---
^ first sent    |^ second sent          |^ last sent

The command type indicates *what* is being set. 
Currently, the only valid command types are `0xFF` to `0xF7` (`255` to `247`). 
These set different things:

Hex     |Dec    |Command type
---     |---    |---
`0xFF`  |`255`  |mode
`0xFE`  |`254`  |set temp
`0xFD`  |`253`  |print done
`0xFC`  |`252`  |max fan speed
`0xFB`  |`251`  |light state
`0xFA`  |`250`  |print name
`0xF9`  |`249`  |control mode
`0xF8`  |`248`  |heaters
`0xF7`  |`247`  |fan

The number of data bytes indicates how many data bytes are contained in the command
(e.g. if there are two data bytes this byte would be `0x02` (`2`), eighteen data bytes would make this `0x12` (`18`), etc.).

The data bytes contain the actual command data. 
What this data does / how it is encoded depends on the command type.

## Data byte actions by command type:

### Mode (`0xFF` hex, `255` dec):

Hex     |Dec    |Action
---     |---    |---
`0x00`  |`0`    |mode to error
`0x01`  |`1`    |mode to standby
`0x02`  |`2`    |mode to cooldown
`0x03`  |`3`    |mode to printing
(other) |(other)|ignored

<br>

### Set temp (`0xFE` hex, `254` dec):

Hex     |Dec    |Action
---     |---    |---
`0x00`  |`0`    |set temp to 0 (deg. c.)
`0x01`  |`1`    |set temp to 1
...     |...    |...
`0x4A`  |`74`   |set temp to 74
`0x4B`  |`75`   |set temp to 75
(other) |(other)|ignored

<br>

### Print done (`0xFD` hex, `253` dec):

Hex     |Dec    |Action
---     |---    |---
`0x00`  |`0`    |print not done
`0x01`  |`1`    |print done
(other) |(other)|ignored

<br>

### Max fan speed (`0xFC` hex, `252` dec):

Hex     |Dec    |Action
---     |---    |---
`0x00`  |`0`    |max fan speed to 0/255
`0x01`  |`1`    |max fan speed to 1/255
...     |...    |...
`0xFE`  |`254`  |max fan speed to 254/255
`0xFF`  |`255`  |max fan speed to 255/255

<br>

### Light state (`0xFB` hex, `251` dec):

Hex     |Dec    |Action
---     |---    |---
`0x00`  |`0`    |lights on
`0x01`  |`1`    |lights off
`0x02`  |`2`    |lights change
(other) |(other)|ignored

<br>

### Print name (`0xFA` hex, `250` dec):

This one is a bit different. Basically, each data byte represents one ASCII-encoded character in the print name. 
Each of these characters are added, one at a time, to the print name in place of the first `NULL`. 
The print name is full of `NULL`s when empty, so basically each character is added to the end of the part of the name that is already there.

#### Notes: 

The print name is cleared after each print automatically (when the mode returns to `standby`).
The print name has a 256-character limit, characters added after it is full will change the last character.

<br>

### Control mode (`0xF9` hex, `249` dec):

Hex     |Dec    |Action
---     |---    |---
`0x00`  |`0`    |undefined; do not use
`0x01`  |`1`    |control mode to temperature
`0x02`  |`2`    |control mode to manual
(other) |(other)|ignored

<br>

### Heaters (`0xF8` hex, `248` dec):

Hex     |Dec    |Action
---     |---    |---
`0x00`  |`0`    |heater off
`0x01`  |`1`    |heater on
(other) |(other)|ignored

#### Note:

Do not use unless the control mode is set to manual, otherwise behavior is undefined

<br>

### Fan (`0xF7` hex, `247` dec):

Hex     |Dec    |Action
---     |---    |---
`0x00`  |`0`    |fan to 0/255
`0x01`  |`1`    |ignored
`0x02`  |`2`    |fan to 2/255
`0x03`  |`3`    |ignored
...     |...    |...
`0xFC`  |`252`  |fan to 252/255
`0xFD`  |`253`  |ignored
`0xFE`  |`254`  |fan to 254/255
`0xFF`  |`255`  |ignored

#### Note:

If the least significant bit is `1`, the byte will be ignored

<br>

### Gcode examples:

#### Setting mode to `printing`:

<details>
<summary> expand </summary>

```
M260 A8 ; Address 8
M260 B255 ; Mode
M260 B1 ; 1 Byte
M260 B3 ; Printing
M260 S ; Send buffer
```

</details>

---

#### Setting a long name:

<details> 
<summary> expand </summary>

```
M260 A8 ; Address 8

; First I2C transaction (32 bytes total)
; Send the first part of the name (30 bytes)
M260 B250 ; Print name
M260 B30 ; 30 Bytes

M260 B84 ; 'T'
M260 B104 ; 'h'
M260 B105 ; 'i'
M260 B115 ; 's'
M260 B32 ; ' '
M260 B105 ; 'i'
M260 B115 ; 's'
M260 B32 ; ' '
M260 B97 ; 'a'
M260 B32 ; ' '
M260 B118 ; 'v'
M260 B101 ; 'e'
M260 B114 ; 'r'
M260 B121 ; 'y'
M260 B32 ; ' '
M260 B108 ; 'l'
M260 B111 ; 'o'
M260 B110 ; 'n'
M260 B103 ; 'g'
M260 B32 ; ' '
M260 B110 ; 'n'
M260 B97 ; 'a'
M260 B109 ; 'm'
M260 B101 ; 'e'
M260 B32 ; ' '
M260 B116 ; 't'
M260 B111 ; 'o'
M260 B32 ; ' '
M260 B116 ; 't'
M260 B101 ; 'e'

M260 S ; Send buffer

; Second I2C transaction (27 bytes total)
; Send the remaining part of the name (25 bytes)
M260 B250 ; Print name
M260 B26 ; 26 Bytes

M260 B115 ; 's'
M260 B116 ; 't'
M260 B32 ; ' '
M260 B115 ; 's'
M260 B116 ; 't'
M260 B117 ; 'u'
M260 B102 ; 'f'
M260 B102 ; 'f'
M260 B46 ; '.'
M260 B32 ; ' '
M260 B84 ; 'T'
M260 B101 ; 'e'
M260 B115 ; 's'
M260 B116 ; 't'
M260 B44 ; ','
M260 B32 ; ' '
M260 B116 ; 't'
M260 B101 ; 'e'
M260 B115 ; 's'
M260 B116 ; 't'
M260 B44 ; ','
M260 B32 ; ' '
M260 B116 ; 't'
M260 B101 ; 'e'
M260 B115 ; 's'
M260 B116 ; 't'

M260 S ; Send buffer
```

</details>

---

#### Manually turning heater on:

<details>
<summary> expand </summary>

```
M260 A8 ; Address 8
M260 B249 ; Control mode to manual
M260 B1 ; 1 Byte
M260 B248 ; Heater
M260 B1 ; Heater on
M260 S ; Send buffer
```

</details>

<br>

## Notes:

**On command length / why this encoding:**

Many commands only require one data byte, but some (like setting the print name) require more. 
Hence the byte indicating the number of data bytes.

---

**On addresses:**

The I2C address of the enclosure by default is `0x08` in hex, or `8` in dec. This can be changed by modifying a macro in the source code.

---

**On v1 compatibility:**

To maintain compatibility with v1, if the `command type` byte is in the range of `0x00` - `0x68` (`0` - `104`), it will be parsed as if it is a v1 command.
v1 commands were only one byte long, so if this is the case the next command is assumed to start with the next byte.
This means that valid v1 commands *are* still fully supported, and can be mixed with v2 commands in a given transmission.
However, it is recommended to use only v2 commands, as support for v1 commands is not guaranteed indefinitely.

---

**On maximum transmission length:**

The Marlin core used by the Prusa Buddy firmware has a limited I2C buffer size of 32 bytes; if you are sending many or long commands you may need to split them into multiple transmissions.
Additionally, the enclosure code has only a 32-byte primary buffer for incoming data, so longer transmissions wouldn't work. 
However, this buffer is flushed to a 256-byte circular buffer nearly immediately, so consecutive commands of up to 32 bytes should be fine. 
If you need more than 32 bytes, split it into multiple transmissions.

---

**If you still have questions, or this document is incomplete, please submit an issue.**