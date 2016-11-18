#ifndef Constants_h
#define Constants_h

#include <ESP8266WebServer.h>
#include <Wire.h>
#include <LIS331.h>




const float G_SCALE = 0.0007324; //sets g-level (48 fullscale range)/(2^16bits) = SCALE

/////////////////////
// Pin Definitions //
/////////////////////

const int BOARD_LED = 5; // 5 -> ESP8266 #5

// GPIO for speaker
const int SPEAKER_1 = 0; // 0 -> ESP8266 #0
const int SPEAKER_2 = 4; // 4 -> ESP8266 #4

// ADC for battery voltage
const int ANALOG_PIN = A0;



#endif
