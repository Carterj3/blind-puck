#ifndef Constants_h
#define Constants_h

#include "Wire.h"
#include "Arduino.h"

const int SPEAKER_OFF_DURATION_MS = 1200;
const int SPEAKER_ON_DURATION_MS = 75;

/////////////////////
// Pin Definitions //
/////////////////////

// GPIO for speaker
// #0, #2, #4, #5, #12, #13, #14, #15, #1.  0,2 are used for LEDS. 4,5 are used for I2C
// 13,14 free
// 0, 2, 15 are involved in boot-mode
// 16 is used to wake up deep sleep
// 4, 5 are for I2C's SDA & SCL
const int SPEAKER_1 = 15;

const int UNUSED_1 = 16;
const int UNUSED_2 = 12;
const int UNUSED_3 = 13;
const int UNUSED_4 = 14;

// GPIO for red
const int BOARD_LED_RED = 0;
const int BOARD_LED_BLUE = 2;

const int BOARD_SDA = 4;
const int BOARD_SCL = 5;

#endif
