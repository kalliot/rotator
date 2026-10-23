#ifndef __DISPLAY__
#define __DISPLAY__

#include <stdlib.h>
#include "homeapp.h"

enum displayline
{
    commstatus,
    dir_azimuth,
    line2,
    line3,
    line4,
    line5,
    turndirection,
    information
};


extern void display_text(const char *str);
extern void display_printf(enum displayline line, const char *format, ...);
extern void display_turn(enum directionStates direction);

#endif