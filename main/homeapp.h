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
    OTA
};

enum states 
{
    STATE_STILL,
    STATE_CW,
    STATE_CCW
};


struct measurement {
    enum meastype id;
    int gpio;
    int err;

    union {
        int count;
        enum states rotatorstate;
        int azimuth;
    } data;
};

extern QueueHandle_t evt_queue;
extern char jsondata[];
extern nvs_handle setup_flash;

#define BLINK_GPIO         23
#define TURNCW_GPIO        16
#define TURNCCW_GPIO       17
#define MIN_EPOCH   1650000000

#endif