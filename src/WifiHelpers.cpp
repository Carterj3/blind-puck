
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
  return "0.1.1.18";
}

String getAdc(){
  int average = 0;
  for(int i=1; i <= 1000; i++){
    average = computeRollingAverage(average, i, analogRead(ANALOG_PIN), 1000);
  }
  return String(average/1000);
}

String resetLis331(LIS331 lis, bool lisX, bool lisY, bool lisZ){
  int16_t x,y,z;
  String response = "";
  response += "{ setPower: " + String(lis.setPowerStatus(LR_POWER_NORM));

  response += ", zLis?: " + String(lis.setXEnable(true));
  response += ", yLis?: " + String(lis.setYEnable(true));
  response += ", xLis?: " + String(lis.setZEnable(true));

  response += ", dataAvail: " + String(lis.statusHasDataAvailable());
  response += ", zAvail: " + String(lis.statusHasZDataAvailable());
  response += ", yAvail: " + String(lis.statusHasYDataAvailable());
  response += ", xAvail: " + String(lis.statusHasXDataAvailable());

  response += ", zOverun: " + String(lis.statusHasZOverrun());
  response += ", yOverun: " + String(lis.statusHasYOverrun());
  response += ", xOverun: " + String(lis.statusHasXOverrun());

  response += ", zEnable: " + String(lis.getZEnable());
  response += ", yEnable: " + String(lis.getYEnable());
  response += ", xEnable: " + String(lis.getXEnable());

  response += ", zValue: " + String(lis.getZValue(&z));
  response += ", yValue: " + String(lis.getYValue(&y));
  response += ", xValue: " + String(lis.getXValue(&x));

  response += ", z: " + String(z);
  response += ", y: " + String(y);
  response += ", x: " + String(x);

  response += ", powerStatus: " + String(lis.getPowerStatus());
  response += ", dataRate: " + String(lis.getDataRate());

  response += "}";

  return response;
}

String getLis331(LIS331 lis, bool lisX, bool lisY, bool lisZ){
  int16_t x,y,z;
  String response = "";
  response += "{ dataAvail: " + String(lis.statusHasDataAvailable());
  response += ", zAvail: " + String(lis.statusHasZDataAvailable());
  response += ", yAvail: " + String(lis.statusHasYDataAvailable());
  response += ", xAvail: " + String(lis.statusHasXDataAvailable());

  response += ", zOverun: " + String(lis.statusHasZOverrun());
  response += ", yOverun: " + String(lis.statusHasYOverrun());
  response += ", xOverun: " + String(lis.statusHasXOverrun());

  response += ", zEnable: " + String(lis.getZEnable());
  response += ", yEnable: " + String(lis.getYEnable());
  response += ", xEnable: " + String(lis.getXEnable());

  response += ", zLis?: " + String(lisX);
  response += ", yLis?: " + String(lisY);
  response += ", xLis?: " + String(lisZ);

  response += ", zValue: " + String(lis.getZValue(&z));
  response += ", yValue: " + String(lis.getYValue(&y));
  response += ", xValue: " + String(lis.getXValue(&x));

  response += ", z: " + String(z);
  response += ", y: " + String(y);
  response += ", x: " + String(x);

  response += ", powerStatus: " + String(lis.getPowerStatus());
  response += ", dataRate: " + String(lis.getDataRate());

  response += "}";

  return response;
}

String getAccel(LIS331 lis){
  int16_t x,y,z;

  lis.getXValue(&x);
  lis.getYValue(&y);
  lis.getZValue(&z);

  String response = "";
  response += "{ x (g's): "+String(float(x)*G_SCALE);
  response += ", y (g's): "+String(float(y)*G_SCALE);
  response += ", z (g's): "+String(float(z)*G_SCALE);
  response += "}";

  return response;
}

String getSpeaker(ESP8266WebServer server){
  int frequency = 2200;
  int delay = 100;
  int repeats = 10;
  String useTone = "true";

  for(int i=0; i<server.args(); i++){
    if(server.argName(i) == "delay"){
      delay = server.arg(i).toInt();
    }else if(server.argName(i) == "repeats"){
      repeats = server.arg(i).toInt();
    }else if(server.argName(i) == "frequency"){
      frequency = server.arg(i).toInt();
    }else if(server.argName(i) == "tone"){
      useTone = server.arg(i);
    }
  }

  if(useTone == "true"){
    for(int i=0; i<repeats; i++){

      delayMicroseconds(delay/2);

      tone(SPEAKER_1, frequency);
      tone(SPEAKER_2, frequency);

      delayMicroseconds(delay/2);

      noTone(SPEAKER_1);
      noTone(SPEAKER_2);
    }
  }else{
    for(int i=0; i<repeats; i++){

      delayMicroseconds(delay/2);

      digitalWrite(SPEAKER_1, HIGH);
      digitalWrite(SPEAKER_2, HIGH);

      delayMicroseconds(delay/2);

      digitalWrite(SPEAKER_1, LOW);
      digitalWrite(SPEAKER_2, LOW);
    }
  }

  String response = "";
  response += "{ repeats: "+String(repeats);
  response += ", delay: "+String(delay);
  response += ", frequency: "+String(frequency);
  response += ", tone: "+String(useTone == "true");

  response += "}";

  return response;
}

float computeMagnitude3d(float x, float y, float z){
   return pow(pow(x,2.0) + pow(y,2.0) + pow(z,2.0), 0.5);
}

int computeRollingAverage(int rollingAverage, int index, int newValue, int scale){
  return (rollingAverage*(index-1))/index + (scale*newValue)/index;
}
