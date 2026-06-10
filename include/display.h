#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino_GFX_Library.h>

enum State { IDLE, CALIBRATE, READY, CALCULATE, FINISHED, FAIL_STATE };

extern Arduino_GFX *gfx;

void initDisplay();
void updateDisplay(State state, int bpm, double delta, unsigned long calcStart, int calCount);

#endif
