// WiFi
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>

// OTA
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

// I2C, Accel
#include <Wire.h>
#include <LIS331.h>

#include <WifiHelpers.h>
#include <Constants.h>

ESP8266WebServer server(80);
LIS331 lis;

bool lisX, lisY, lisZ;

bool TOGGLE = LOW;
unsigned long lastRead = 0;

void initHardware();
void setupWiFi();

void setup()
{
  initHardware();
  setupWiFi();


  // HTTP Server
  server.begin();

  server.on("/version", [](){
    server.send(200, "application/json", getVersion());
  });

  server.on("/adc", [](){
    server.send(200, "application/json", getAdc());
  });

  server.on("/accel", [](){
    server.send(200, "application/json", getAccel(lis));
  });

  server.on("/lis", [](){
    server.send(200, "application/json", getLis331(lis, lisX, lisY, lisZ));
  });

  server.on("/speaker", [](){
    server.send(200, "application/json", getSpeaker(server));
  });

  server.begin();

  // OTA? Server
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  // ArduinoOTA.setHostname("myesp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
    digitalWrite(SPEAKER_1, HIGH);
    digitalWrite(SPEAKER_2, HIGH);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
    digitalWrite(SPEAKER_1, LOW);
    digitalWrite(SPEAKER_2, LOW);
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
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
  }
}

void loop()
{
  toggleBoardLed();

  server.handleClient();
  ArduinoOTA.handle();

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
  String AP_NameString = "ESP8266 Thing " + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  WiFi.softAP(AP_NameChar, WiFiAPPSK);
}

void initHardware()
{
  Serial.begin(115200);

  pinMode(SPEAKER_1, OUTPUT);
  pinMode(SPEAKER_2, OUTPUT);
  pinMode(BOARD_LED, OUTPUT);

  // Don't need to set ANALOG_PIN as input

  lis.setPowerStatus(LR_POWER_NORM);
  lisX = lis.setXEnable(true);
  lisY = lis.setYEnable(true);
  lisZ = lis.setZEnable(true);


  digitalWrite(SPEAKER_1, HIGH);
  digitalWrite(SPEAKER_2, HIGH);

  delay(2);

  digitalWrite(SPEAKER_1, LOW);
  digitalWrite(SPEAKER_2, LOW);
}
