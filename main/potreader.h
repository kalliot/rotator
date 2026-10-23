#ifndef __POTREADER__
#define __POTREADER__

#include "homeapp.h"
#include "mqtt_client.h"
#include "esp_adc/adc_oneshot.h"


extern void pot_set_calibr_low(void);
extern void pot_set_calibr_high(void);
extern int pot_get_calibr_low();
extern int pot_get_calibr_high();
extern float pot_get_azimuth(void);
extern void pot_send_azimuth(void);
extern void pot_set_tolerance(int tolerance);
extern void  pot_save_calibrations(void);
extern bool pot_init(uint8_t *chip, adc_oneshot_unit_handle_t adc_handle, adc_channel_t channel, int intervalMs, int cnt);
extern void  pot_close(void);


#endif