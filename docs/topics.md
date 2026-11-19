# Rotator topics

interpretations: BASETOPIC is for example ham, ID is a six character id generated from esp32 device id.

- BASETOPIC/rotator/ID/homepos
```json
    {
        "dev": "5d11a8",
        "id": "homepos",
        "lat": 60.947784,
        "lng": 25.930342
    }

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

- BASETOPIC/rotator/ID/calibrations
```json
{
  "dev": "5d11a8",
  "id": "calibrations",
  "min": 37,
  "max": 3019,
  "ts": 1794922410
}

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
