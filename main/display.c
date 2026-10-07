
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <oled_display.h>

void display_text(const char *str)
{
    static int line = 0;

    if (!line) 
    {
        vTaskDelay(pdMS_TO_TICKS(80));
        oled_clear();
        vTaskDelay(pdMS_TO_TICKS(120));
    }    
    else vTaskDelay(pdMS_TO_TICKS(80));
    oled_draw_text(0, line++, str);
    if (line == 8) line = 0;
}

void display_printf(const char *format, ...)
{
    va_list args;
    char buffer[32];

    va_start(args, format);
    vsprintf(buffer, format, args);
    va_end(args);
    buffer[21] = 0;
    display_text(buffer);
}

