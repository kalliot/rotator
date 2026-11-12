
#include <time.h>
#include "driver/gpio.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "homeapp.h"
#include "potreader.h"
#include "statistics/statistics.h"
#include "display.h"
#include "rotator.h"


static int targetAzim = -1;
static int dispTargetAzim = -1;
static int currAzim = -1;
static bool turningCW = false;
static bool turningCCW = false;
static bool initDone = false;
static char stateTopic[64];
static uint8_t *chipid;
static esp_timer_handle_t start_cw;
static esp_timer_handle_t start_ccw;
static const char *TAG = "ROT_ROTATOR";

static int startedDegr = 0;
static uint64_t stoppedTic = 0;
static uint64_t startedTic = 0;



#define INERTDEGR  "inertdegr"
#define INERTMS    "inertms"
#define BEAMWIDTH  "beamwidth"
#define HARDSTOP   "hardstop"
#define TURNSPEED  "turnspeed"

#define DIRECTION_CW      true
#define DIRECTION_CCW     false

#define RELAYSYNCMS    50

struct rotatorSettings setup =
{
    .hardStop = 0,
    .inertiaMs = 2000,
    .inertiaDegr = 1,
    .beamWidth = 10,
    .turnSpeed = 25
};

enum machine_states
{
    SM_TARGET,
    SM_BUTTON
} stateMachine;

struct stateRec 
{
    enum directionStates state;
    char *statename;
} stateStrings[] =
{ 
    { STATE_STILL, "still"},
    { STATE_CW, "cw"},
    { STATE_CCW, "ccw"},
    { STATE_NONE, "none"}
};    

struct reasonRec
{
    enum reasons reason;
    char *reasonname;
} reasonStrings[] =
{
    { REASON_BUTTON, "button"},
    { REASON_DIRCHANGE, "dirchange"},
    { REASON_MQTT, "mqtt"},
    { REASON_TARGET, "target"},
    { REASON_TIMEOUT, "timeout"},
    { REASON_TURNTO, "turnto"},
    { REASON_NONE, "none"},
};


static void queue_state(enum directionStates state, int target, enum reasons reason)
{
    struct measurement meas;
    meas.id = ROTATORSTATE;
    meas.data.dirstates.rotatorstate = state;
    meas.data.dirstates.targetAzim = target;
    meas.reason = reason;
    xQueueSend(evt_queue, &meas, 0);
}

static char *reasonStr(enum reasons reason)
{
    int i;

    for (i=0; reasonStrings[i].reason != REASON_NONE; i++)
    {
        if (reasonStrings[i].reason == reason) break;
    }
    return reasonStrings[i].reasonname;
}


static char *stateStr(enum directionStates state)
{
    int i;

    for (i=0; stateStrings[i].state != STATE_NONE; i++)
    {
        if (stateStrings[i].state == state) break;
    }
    return stateStrings[i].statename;
}

static enum directionStates str2state(char *str)
{
    int i;

    for (i=0; stateStrings[i].state != STATE_NONE; i++)
    {
        if (!strcmp(stateStrings[i].statename, str)) break;
    }
    return stateStrings[i].state;
}

void rotator_show_settings(void)
{
    struct measurement meas;
    meas.id = PUBLISH_ROTATOR_SETTINGS;
    xQueueSend(evt_queue, &meas, 0);
}


static void publish_state(char * prefix, struct directionCtrl *dirCtrl, enum reasons reason, esp_mqtt_client_handle_t client)
{
    time_t now;
    int retain = 1;

    time(&now);
    if (now < MIN_EPOCH)
    {
        now = 0;
        retain = 0;
    }
    static char *datafmt = "{\"dev\":\"%x%x%x\",\"id\":\"movestate\",\"state\":\"%s\",\"reason\":\"%s\",\"target\":%d,\"ts\":%jd}";
    sprintf(stateTopic,"%s/rotator/%x%x%x/state", prefix, chipid[3], chipid[4], chipid[5]);

    sprintf(jsondata, datafmt,
                chipid[3],chipid[4],chipid[5],
                stateStr(dirCtrl->rotatorstate),
                reasonStr(reason),
                dirCtrl->targetAzim,
                now);
    esp_mqtt_client_publish(client, stateTopic, jsondata , 0, 0, retain);
    statistics_getptr()->sendcnt++;
}


