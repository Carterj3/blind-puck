
/*
0000   48 54 54 50 2f 31 2e 31 20 32 30 30 20 4f 4b 0d  HTTP/1.1 200 OK.
0010   0a 43 6f 6e 74 65 6e 74 2d 54 79 70 65 3a 20 61  .Content-Type: a
0020   70 70 6c 69 63 61 74 69 6f 6e 2f 6a 73 6f 6e 0d  pplication/json.
0030   0a 43 6f 6e 74 65 6e 74 2d 4c 65 6e 67 74 68 3a  .Content-Length:
0040   20 33 0d 0a 43 6f 6e 6e 65 63 74 69 6f 6e 3a 20   3..Connection:
0050   63 6c 6f 73 65 0d 0a 41 63 63 65 73 73 2d 43 6f  close..Access-Co
0060   6e 74 72 6f 6c 2d 41 6c 6c 6f 77 2d 4f 72 69 67  ntrol-Allow-Orig
0070   69 6e 3a 20 2a 0d 0a 0d 0a                       in: *....

0000   38 30 32                                         802
*/

#include "Arduino.h"

#include <ESP8266WebServer.h>
#include <WifiHelpers.h>
#include <Constants.h>

// I2C, Accel
#include <Wire.h>
#include <LIS331.h>

String getVersion(){
  return "0.2.3.0";
}

String getTimeLeft(){
  int average = 0;
  int count = 100;
  int scale = 100;

  for(int i=1; i <= count; i++){
    average = computeRollingAverage(average, i, analogRead(A0), scale);
  }

  average = average / scale;

  int minutesLeft;
  if( average > DECHARGING_MAX){
    minutesLeft = DECHARGING_TABLE[0][1];
  }else if( average < DECHARGING_MIN){
    int lastIndex = (DECHARGING_MAX - DECHARGING_MIN) / DECHARGING_SCALE;
    minutesLeft = DECHARGING_TABLE[lastIndex][1];
  }else{
    int mod = (average % 5);
    int ceilingIndex = (DECHARGING_MAX - (average + (5 - mod)));
    int floorIndex = (DECHARGING_MAX - (average + (0 - mod)));

    int ceilingMinutes = DECHARGING_TABLE[ceilingIndex][1];
    int floorMinutes= DECHARGING_TABLE[floorIndex][1];

    minutesLeft = ((mod)*ceilingMinutes + (5 - mod)*floorMinutes) / 2;
  }

  String response = "";
  response += "{ \"adc\": " + String(average);
  response += ", \"hours\": " + String(minutesLeft / 60);
  response += ", \"minutes\": " + String(minutesLeft % 60);
  response += "}";

  return response;
}

int getAdc(){
  int average = 0;
  int count = 100;
  int scale = 100;

  for(int i=1; i <= count; i++){
    average = computeRollingAverage(average, i, analogRead(A0), scale);
  }

  return average / scale;
}

float computeMagnitude3d(float x, float y, float z){
   return pow(pow(x,2.0) + pow(y,2.0) + pow(z,2.0), 0.5);
}

int computeRollingAverage(int rollingAverage, int index, int newValue, int scale){
  return (rollingAverage*(index-1))/index + (scale*newValue)/index;
}
