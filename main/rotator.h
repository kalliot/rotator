#ifndef __TURN_ROTATOR__
#define __TURN_ROTATOR__


extern void rotator_turn2target(int target);
extern bool rotator_turn(char *direction);
extern bool rotator_turned(int azim);
extern void rotator_publish_state(char *prefix, struct measurement *data, esp_mqtt_client_handle_t client);
extern void rotator_init(uint8_t *chip);


#endif