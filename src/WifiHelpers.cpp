#include "Arduino.h"

#include <ESP8266WebServer.h>
#include <WifiHelpers.h>
#include <Constants.h>

// I2C, Accel
#include <Wire.h>
#include <LIS331.h>

String getVersion(){
  return "\"0.2.3.4\"";
}

String getTimeLeft(){
  int average = getAdc();

  int minutesLeft;
  if( average > DECHARGING_MAX){
    minutesLeft = DECHARGING_TABLE[0][1];
  }else if( average < DECHARGING_MIN){
    int lastIndex = (DECHARGING_MAX - DECHARGING_MIN) / DECHARGING_SCALE;
    minutesLeft = DECHARGING_TABLE[lastIndex][1];
  }else{
    int mod = (average % DECHARGING_SCALE);
    int ceilingIndex = (DECHARGING_MAX - (average + (DECHARGING_SCALE - mod))) / DECHARGING_SCALE;
    int floorIndex = (DECHARGING_MAX - (average + (0 - mod))) / DECHARGING_SCALE;

    int ceilingMinutes = DECHARGING_TABLE[ceilingIndex][1];
    int floorMinutes= DECHARGING_TABLE[floorIndex][1];

    minutesLeft = ((mod)*ceilingMinutes + (DECHARGING_SCALE - mod)*floorMinutes) / DECHARGING_SCALE;
  }

  int timeLeft;
  if( average > CHARGING_MAX){
    timeLeft = CHARGING_TABLE[0][1];
  }else if( average < CHARGING_MIN){
    int lastIndex = (CHARGING_MAX - CHARGING_MIN) / CHARGING_SCALE;
    timeLeft = CHARGING_TABLE[lastIndex][1];
  }else{
    int mod = (average % CHARGING_SCALE);
    int ceilingIndex = (CHARGING_MAX - (average + (CHARGING_SCALE - mod))) / CHARGING_SCALE;
    int floorIndex = (CHARGING_MAX - (average + (0 - mod))) / CHARGING_SCALE;

    int ceilingMinutes = CHARGING_TABLE[ceilingIndex][1];
    int floorMinutes= CHARGING_TABLE[floorIndex][1];

    timeLeft = ((mod)*ceilingMinutes + (CHARGING_SCALE - mod)*floorMinutes) / CHARGING_SCALE;
  }

  String response = "";
  response += "{ \"adc\": " + String(average);
  response += ", \"decharging hours\": " + String(minutesLeft / 60);
  response += ", \"decharging minutes\": " + String(minutesLeft % 60);
  response += ", \"charging hours\": " + String(timeLeft / 60);
  response += ", \"charging minutes\": " + String(timeLeft % 60);
  response += "}";

  return response;
}

int getAdc(){
  int average = 0;
  int count = 1000;
  int scale = 1000;

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
