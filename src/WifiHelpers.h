#ifndef WifiHelpers_h
#define WifiHelpers_h

// I2C, Accel
#include <Wire.h>
#include <LIS331.h>

#include <WiFiClient.h>

String getVersion();
String getAdc();
String resetLis331(LIS331 lis, bool lisX, bool lisY, bool lisZ);
String getLis331(LIS331 lis, bool lisX, bool lisY, bool lisZ);
String getAccel(LIS331 lis);
String getSpeaker(ESP8266WebServer server);

float computeMagnitude3d(float x, float y, float z);
int computeRollingAverage(int rollingAverage, int index, int newValue, int scale);

#endif
