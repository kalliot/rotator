#ifndef __STATEREADER__
#define __STATEREADER__

#include "homeapp.h"

extern void stateread_init(int amount);
extern bool stateread_start(int index, int gpio);
extern void stateread_done(struct measurement *meas);

#endif