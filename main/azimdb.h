
#ifndef __AZIMDB__
#define __AZIMDB__

#include <time.h>
#include "mqtt_client.h"

struct directionNames
{
    char name[12];
    int azimuth;
};


extern struct directionNames interestingDirections[];


extern bool azimdb_init(uint8_t *chip);
extern void azimdb_show(void);
extern bool azimdb_delname(char *name);
extern char *azimdb_find_nearest(int azimuth, int beamwidth);
extern struct directionNames *azimdb_find_named(char *name);
extern bool azimdb_update(char *name, int azimuth);
extern void azimdb_publish(esp_mqtt_client_handle_t client, char *prefix, char *appname, time_t now);


#endif