static void rotator_publish_settings(char * prefix, esp_mqtt_client_handle_t client)
{
    time_t now;
    int retain = 1;

    time(&now);
    if (now < MIN_EPOCH)
    {
        now = 0;
        retain = 0;
    }
    static char *datafmt = "{\"dev\":\"%x%x%x\",\"id\":\"settings\",\"hardstop\":%d,\"ts\":%jd,\"inertiams\":%d, \"inertiadegr\":%d,\"beamwidth\":%d,\"turnspeed\":%d}";
    sprintf(stateTopic,"%s/rotator/%x%x%x/settings", prefix, chipid[3], chipid[4], chipid[5]);

    sprintf(jsondata, datafmt,
                chipid[3],chipid[4],chipid[5],
                setup.hardStop, now, setup.inertiaMs, setup.inertiaDegr, setup.beamWidth, setup.turnSpeed);
    esp_mqtt_client_publish(client, stateTopic, jsondata , 0, 0, retain);
    statistics_getptr()->sendcnt++;
}

bool rotator_publish_event(char *prefix,struct measurement *meas, esp_mqtt_client_handle_t client)
{
    bool ret = false;

    switch (meas->id)
    {
        case PUBLISH_ROTATOR_SETTINGS:
            rotator_publish_settings(prefix, client);
            ret = true;
        break;

        case ROTATORSTATE:
            publish_state(prefix, &meas->data.dirstates, meas->reason, client);
            ret = true;
        break;

        default:
            ret = false;
        break;
    }
    return ret;
}


void rotator_setsettings(struct rotatorSettings *newsetup)
{
    bool wrote = false;
    if (newsetup->inertiaDegr != setup.inertiaDegr )
    {
        setup.inertiaDegr = newsetup->inertiaDegr;
        flash_write(setup_flash, INERTDEGR, newsetup->inertiaDegr);
        wrote = true;
    }
    if (newsetup->inertiaMs != setup.inertiaMs )
    {
        setup.inertiaMs = newsetup->inertiaMs;
        flash_write(setup_flash, INERTMS, newsetup->inertiaMs);
        wrote = true;
    }
    if (newsetup->beamWidth != setup.beamWidth )
    {
        setup.beamWidth = newsetup->beamWidth;
        flash_write(setup_flash, BEAMWIDTH, newsetup->beamWidth);
        wrote = true;
    }
    if (newsetup->hardStop != setup.hardStop )
    {
        setup.hardStop = newsetup->hardStop;
        flash_write(setup_flash, HARDSTOP, newsetup->hardStop);
        wrote = true;
        ESP_LOGI(TAG,"write hardstop %d", setup.hardStop);
    }
    if (newsetup->turnSpeed != setup.turnSpeed )
    {
        setup.turnSpeed = newsetup->turnSpeed;
        flash_write(setup_flash, TURNSPEED, newsetup->turnSpeed);
        wrote = true;
        ESP_LOGI(TAG,"write turnspeed %d", setup.turnSpeed);
    }
    if (wrote) flash_commitchanges(setup_flash);
}


void read_settings(void)
{
    setup.inertiaDegr = flash_read(setup_flash, INERTDEGR,setup.inertiaDegr);
    setup.inertiaMs = flash_read(setup_flash, INERTMS, setup.inertiaMs);
    setup.beamWidth = flash_read(setup_flash, BEAMWIDTH, setup.beamWidth);
    setup.hardStop = flash_read(setup_flash, HARDSTOP, setup.hardStop);
    setup.turnSpeed = flash_read(setup_flash, TURNSPEED, setup.turnSpeed);
}

