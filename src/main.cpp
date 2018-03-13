// WiFi
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPUpdateServer.h>

#include "BpSpeaker.h"
#include "BpImu.h"

#include "Blindpuck.h"
#include "Constants.h"

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

int SPEAKER_PINS[2] = { SPEAKER_1, BOARD_LED_BLUE };
BpSpeaker speakerModule(SPEAKER_PINS, 2, SPEAKER_ON_DURATION_MS, SPEAKER_OFF_DURATION_MS);
BpImu imuModule;

const char* host = "esp8266-webupdate";
const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";

const char WiFiAPPSK[] = "sparkfun";

void initHardware();
void setupWiFi();
void setupServer();

void setup()
{

  initHardware();
  setupWiFi();
  setupServer();

  delay(2000);

  digitalWrite(BOARD_LED_RED, HIGH);
}


void loop()
{

  speakerModule.tick();
  imuModule.tick();

  server.handleClient();
  delay(25);
}

void setupWiFi()
{
  WiFi.mode(WIFI_AP);

  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "Thing-":
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "_P_v3.0_" + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  WiFi.softAP(AP_NameChar, WiFiAPPSK);
}

void setupServer()
{
  // HTTP Server
  server.on("/version", [](){
    server.send(200, "application/json", "\"3.0.0.0\"");
  });

  server.on("/restart", [](){
     server.send(200, "application/json", "true");
     ESP.restart();
  });

  server.on("/adc", [](){
    String response = String(getAdc());
    server.send(200, "application/json", response);
    response = "";
  });

  server.on("/information", [](){
    float distance[3];
    float velocity[3];
    float gravity[3];
    int rotation[3];
    bool overrun[3];
    long lastReadTime;

    imuModule.debugGetValues(distance, velocity, gravity, rotation, overrun, &lastReadTime);

    String response =
      "{ \"freeHeap\": " + String(ESP.getFreeHeap()) +
      ", \"cycleCount\": " + String(ESP.getCycleCount()) +
      ", \"cpuFreqMhz\": " + String(ESP.getCpuFreqMHz()) +
      ", \"chipId\": " + String(ESP.getChipId()) +
      ", \"xDistance\": " + String(distance[0]) +
      ", \"yDistance\": " + String(distance[1]) +
      ", \"zDistance\": " + String(distance[2]) +
      ", \"xVelocity\": " + String(velocity[0]) +
      ", \"yVelocity\": " + String(velocity[1]) +
      ", \"zVelocity\": " + String(velocity[2]) +
      ", \"xGravity\": " + String(gravity[0]) +
      ", \"yGravity\": " + String(gravity[1]) +
      ", \"zGravity\": " + String(gravity[2]) +
      ", \"xRotation\": " + String(rotation[0]) +
      ", \"yRotation\": " + String(rotation[1]) +
      ", \"zRotation\": " + String(rotation[2]) +
      ", \"xOverrun\": " + String(overrun[0]) +
      ", \"yOverrun\": " + String(overrun[1]) +
      ", \"zOverrun\": " + String(overrun[2]) +
      ", \"lastReadTime\": " + String(lastReadTime) +
      "}";
    server.send(200, "application/json", response);
    response = "";
  });

  server.on("/speaker/enable", [](){
    speakerModule.start();
    server.send(200, "application/json", "true");
  });

  server.on("/speaker/disable", [](){
    speakerModule.stop();
    server.send(200, "application/json", "false");
  });

  server.on("/imu/reset", [](){
    imuModule.reset();
    server.send(200, "application/json", "true");
  });

  server.on("/imu/position", [](){
    float xDistance, yDistance, zDistance;
    bool xValid, yValid, zValid;
    imuModule.getPositionDelta(
      &xDistance, &yDistance, &zDistance,
      &xValid, &yValid, &zValid );

    String response =
      "{ \"xDelta\": "  + String(xDistance) +
      ", \"yDelta\": "  + String(yDistance) +
      ", \"zDelta\": "  + String(zDistance) +
      ", \"xValid\": " + (xValid ? "true" : "false") +
      ", \"yValid\": " + (yValid ? "true" : "false") +
      ", \"zValid\": " + (zValid ? "true" : "false") +
      "}";

    server.send(200, "application/json", response);
    response = "";
  });

   httpUpdater.setup(&server, update_path, update_username, update_password);
   server.begin();
}

void initHardware()
{
  pinMode(A0, INPUT);

  pinMode(BOARD_LED_RED, OUTPUT);
}
