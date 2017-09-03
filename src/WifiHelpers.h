#ifndef WifiHelpers_h
#define WifiHelpers_h

// I2C, Accel
#include <Wire.h>
#include <LIS331.h>

#include <WiFiClient.h>

String getTimeLeft();
String getVersion();
int getAdc();

float computeMagnitude3d(float x, float y, float z);
int computeRollingAverage(int rollingAverage, int index, int newValue, int scale);

#endif
