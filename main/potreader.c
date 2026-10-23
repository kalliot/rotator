
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "mqtt_client.h"
#include "flashmem.h"
#include "statistics/statistics.h"
#include "homeapp.h"



static adc_oneshot_unit_handle_t adc1_handle;
static uint8_t *chipid;
static int sampleInterval = 1000;
static int azimuth;
static int samplecnt = 10;
static int lastRaw = 0;
static int prevAzim = 0;
static int currTolerance = 3;
static SemaphoreHandle_t mutex;

static const char *TAG = "potreader";
static int errState = 0;
static adc_channel_t pot_channel;

enum {
    CAL_MIN,
    CAL_MAX
};

static struct {
    char *rawname;
    int raw;
} calibr[] = {
    { "cal.minraw",149,},  // default values.
    { "cal.maxraw",3800}
};


static float convert(int raw)
{
    float rdiff = calibr[CAL_MAX].raw - calibr[CAL_MIN].raw;   
    float d = raw - calibr[CAL_MIN].raw;                       
    float x = d / rdiff;                                       
    return x * 360;
}

static int pot_read(void)
{
    int adc_raw;

    adc_oneshot_read(adc1_handle, pot_channel, &adc_raw);
    return adc_raw;
}

static void queue_measurement(int azimval, int err)
{
    struct measurement meas;
    meas.id = AZIMUTH;
    meas.gpio = 36;
    meas.data.azimuth = azimval;
    meas.err = err;
    xQueueSend(evt_queue, &meas, 0);
}


void pot_set_calibr_low(void)
{
    if (xSemaphoreTake(mutex, (TickType_t) 1000) == pdTRUE)
    {
        int rawvalue  = lastRaw;
        ESP_LOGI(TAG,"changing calibration low %d -> %d\n", calibr[CAL_MIN].raw, rawvalue);
        calibr[CAL_MIN].raw  = rawvalue;
        xSemaphoreGive(mutex);
    }
}


void pot_set_calibr_high(void)
{
    
    if (xSemaphoreTake(mutex, (TickType_t) 1000) == pdTRUE)
    {
        int rawvalue  = lastRaw;
        ESP_LOGI(TAG,"changing calibration high %d -> %d\n", calibr[CAL_MAX].raw, rawvalue);
        calibr[CAL_MAX].raw  = rawvalue;
        xSemaphoreGive(mutex);
    }
}

int pot_get_calibr_low(void)
{
    return calibr[CAL_MIN].raw;
}

int pot_get_calibr_high(void)
{
    return calibr[CAL_MAX].raw;
}

void pot_send_azimuth(void)
{
    queue_measurement(convert(pot_read()),0);
}

bool pot_save_calibrations(void)
{
    ESP_LOGI(TAG,"saving calibrations to flash");
    if (calibr[CAL_MAX].raw  < calibr[CAL_MIN].raw)
    {
        ESP_LOGE(TAG,"Error: calibration maxraw is lower than minraw");
        return false;
    }
    flash_write(setup_flash, calibr[CAL_MAX].rawname, calibr[CAL_MAX].raw);
    flash_write(setup_flash, calibr[CAL_MIN].rawname, calibr[CAL_MIN].raw);
    flash_commitchanges(setup_flash);
    queue_measurement(convert(pot_read()),0);
    return true;
}


float pot_get_azimuth(void)
{
    for (int i=0; i < 3; i++)
    {
        if (xSemaphoreTake(mutex, (TickType_t ) 1000) == pdTRUE)
        {
            prevAzim = azimuth;
            xSemaphoreGive(mutex);
            break;
        }
    }
    return prevAzim;
}


static void pot_reader(void* arg)
{
    int cnt = 0;
    int sum = 0;
    int minraw =  0xfff; // dont count on smallest
    int maxraw = -0xfff; // and biggest samples
    int raw = pot_read();
    azimuth = convert(raw);

    for(;;)
    {
        raw = pot_read();

        sum += raw;
        errState = 0; // check which value we have here, if ntc is missing
        if (raw == 0xfff || raw == -0xfff) errState = 1;
        if (raw < minraw) minraw = raw;
        if (raw > maxraw) maxraw = raw;
        if (++cnt == samplecnt)
        {
            // dont count on min and max values
            int avg = (sum - minraw - maxraw) / (samplecnt - 2);
            cnt = 0;
            sum = 0;
            //ESP_LOGI(TAG,"pot_averaged raw is %d", avg);
            if (xSemaphoreTake(mutex, (TickType_t) 1000) == pdTRUE)
            {
                lastRaw = avg; // lastraw is needed for calibrations;
                azimuth = convert(avg);
                int diff = abs(prevAzim - azimuth);
                time_t now;
                time(&now);
                if (diff >= currTolerance)
                {
                    prevAzim = azimuth;
                    queue_measurement(azimuth, errState);
                }
                xSemaphoreGive(mutex);
            }
            minraw =  0xfff;
            maxraw = -0xfff;
        }
        vTaskDelay(sampleInterval / portTICK_PERIOD_MS);
    }
}

void pot_set_tolerance(int tolerance)
{
    for (int i=0; i < 3; i++)
    {
        if (xSemaphoreTake(mutex, (TickType_t ) 1000) == pdTRUE)
        {
            currTolerance = tolerance;
            xSemaphoreGive(mutex);
            break;
        }
    }
}

bool pot_init(uint8_t *chip, adc_oneshot_unit_handle_t adc_handle, adc_channel_t channel, int intervalMs, int cnt)
{
    pot_channel = channel;
    mutex = xSemaphoreCreateMutex();
    if (mutex == NULL)
    {
        ESP_LOGE(TAG,"failed to create mutex");
        return false;
    }
    samplecnt = cnt;
    calibr[CAL_MIN].raw      = flash_read(setup_flash, calibr[CAL_MIN].rawname,  calibr[CAL_MIN].raw);
    calibr[CAL_MAX].raw      = flash_read(setup_flash, calibr[CAL_MAX].rawname,  calibr[CAL_MAX].raw);

    chipid = chip;
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = ADC_ATTEN_DB_6
    };
    adc1_handle = adc_handle;
    adc_oneshot_config_channel(adc1_handle, pot_channel, &config);
    // mutex is here not needed, we are not yet threading.
    azimuth = convert(pot_read());
    ESP_LOGI(TAG,"pot_init, first azimuth read is %f", azimuth);
    sampleInterval = intervalMs / samplecnt;
    xTaskCreate(pot_reader, "pot_reader", 2048, NULL, 10, NULL);
    return true;
}

void pot_close(void)
{
    return;
}

