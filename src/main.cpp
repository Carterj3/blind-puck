// WiFi
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>

// OTA
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// I2C, Accel
#include <Wire.h>
#include <LIS331.h>

#include "FS.h"

#include <WifiHelpers.h>
#include <Constants.h>

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;

LIS331 lis;

const char* host = "esp8266-webupdate";
const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";
const char WiFiAPPSK[] = "sparkfun";

float last_x, last_y, last_z;
float max_x, max_y, max_z;

bool lisX, lisY, lisZ;
bool RUNNING = false;

bool TOGGLE = LOW;
unsigned long lastRead = 0;

void initHardware();
void setupWiFi();
void toggleBoardLed();

void setup()
{
  initHardware();
  setupWiFi();


  // HTTP Server
  server.on("/version", [](){
    server.send(200, "application/json", getVersion());
  });

  server.on("/spiff", [](){
    FSInfo fs_info;
    SPIFFS.info(fs_info);

    String response = "";
    response += "{ totalBytes: " + String(fs_info.totalBytes);
    response += ", usedBytes : " + String(fs_info.usedBytes);
    response += ", blockSize : " + String(fs_info.blockSize);
    response += ", pageSizee : " + String(fs_info.pageSize);
    response += ", maxOpenFiles: " + String(fs_info.maxOpenFiles);
    response += ", maxPathLength: " + String(fs_info.maxPathLength);

    server.send(200, "application/json", response);
  });

  server.on("/enable", [](){
    RUNNING = true;
    server.send(200, "application/json", "true");
  });

  server.on("/disable", [](){
    RUNNING = false;
    server.send(200, "application/json", "false");
  });

  server.on("/adc", [](){
    server.send(200, "application/json", getAdc());
  });

  server.on("/accel", [](){
    server.send(200, "application/json", getAccel(lis));
  });

  server.on("/accelMax", [](){
    String response = "";
    response += "{ max_x: " + String(max_x);
    response += ", last_x: " + String(last_x);
    response += ", max_y: " + String(max_y);
    response += ", last_y: " + String(last_y);
    response += ", max_z: " + String(max_z);
    response += ", last_z: " + String(last_z);
    response += " magnitude_last: " + String(computeMagnitude3d(last_x, last_y, last_z));
    response += " magnitude_max: " + String(computeMagnitude3d(max_x, max_y, max_z));
    response += "}";

    server.send(200, "application/json", response);
  });

  server.on("/lisReset", [](){
    server.send(200, "application/json", resetLis331(lis, lisX, lisY, lisZ));
  });

  server.on("/lis", [](){
    server.send(200, "application/json", getLis331(lis, lisX, lisY, lisZ));
  });

  server.on("/speaker", [](){
    server.send(200, "application/json", getSpeaker(server));
  });

  MDNS.begin(host);

  httpUpdater.setup(&server, update_path, update_username, update_password);

  server.begin();
  MDNS.addService("http", "tcp", 80);

  delay(2000);
}

void toggleBoardLed(){
  // millis() This number will overflow (go back to zero), after approximately 50 days.
  if(millis() < lastRead){
    lastRead = millis();
  }

  if(millis() > lastRead + 200){
    TOGGLE = !TOGGLE;
    digitalWrite(BOARD_LED, TOGGLE);
    lastRead = millis();


    if(RUNNING){
      float magnitude = computeMagnitude3d(last_x, last_y, last_z);

      tone(SPEAKER_1, 2200);
      tone(SPEAKER_2, 2200);

      delay(50);

      noTone(SPEAKER_1);
      noTone(SPEAKER_2);
    }
  }
}

void loop()
{
  int16_t x,y,z;
  toggleBoardLed();

  server.handleClient();
  ArduinoOTA.handle();

  if(lis.statusHasZDataAvailable() && lis.getZValue(&z)){
    last_z = float(z)*G_SCALE;
    max_z = max(abs(last_z), abs(max_z)) ;
  }
  if(lis.statusHasXDataAvailable() && lis.getXValue(&x)){
    last_x = float(x)*G_SCALE;
    max_x = max(abs(last_x), abs(max_x));
  }
  if(lis.statusHasYDataAvailable() && lis.getYValue(&y)){
    last_y = float(y)*G_SCALE;
    max_y = max(abs(last_y), abs(max_y));
  }

  delay(100);

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
  String AP_NameString = "ESP8266Thing" + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  WiFi.softAP(AP_NameChar, WiFiAPPSK);
}

void initHardware()
{
  Serial.begin(115200);
  Wire.begin();
  SPIFFS.begin();

  pinMode(SPEAKER_1, OUTPUT);
  pinMode(SPEAKER_2, OUTPUT);
  pinMode(BOARD_LED, OUTPUT);

  // Don't need to set ANALOG_PIN as input

  lis.setPowerStatus(LR_POWER_NORM);
  lisX = lis.setXEnable(true);
  lisY = lis.setYEnable(true);
  lisZ = lis.setZEnable(true);
  lis.setGRange(0x30); // 24g


  digitalWrite(SPEAKER_1, HIGH);
  digitalWrite(SPEAKER_2, HIGH);

  delay(2);

  digitalWrite(SPEAKER_1, LOW);
  digitalWrite(SPEAKER_2, LOW);
}