static void start_turning(enum directionStates direction, int target)
{
    uint64_t nowTic = esp_timer_get_time();

    uint64_t sinceStopped;

    if (stoppedTic != 0)
    {
        sinceStopped = (nowTic - stoppedTic) / 1000;
        ESP_LOGI(TAG, "Tics gone since stop %llu", sinceStopped);
        if (sinceStopped > 0 && (sinceStopped < setup.inertiaMs))
        {
            vTaskDelay(pdMS_TO_TICKS(setup.inertiaMs - sinceStopped));
        }
    }
    queue_state(direction, target, REASON_TURNTO);
    switch (direction)
    {
        case STATE_CCW:
            gpio_set_level(DIRECTIONRELAY_GPIO, DIRECTION_CCW); // actually this is just for code clarity.
            gpio_set_level(MAINSRELAY_GPIO, true);  // feed mains to the big transformer
            turningCCW = true;
            startedTic = nowTic;
            startedDegr = pot_get_azimuth();
            display_turn(STATE_CCW);
        break;

        case STATE_CW:
            gpio_set_level(DIRECTIONRELAY_GPIO, DIRECTION_CW); // turn 24v directionrelay on
            vTaskDelay(pdMS_TO_TICKS(RELAYSYNCMS));      // be sure it was turned first
            gpio_set_level(MAINSRELAY_GPIO, true);  // feed mains to the big transformer
            turningCW = true;
            startedTic = nowTic;
            startedDegr = pot_get_azimuth();
            display_turn(STATE_CW);
        break;

        default:
            ESP_LOGI(TAG,"unknown turning direction, nothing done!");
            return;
    }
}

bool rotator_isturning_ccw(void)
{
    return turningCCW;
}

bool rotator_isturning_cw(void)
{
    return turningCW;
}


static void start_timered_turn_callback(void *arg)
{
    enum directionStates *direction = (enum directionStates *) arg;
    start_turning(*direction, dispTargetAzim);
}


 void stop_turning(enum reasons reason)
{
    queue_state(STATE_STILL, -1, reason);
    gpio_set_level(MAINSRELAY_GPIO, false); // take 230v off first, THIS IS IMPORTANT.
    vTaskDelay(pdMS_TO_TICKS(RELAYSYNCMS));
    gpio_set_level(DIRECTIONRELAY_GPIO, DIRECTION_CCW); // and then 24v
    stoppedTic = esp_timer_get_time();
    display_turn(STATE_STILL);
    turningCCW = false;
    turningCW = false;
    startedTic = 0;
}

int rotator_get_turnspeed(void)
{
    int turnSpeed = 0;
    uint64_t currTic = esp_timer_get_time();
    int currDegr = pot_get_azimuth();
    int turnSeconds = (currTic - startedTic) / 1000000;

    int degsTurned = currDegr - startedDegr;
    if (turnSeconds > 0)
    {
        turnSpeed = abs(10 * degsTurned / turnSeconds);
    }

    if (startedTic != 0 && turnSeconds != 0)
    {
        ESP_LOGI(TAG,"turn of %d degrees took %d seconds", degsTurned, turnSeconds);
        ESP_LOGI(TAG,"turn speed was %d degrees / 10sec", turnSpeed);
        if (turnSeconds > 10 && turnSpeed < (setup.turnSpeed * 0.75))
        {
            stop_turning(REASON_TIMEOUT);
            pot_set_tolerance(IDLE_ACCURACY);
        }
    }
    return turnSpeed;
}


bool rotator_turn(enum directionStates direction, enum reasons reason)
{
    bool ret = false;

    if (!initDone) return false;

    switch (direction)
    {
        case STATE_CCW:
            if (turningCW)
            {   stop_turning(REASON_DIRCHANGE);
                ESP_ERROR_CHECK(esp_timer_start_once(start_ccw, setup.inertiaMs * 1000));
            }
            else 
            {
                start_turning(STATE_CCW, -1);
                display_turn(direction);
            }
            stateMachine = SM_BUTTON;
            ret = true;
        break;

        case STATE_CW:
            if (turningCCW)
            {
                stop_turning(REASON_DIRCHANGE);
                ESP_ERROR_CHECK(esp_timer_start_once(start_cw, setup.inertiaMs * 1000));
            }
            else
            {
                start_turning(STATE_CW,-1);
                display_turn(direction);
            }
            stateMachine = SM_BUTTON;
            ret = true;
        break;

        case STATE_STILL:
            esp_timer_stop(start_ccw);
            esp_timer_stop(start_cw);
            stop_turning(reason);
        break;

        case STATE_NONE:
            ESP_LOGI(TAG,"unknown turning direction, nothing done");
        break;
    }
    return ret;
}

