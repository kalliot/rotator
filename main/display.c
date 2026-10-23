
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <oled_display.h>
#include "display.h"


void display_text(const char *str)
{
    static int line = 0;

    vTaskDelay(pdMS_TO_TICKS(100));
    if (str == NULL)
    {
        oled_clear();
        vTaskDelay(pdMS_TO_TICKS(140));
        line = 0;
        return;
    }
    if (!line)
    {
        oled_clear();
        vTaskDelay(pdMS_TO_TICKS(140));
    }
    oled_draw_text(0, line++, str);
    if (line == 8) line = 0;
}


void display_printf(enum displayline line, const char *format, ...)
{
    va_list args;
    char buffer[21];

    buffer[0]=' ';
    if (format == NULL)
    {
        oled_draw_text(0,line,"                     ");
    }
    else
    {
        va_start(args, format);
        vsprintf(&buffer[1], format, args);
        va_end(args);
        oled_draw_text(0, line, buffer);
    }
    vTaskDelay(pdMS_TO_TICKS(150));
}


void display_turn(enum directionStates direction)
{
    switch (direction)
    {
        case STATE_CCW:
            display_printf(turndirection, "<--                ");
            break;

        case STATE_CW:
            display_printf(turndirection, "                -->");
            break;

        case STATE_STILL:
            display_printf(turndirection, "        ---        ");
            break;

        case STATE_NONE:
            display_printf(turndirection, "                   ");
            break;

        default:
            break;
    }
}
