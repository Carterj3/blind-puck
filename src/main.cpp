// WiFi
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>

// I2C, Accel
#include <Wire.h>
#include <LIS331.h>

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

bool RUNNING = false;
unsigned int pitch = 2200;

bool TOGGLE = LOW;
unsigned long lastRead = 0;

bool SLEEP_WIFI_OFF = false;
unsigned long sleepStart = 0;
unsigned long sleepEnd = 0;

void initHardware();
void setupWiFi();
void setupServer();

void toggleBoardLed();

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting Sketch...");

  initHardware();
  setupWiFi();
  setupServer();

  delay(2000);
}

void toggleBoardLed(){
  // millis() This number will overflow (go back to zero), after approximately 50 days.
  if(millis() < lastRead){
    lastRead = millis();
  }

  if(millis() > lastRead + 500){
    TOGGLE = !TOGGLE;
    digitalWrite(BOARD_LED_RED, TOGGLE);
    digitalWrite(BOARD_LED_BLUE, RUNNING && TOGGLE);
    lastRead = millis();

    if(RUNNING){
      float magnitude = computeMagnitude3d(last_x, last_y, last_z);

      tone(SPEAKER_1, pitch);

      delay(50);

      noTone(SPEAKER_1);
    }
  }
}

void loop()
{
  int16_t x,y,z;
  toggleBoardLed();
  server.handleClient();


  if(SLEEP_WIFI_OFF && (millis() > sleepEnd)){
    setupWiFi();
    setupServer();
  }

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
  SLEEP_WIFI_OFF = false;
  WiFi.mode(WIFI_AP);

  // Do a little work to get a unique-ish name. Append the
  // last two bytes of the MAC (HEX'd) to "Thing-":
  uint8_t mac[WL_MAC_ADDR_LENGTH];
  WiFi.softAPmacAddress(mac);
  String macID = String(mac[WL_MAC_ADDR_LENGTH - 2], HEX) +
                 String(mac[WL_MAC_ADDR_LENGTH - 1], HEX);
  macID.toUpperCase();
  String AP_NameString = "_P_v1_" + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  WiFi.softAP(AP_NameChar, WiFiAPPSK);

  WiFi.setOutputPower(0);
  WiFi.begin(AP_NameChar,  WiFiAPPSK);
}

void setupServer()
{
  // HTTP Server
  server.on("/version", [](){
    server.send(200, "application/json", getVersion());
  });

  server.on("/restart", [](){
     server.send(200, "application/json", getAdc());
     ESP.restart();
  });

  server.on("/enable", [](){
    RUNNING = true;
    server.send(200, "application/json", "true");
  });

  server.on("/disable", [](){
    RUNNING = false;
    server.send(200, "application/json", "false");
  });

  server.on("/pitch", [](){
    String response = "";

    if(! server.hasArg("pitch")){
      response += "{";
      response += " pitch: ";
      response += String(pitch);
      response += "}";

      server.send(200, "application/json", response);
      return;
    }

    String newPitch_String = server.arg("pitch");
    int newPitch = newPitch_String.toInt();

    if(newPitch < 0 || newPitch > 22000){
      response += "{";
      response += " newPitch: " + String(pitch);
      response += " pitch: " + String(newPitch);
      response += "}";

      server.send(200, "application/json", response);
    }

    response += "{";
    response += " newPitch: " + String(pitch);
    response += " oldPitch: " + String(newPitch);
    response += "}";

    pitch = newPitch;

    server.send(200, "application/json", response);
  });

  server.on("/disableWifi", [](){
    server.send(200, "application/json", "wifi off");

    sleepStart = millis();
    sleepEnd = 60*1000 + millis();

    server.stop();
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    delay(1);

    SLEEP_WIFI_OFF = true;
  });

  server.on("/adc", [](){
    server.send(200, "application/json", getAdc());
  });

  server.on("/life", [](){
    server.send(200, "application/json", getTimeLeft());
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

   httpUpdater.setup(&server, update_path, update_username, update_password);
   server.begin();


   MDNS.begin(host);

   MDNS.addService("http", "tcp", 80);
   Serial.printf("HTTPUpdateServer ready! Open http://%s.local%s in your browser and login with username '%s' and password '%s'\n", host, update_path, update_username, update_password);
}

void initHardware()
{
  //  Wire.begin(int sda, int scl)
  Wire.begin(4, 5);

  pinMode(SPEAKER_1, OUTPUT);
  pinMode(SPEAKER_2, OUTPUT);
  pinMode(SPEAKER_3, OUTPUT);
  pinMode(SPEAKER_4, OUTPUT);
  pinMode(SPEAKER_5, OUTPUT);

  pinMode(A0, INPUT);

  pinMode(BOARD_LED_RED, OUTPUT);
  pinMode(BOARD_LED_BLUE, OUTPUT);

  // Don't need to set ANALOG_PIN as input

  lis.setPowerStatus(LR_POWER_NORM);
  lis.setGRange(0x30); // 24g

}
