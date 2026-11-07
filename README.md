# rotator
esp32 antenna rotator control

Connections are:

1. Rotator azimuth potentiometer, 0 - 1000ohm
2. Two relays, first puts the 230v power to the transformer, second selects rotation direction
3. Direction buttons, clockwise or counterclockwise
4. tiny 1.3 inch display to show some info.

Features:
1. Some pre-defined directions can be stored into esp32 flash memory, these directions are shown
   in the display, if antenna is turned close enough to these directions.
2. If connected to a mqtt server via wifi, all control can be done with a web interface.
3. Web interface allows rotator settings update and simple control.
4. Better ui can be done with a openStreetMap interface. This interface shows antenna direction
   like a beam over a map. Turn direction can be done by clicking the map, or by html buttons.



