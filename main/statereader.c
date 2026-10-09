#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/gpio.h"
#include "soc/frc_timer_reg.h"
#include "esp_log.h"
#include "homeapp.h"
#include "statereader.h"


static struct stateInstance
{
    int gpio;
    bool prevState;
    SemaphoreHandle_t xSemaphore;
} *instances;


static int instance_count;
static const char *TAG = "STATEREADER";

static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    struct stateInstance *instance = (struct stateInstance *) arg;
    xSemaphoreGive(instance->xSemaphore);
}

static void queue_message(struct stateInstance *instance, bool state)
{
    struct measurement meas;
    meas.gpio = instance->gpio;
    meas.id = STATE;
    meas.data.state = state;
    xQueueSendFromISR(evt_queue, &meas, NULL);
    instance->prevState = state;
}


static void state_reader(void *arg)
{
    struct stateInstance *instance = (struct stateInstance *) arg;
    bool state;

    state = !gpio_get_level(instance->gpio);
    queue_message(instance, state);

    while (1) {
        if (xSemaphoreTake(instance->xSemaphore, portMAX_DELAY)) {
            vTaskDelay(50 / portTICK_PERIOD_MS); // wait for all glitches
            state = !gpio_get_level(instance->gpio);
            if (state != instance->prevState) {
                queue_message(instance, state);
            }
        } 
    }
}


void stateread_init(int amount)
{
    ESP_LOGI(TAG,"statereader init");
    instance_count = amount;
    instances = malloc(sizeof(struct stateInstance) * amount);

    for (int i=0; i<amount; i++) {
        instances[i].prevState = false;
        instances[i].xSemaphore = xSemaphoreCreateBinary();
    }
    //gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    ESP_LOGI(TAG,"statereader init done");
}


bool stateread_start(int index, int gpio)
{
    ESP_LOGI(TAG,"statereader start index %d, gpio %d",index, gpio);
    if (index < instance_count) 
    {
        struct stateInstance *instance = &instances[index];

        instances[index].gpio = gpio;
        gpio_reset_pin(gpio);
        gpio_pullup_en(gpio);
        gpio_set_direction(gpio, GPIO_MODE_INPUT);
        gpio_set_intr_type(gpio, GPIO_INTR_ANYEDGE);
        xTaskCreate(state_reader, "state reader", 2048, (void*) instance, 10, NULL);
        gpio_isr_handler_add(gpio, gpio_isr_handler, (void*) instance);
        ESP_LOGI(TAG,"statereader start done");
        return true;
    }
    ESP_LOGD(TAG,"too big instance number given, max is %d", instance_count -1);
    return false;
}


struct stateInstance * find_instance_by_gpio(int gpio)
{
    for (int i=0; i<instance_count;i++) {
        if (instances[i].gpio == gpio) return &instances[i];
    }
    return NULL;
}


void stateread_done(struct measurement *meas)
{
    struct stateInstance *inst = find_instance_by_gpio(meas->gpio);

    if (inst == NULL) {
        ESP_LOGD(TAG,"gpio %d not found from instances", meas->gpio);
        return;
    }
    inst->prevState = meas->data.state;
}
