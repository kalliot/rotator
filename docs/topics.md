# Rotator mqtt topics

## First the topics which are sent by rotator controller ( esp32 )
interpretations: BASETOPIC is for example ham, ID is a six character id generated from esp32 device id.


###info
BASETOPIC/rotator/ID/info
```json
{
    "dev": "5be9c0",
    "id": "info",
    "memfree": 202440,
    "idfversion": "v5.2.3",
    "progversion": "0.0.1.4"
}
```

##statistics 
BASETOPIC/rotator/ID/statistics
```json
{
    "dev": "5be9c0",
    "id": "statistics",
    "connectcnt": 1,
    "disconnectcnt": 1,
    "sendcnt": 3,
    "sensorerrors": 0,
    "max_queued": 1,
    "ts": 1795082926,
    "started": 1795082921,
    "rssi": -78
}
```

###homepos 
BASETOPIC/rotator/ID/homepos
+ This should be stored in mosquitto topics with retention flag on. Default position is stored in
  javascript, pointing to Nastola municipal house.
```json
{
    "dev": "5d11a8",
    "id": "homepos",
    "lat": 60.947784,
    "lng": 25.930342
}
```

- BASETOPIC/rotator/ID/setup/names
```json
{
    "dev": "5d11a8",
    "id": "nametable",
    "maxnames": 20,
    "ts": 1794922410,
    "names": [
        {
        "name": "Kotka",
        "azimuth": 134
        },
        {
        "name": "Inkoo",
        "azimuth": 228
        },
        {
        "name": "Vantaa",
        "azimuth": 216
        },
        {
        "name": "Lahti",
        "azimuth": 287
        },
        {
        "name": "Kouvola",
        "azimuth": 100
        },
        {
        "name": "Tallinna",
        "azimuth": 202
        },
        {
        "name": "Joensuu",
        "azimuth": 45
        },
        {
        "name": "Jyvaskyla",
        "azimuth": 355
        },
        {
        "name": "Tampere",
        "azimuth": 301
        }
    ]
}
```

- BASETOPIC/rotator/ID/settings
```json
{
    "dev": "5d11a8",
    "id": "settings",
    "hardstop": 78,
    "ts": 1794922410,
    "inertiams": 1000,
    "inertiadegr": 3,
    "beamwidth": 10,
    "turnspeed": 45
}
```

- BASETOPIC/rotator/ID/state
```json
{
    "dev": "5d11a8",
    "id": "movestate",
    "state": "still",
    "reason": "target",
    "target": -1,
    "ts": 1794995422
}
```

- BASETOPIC/rotator/ID/calibrations
```json
{
    "dev": "5d11a8",
    "id": "calibrations",
    "min": 37,
    "max": 3019,
    "ts": 1794922410
}
```

- BASETOPIC/rotator/ID/parameters/azimuth
```json
{
    "dev": "5d11a8",
    "id": "azimuth",
    "value": 101,
    "name": "Kouvola",
    "speed": "20",
    "speedstate": "none",
    "ts": 1794995428
}
```
## Next the topics which are sent by userinterface. 
   Userinterfaces in this project are  rotatormap.html ( map webgui ) and rotator.html (setup webgui).
   All the messages from userinterface are the "set" topic. What is actually done
   is decided by the json "id" field.

- home/kallio/rotator/5be9c0/set
+ calibrate minimum degrees. This has only the digital values of fully counterclockwise.
  Next message should be executed as follows. Turn rotator fully counterclockwise to hardstop.
  Now push Cal min button in setup ui.
```json
{
  "dev": "5be9c0",
  "id": "calmin"
}
```

+ calibrate maximum degrees. This has only the digital values of fully clockwise.
  Next message should be executed as follows. Turn rotator fully clockwise to hardstop.
  Now push Cal max button in setup ui. After min and max calibrations the rotator
  has calibrated potentiometer values in its memory.
```json
{
  "dev": "5be9c0",
  "id": "calmax"
}
```

+ save calibrations
```json
{
  "dev": "5be9c0",
  "id": "savecalibr"
}
```

+ If you somehow get lost with the calibrations, you can restore them by sending a
  calibrations message. If you happen to have stored them from BASETOPIC/rotator/ID/calibrations message.
```json
{
  "id": "calibrations",
  "dev": "5be9c0",
  "min": 146,
  "max": 3793
}
```

+ Start turning clockwise
```json
{
  "dev": "5be9c0",
  "id": "turn",
  "direction": "cw"
}
```

+ Start turning counterclockwise
```json
{
  "dev": "5be9c0",
  "id": "turn",
  "direction": "cw"
}
```

+ Stop turning, whatever direction.
```json
{
  "dev": "5be9c0",
  "id": "turn",
  "direction": "still"
}
```

+ Turn to a named direction. Turning stops automatically, when the
  target direction is reached.
```json
{
  "dev": "5be9c0",
  "id": "namedtarget",
  "name": "Lahti"
}
```

+ Turn to specific azimuth. Turning stops, when target is reached.
```json
{
  "dev": "5be9c0",
  "id": "newtarget",
  "degr": 197
}
```

+ Update/Insert nametable. If both name and direction are missing from the table,
  a new record is inserted. If name is known, but azimuth does not match, azimuth is
  updated. If azimuth is found, update the name.
```json
{
  "dev": "5be9c0",
  "id": "updatenames",
  "name": "Tallinna",
  "azimuth": 202
}
```

+ Delete from nametable. Key is the name. Delete is not possible to do with azimuth.
```json
{
  "dev": "5be9c0",
  "id": "delname",
  "name": "Kotka"
}
```

+ Send new settings to the controller.
```json
{
  "dev": "5be9c0",
  "id": "setsettings",
  "hardstop": 78,
  "inertiams": 1500,
  "inertiadegr": 1,
  "turnspeed": 45,
  "beamwidth": 10
}
```

