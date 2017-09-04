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

const int CHARGING_MAX = 795;
const int CHARGING_MIN = 645;
const int CHARGING_SCALE = 5;
const int CHARGING_TABLE[][2] =
                  {{ 795, 15  }
                  ,{ 790, 34  }
                  ,{ 785, 54  }
                  ,{ 780, 74  }
                  ,{ 775, 96  }
                  ,{ 770, 115 }
                  ,{ 765, 137 }
                  ,{ 760, 163 }
                  ,{ 755, 187 }
                  ,{ 750, 220 }
                  ,{ 745, 255 }
                  ,{ 740, 288 }
                  ,{ 735, 309 }
                  ,{ 730, 348 }
                  ,{ 725, 413 }
                  ,{ 720, 525 }
                  ,{ 715, 569 }
                  ,{ 710, 601 }
                  ,{ 705, 627 }
                  ,{ 700, 668 }
                  ,{ 695, 674 }
                  ,{ 690, 680 }
                  ,{ 685, 684 }
                  ,{ 680, 687 }
                  ,{ 675, 691 }
                  ,{ 670, 694 }
                  ,{ 665, 696 }
                  ,{ 660, 699 }
                  ,{ 655, 701 }
                  ,{ 650, 703 }
                  ,{ 645, 704 }
                  };

const int DECHARGING_MAX = 800;
const int DECHARGING_MIN = 645;
const int DECHARGING_SCALE = 5;
const int DECHARGING_TABLE[][2] =
                  {{ 800, 477 }
							    ,{ 795, 477 }
								  ,{ 790, 473 }
									,{ 785, 464 }
									,{ 780, 453 }
									,{ 775, 438 }
									,{ 770, 425 }
									,{ 765, 413 }
									,{ 760, 396 }
									,{ 755, 384 }
									,{ 750, 365 }
									,{ 745, 350 }
									,{ 740, 330 }
									,{ 735, 313 }
									,{ 730, 297 }
									,{ 725, 271 }
									,{ 720, 248 }
									,{ 715, 203 }
									,{ 710, 156 }
									,{ 705, 110 }
									,{ 700, 84 }
									,{ 695, 68 }
									,{ 690, 42 }
									,{ 685, 37 }
									,{ 680, 32 }
									,{ 675, 29 }
									,{ 670, 26 }
									,{ 665, 24 }
									,{ 660, 22 }
									,{ 655, 19 }
									,{ 650, 18 }
									,{ 645, 16 }
                };

#endif
