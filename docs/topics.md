# Rotator mqtt topics

## First the topics which are sent by rotator controller ( esp32 )
+ interpretations: BASETOPIC is for example ham, ID is a six character id generated from esp32 device id.

- [info about the system](#info)
- [statistics of the controller](#statistics)
- [my home location](#homepos)
- [known locations table ie. names](#names)
- [rotator specific settings](#settings)
- [rotation state](#state)
- [calibrations](#calibrations)
- [azimuth](#azimuth)

+ [topics from rotator to userinterface](#Next-the-topics-which-are-sent-to-userinterface).

- [Calibrate potentiometer digital value of 359 degrees](#calibrate-maximum-degrees)
- [Calibrate potentiometer digital value of 0 degrees](#calibrate-minimum-degrees)
- [Save calibrations to flash](#save-calibrations)
- [Set known calibrations](#set-known-calibrations)
- [Start turning clockwise](#start-turning-clockwise)
- [Stop turning, whatever direction](#stop-turning-whatever-direction)
- [Turn to a named direction](#turn-to-a-named-direction)
- [Turn to an azimuth](#turn-to-specific-azimuth)
- [Update or insert data in nametable](#update-or-insert-into-nametable)
- [Delete from nametable](#delete-from-nametable)
- [Send new settings to the controller](#send-new-settings-to-the-controller)

### info
BASETOPIC/rotator/ID/info
Topic is updated in boot, immediately after mqtt connection.
```json
{
    "dev": "5be9c0",
    "id": "info",
    "memfree": 202440,
    "idfversion": "v5.2.3",
    "progversion": "0.0.1.4"
}
```

## statistics 
BASETOPIC/rotator/ID/statistics
Topic is update after boot, and after an interval about 15 minutes.
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

### homepos 
BASETOPIC/rotator/ID/homepos
+ This should be stored in mosquitto topics with retention flag on. Default position is in
  javascript, pointing to Nastola municipal house. This message overrides the default position.
  At the moment there is no userinterface to update this topic. You can update topic with a
  mosquitto_pub command.
```json
{
    "dev": "5d11a8",
    "id": "homepos",
    "lat": 60.947784,
    "lng": 25.930342
}
```

### names 
- BASETOPIC/rotator/ID/setup/names
  Table is sent in boot and each time user changes this nametable.
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
### settings
- BASETOPIC/rotator/ID/settings
  This topic is update after user has update settings. This way every topic listener gets the new updated settings.

   + Inertiams is in milliseconds, how long does it take rotation to stop. Inertiadegr is how many degrees antennas turn
     after rotator power is off after rotation.
   + Beamwidth depends on antenna type. The more elements the sharper the beam.
     Beamwidth is used to certain optimizations, for example: if current azimuth is 200 degrees and user wants to turn
   to 202 degrees and this new position is inside beamwidth, rotator does nothing.
   + Turnspeed is average speed rotator turns in 10 seconds. This value is used to interpret speedstate in azimuth message.
   + Hardstop is the azimuth where your antenna is pointing when rotator is in hardstop. I have realized that hardstop cannot
     be in north, because I use my antenna for aurora qso:s, so antenna is pointing a little northwest or northeast.
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
### state
- BASETOPIC/rotator/ID/state
  State is updated everytime rotator motor is starting or stopping.
  state can be still, cw, ccw
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
### calibrations
- BASETOPIC/rotator/ID/calibrations
  Topic is updated in boot and if calibrations have changed.
```json
{
    "dev": "5d11a8",
    "id": "calibrations",
    "min": 37,
    "max": 3019,
    "ts": 1794922410
}
```
### azimuth
- BASETOPIC/rotator/ID/parameters/azimuth
  Controller sends this message each time the azimuth has changed at least one degree, while turning.
  When still, azimuth is sent after 4 degree change. This way the userinterface can show the live
  azimuth, while turning. If azimuth is close to a named degrees, name is shown as well. Speed is
  the turning speed degrees per 10sec. Speedstate can be high, low, normal or none.
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
## Next the topics which are sent to userinterface
   Userinterfaces in this project are rotatormap.html ( map webgui ) and rotator.html (setup webgui).
   All the messages from userinterface are the "set" topic. What is actually done
   is decided by the json "id" field.

- home/kallio/rotator/5be9c0/set

### calibrate minimum degrees
+ This has only the digital value of fully counterclockwise potentiometer reading.
  Message should be executed as follows. Turn rotator fully counterclockwise to hardstop.
  Now push Cal min button in setup ui. Note that this message has no data, it only asks
  the rotator store current potentiometer to calibration variable.
```json
{
  "dev": "5be9c0",
  "id": "calmin"
}
```

### calibrate maximum degrees
+ This has only the digital value of fully clockwise potentiometer reading. This message
  is only a command, it does not have any data.
  Message should be executed as follows. Turn rotator fully clockwise to hardstop.
  Now push Cal max button in setup ui. After min and max calibrations the rotator
  has calibrated potentiometer values in its memory.
```json
{
  "dev": "5be9c0",
  "id": "calmax"
}
```

### save calibrations
+ Calibration values are saved from ram memory to controllers flash memory.
```json
{
  "dev": "5be9c0",
  "id": "savecalibr"
}
```

### set known calibrations
+ If you somehow get lost with the calibrations, you can restore them by sending a
  calibrations message. If you happen to have stored them from BASETOPIC/rotator/ID/calibrations message.
  This might be useful, if the rotator needs different calibrations for different ambient temperatures
  We have to understand, that the measuring potentiometer is in volatile environment. Outside temperature
  hera in Finland may wary from -30 to +35 degrees Celcius, depending of the time of year.
  After this message you need to save the calibrations, if you want them to be in effect after next boot.
```json
{
  "id": "calibrations",
  "dev": "5be9c0",
  "min": 146,
  "max": 3793
}
```

### start turning clockwise
```json
{
  "dev": "5be9c0",
  "id": "turn",
  "direction": "cw"
}
```

### start turning counterclockwise
```json
{
  "dev": "5be9c0",
  "id": "turn",
  "direction": "cw"
}
```

### stop turning whatever direction
```json
{
  "dev": "5be9c0",
  "id": "turn",
  "direction": "still"
}
```

### turn to a named direction
+ Turning stops automatically, when the
  target direction is reached.
```json
{
  "dev": "5be9c0",
  "id": "namedtarget",
  "name": "Lahti"
}
```

### turn to specific azimuth
+ Turning stops, when target is reached.
```json
{
  "dev": "5be9c0",
  "id": "newtarget",
  "degr": 197
}
```

### update or insert into nametable
+ If both name and direction are missing from the table,
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

### delete from nametable
+ Key is the name. Delete is not possible to do with azimuth.
```json
{
  "dev": "5be9c0",
  "id": "delname",
  "name": "Kotka"
}
```

### send new settings to the controller
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

