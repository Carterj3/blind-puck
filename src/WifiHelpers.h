#ifndef WifiHelpers_h
#define WifiHelpers_h

#include <WiFiClient.h>

void handleWiFiClient(WiFiClient client);

int computeRollingAverage(int rollingAverage, int index, int newValue, int scale);

#endif
