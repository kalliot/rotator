#ifndef __TURN_ROTATOR__
#define __TURN_ROTATOR__

#include "homeapp.h"

struct rotatorSettings
{
    int hardStop;
    int inertiaMs;
    int inertiaDegr;
    int beamWidth;
};

extern void rotator_setsettings(struct rotatorSettings *newsetup);
extern struct rotatorSettings *rotator_getsettings();
extern void rotator_show_settings(void);
extern bool rotator_publish_event(char *prefix, struct measurement *meas, esp_mqtt_client_handle_t client);
extern bool rotator_turn2target(int target);
extern bool rotator_turn(enum directionStates direction);
extern bool rotator_turn_bystr(char *dirStr);
extern bool rotator_turned(int azim);
extern void rotator_init(uint8_t *chip);


#endif