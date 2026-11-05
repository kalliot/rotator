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
    PUBLISH_POT_CALIBR,
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

enum reasons {
    REASON_TIMEOUT,  // turning too slowly, timeout
    REASON_BUTTON,   // user pressed a physical button
    REASON_DIRCHANGE,// normal stop before changing the turn direction
    REASON_MQTT,     // stopped because of a mqtt command
    REASON_TARGET,   // reached the target azimuth.
    REASON_TURNTO,
    REASON_NONE
};

struct directionCtrl
{
    enum directionStates rotatorstate;
    int targetAzim;
};

struct measurement {
    enum meastype id;
    int gpio;
    int err;
    enum reasons reason;
    union {
        int count;
        int state; // used for buttons
        struct directionCtrl dirstates;
        //enum directionStates rotatorstate;
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