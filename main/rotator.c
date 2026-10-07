
#include <time.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "homeapp.h"
#include "potreader.h"
#include "statistics/statistics.h"


static int targetAzim = -1;
static int currAzim = -1;
static bool turningCW = false;
static bool turningCCW = false;
static char stateTopic[64];
static uint8_t *chipid;

static const char *TAG = "ROT_ROTATOR";


static void queue_state(enum states state)
{
    struct measurement meas;
    meas.id = ROTATORSTATE;
    meas.data.rotatorstate = state;
    xQueueSend(evt_queue, &meas, 0);
}

struct stateRec 
{
    enum states state;
    char *statename;
};

struct stateRec stateStrings[] =
{ 
    { STATE_STILL, "still"},
    { STATE_CW, "cw"},
    { STATE_CCW, "ccw"}
};    

static char *stateStr(enum states state)
{
    for (int i=0; i<3; i++)
    {
        if (stateStrings[i].state == state)
        {
            return stateStrings[i].statename;
        }
    }
    return "none";
}

void rotator_publish_state(char * prefix, struct measurement *data, esp_mqtt_client_handle_t client)
{
    time_t now;
    int retain = 1;

    time(&now);
    if (now < MIN_EPOCH)
    {
        now = 0;
        retain = 0;
    }
    static char *datafmt = "{\"dev\":\"%x%x%x\",\"id\":\"movestete\",\"state\":%s,\"ts\":%jd}";
    sprintf(stateTopic,"%s/rotator/%x%x%x/state", prefix, chipid[3], chipid[4], chipid[5]);

    sprintf(jsondata, datafmt,
                chipid[3],chipid[4],chipid[5],
                stateStr(data->data.rotatorstate),
                now);
    esp_mqtt_client_publish(client, stateTopic, jsondata , 0, 0, retain);
    statistics_getptr()->sendcnt++;
}


void rotator_turn2target(int target)
{
    if (target == -1) return;

    ESP_LOGI(TAG,"received turn to %d command", target);
    targetAzim = target;
    if (targetAzim > currAzim)
    {
        if (turningCCW)
        {
            gpio_set_level(TURNCCW_GPIO, false);
            queue_state(STATE_STILL);
            turningCCW = false;
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        gpio_set_level(TURNCW_GPIO, true);
        turningCW = true;
        queue_state(STATE_CW);
    }
    else
    {           
        if (turningCW)
        {
            gpio_set_level(TURNCW_GPIO, false);
            queue_state(STATE_STILL);
            turningCW = false;
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
        gpio_set_level(TURNCCW_GPIO, true);
        turningCCW = true;
        queue_state(STATE_CCW);
    }
}

bool rotator_turned(int azim)
{
    bool ret = true;
    if (turningCW && azim >= targetAzim)
    {
        gpio_set_level(TURNCW_GPIO, false);
        turningCW = false;
        queue_state(STATE_STILL);
        ESP_LOGI(TAG,"ended turn cw to target %d, azim=%d", targetAzim, azim);
        ret = false;
    }
    if (turningCCW && azim <= targetAzim)
    {
        gpio_set_level(TURNCCW_GPIO, false);
        turningCCW = false;
        queue_state(STATE_STILL);
        ESP_LOGI(TAG,"ended turn ccw to target %d, azim=%d", targetAzim, azim);
        ret = false;
    }    
    currAzim = azim;
    return ret;
}

void rotator_init(uint8_t *chip)
{
    chipid = chip;
    currAzim = pot_get_azimuth();
}
