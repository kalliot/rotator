#ifndef __HOMEAPP__
#define __HOMEAPP__

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "flashmem.h"


#define DEBUG_TO_MQTT 1

enum meastype
{
    AZIMUTH,
    ROTATORSTATE,
    ADB_REFRESHED,
    PUBLISH_ROTATOR_SETTINGS,
    REFRESHPOS,
    STATE,
    OTA
};

enum directionStates
{
    STATE_STILL,
    STATE_CW,
    STATE_CCW,
    STATE_NONE
};


struct measurement {
    enum meastype id;
    int gpio;
    int err;

    union {
        int count;
        int state;
        enum directionStates rotatorstate;
        int azimuth;
    } data;
};

extern QueueHandle_t evt_queue;
extern char jsondata[];
extern nvs_handle setup_flash;

#define BLINK_GPIO      23
#define CWBUTTON_GPIO   CONFIG_CWBUTTON_GPIO
#define CCWBUTTON_GPIO  CONFIG_CCWBUTTON_GPIO
#define CWRELAY_GPIO    CONFIG_CWRELAY_GPIO
#define CCWRELAY_GPIO   CONFIG_CCWRELAY_GPIO
#define MIN_EPOCH   1650000000

#endif