bool rotator_turn_bystr(char *dirStr)
{
    return rotator_turn(str2state(dirStr), REASON_MQTT);
}

bool rotator_turn2target(int potTarget, int dispTarget)
{
    bool ret = false;
    if (potTarget == -1) return ret;

    int tolerance = setup.beamWidth / 2;
    dispTargetAzim = -1;
    ESP_LOGI(TAG,"received turn to %d command", potTarget);
    // we are close to cw end and target is close to ccw end,
    // then turn to closest end.
    // example1: currentAzim = 357 and target = 3 --> dont turn a long turn, instead turn to cw endpoint ie 360.
    if (((360 - currAzim) < 90) && (potTarget < tolerance))
    {
        ESP_LOGI(TAG,"optimising long ccw rotation to short cw rotation");
        targetAzim = 360;
        tolerance = 0; // beamwidht is already in use
    }
    // example2: currentAzim = 3 and potTarget = 357 --> dont turn a long turn, instead turn to ccw endpoint ie 0.
    else if (((360 - currAzim) > (360 - 90)) && potTarget > (360 - tolerance))
    {
        ESP_LOGI(TAG,"optimising long cw rotation to short ccw rotation");
        targetAzim = 0;
        tolerance = 0; // beamwidht is already in use
    }
    else
        targetAzim = potTarget;

    if ((targetAzim - tolerance) > currAzim)
    {
        if (turningCCW)
        {   stop_turning(REASON_DIRCHANGE);
            dispTargetAzim = dispTarget;
            ESP_ERROR_CHECK(esp_timer_start_once(start_cw,  setup.inertiaMs * 1000));
        }
        else start_turning(STATE_CW, dispTarget);
        stateMachine = SM_TARGET;
        ret = true;
    }
    else if ((targetAzim + tolerance) < currAzim)
    {
        if (turningCW)
        {
            stop_turning(REASON_DIRCHANGE);
            dispTargetAzim = dispTarget;
            ESP_ERROR_CHECK(esp_timer_start_once(start_ccw,  setup.inertiaMs * 1000));
        }
        else start_turning(STATE_CCW, dispTarget);
        stateMachine = SM_TARGET;
        ret = true;
    }
    else if (turningCW || turningCCW)
    {
        stop_turning(REASON_TARGET);
    }
    return ret;
}


bool rotator_turned(int azim)
{
    bool ret = true;

    if (stateMachine == SM_TARGET)
    {
        if (turningCW && (azim >= (targetAzim - setup.inertiaDegr)))
        {
            stop_turning(REASON_TARGET);
            ESP_LOGI(TAG,"ended turn cw to target %d, azim=%d", targetAzim, azim);
            ret = false;
        }
        if (turningCCW && (azim <= (targetAzim + setup.inertiaDegr)))
        {
            stop_turning(REASON_TARGET);
            ESP_LOGI(TAG,"ended turn ccw to target %d, azim=%d", targetAzim, azim);
            ret = false;
        }
    }
    currAzim = azim;
    return ret; // true, if we still want to continue
}

struct rotatorSettings *rotator_getsettings()
{
    return &setup;
}

void rotator_init(uint8_t *chip)
{
    static enum directionStates cw = STATE_CW;
    static enum directionStates ccw = STATE_CCW;


    gpio_reset_pin(DIRECTIONRELAY_GPIO);
    gpio_reset_pin(MAINSRELAY_GPIO);

    gpio_set_direction(DIRECTIONRELAY_GPIO,  GPIO_MODE_OUTPUT);
    gpio_set_direction(MAINSRELAY_GPIO, GPIO_MODE_OUTPUT);

    const esp_timer_create_args_t start_ccw_args =
    {
        .callback = start_timered_turn_callback,
        .arg = (void *) &ccw,
        .name = "start_ccw_timer"
    };
    const esp_timer_create_args_t start_cw_args =
    {
        .callback = start_timered_turn_callback,
        .arg = (void *) &cw,
        .name = "start_cw_timer"
    };

    chipid = chip;
    read_settings();
    currAzim = pot_get_azimuth();
    esp_timer_create(&start_cw_args, &start_cw);
    esp_timer_create(&start_ccw_args, &start_ccw);
    initDone = true;
}
