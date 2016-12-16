#ifndef Constants_h
#define Constants_h

#include <ESP8266WebServer.h>
#include <Wire.h>
#include <LIS331.h>

const float G_SCALE = 0.0007324; //sets g-level (48 fullscale range)/(2^16bits) = SCALE

/////////////////////
// Pin Definitions //
/////////////////////

// GPIO for speaker
// #0, #2, #4, #5, #12, #13, #14, #15, #1.  0,2 are used for LEDS. 4,5 are used for I2C
const int SPEAKER_1 = 15;
const int SPEAKER_2 = 16;
const int SPEAKER_3 = 12;
const int SPEAKER_4 = 13;
const int SPEAKER_5 = 14;



// GPIO for red
const int BOARD_LED_RED = 0;
const int BOARD_LED_BLUE = 2;

#endif
