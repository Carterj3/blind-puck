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

ADC_MODE(ADC_VCC); // get VCC on ADC

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

bool RUNNING = true;
int COUNTER = 0;

bool TOGGLE = LOW;
unsigned long lastRead = 0;

void initHardware();
void setupWiFi();
void toggleBoardLed();

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Booting Sketch...");


  initHardware();
  setupWiFi();

  // HTTP Server
  server.on("/version", [](){
    server.send(200, "application/json", getVersion());
  });

  server.on("/restart", [](){
     server.send(200, "application/json", String(ESP.getVcc()));
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

   httpUpdater.setup(&server, update_path, update_username, update_password);
   server.begin();


  MDNS.begin(host);

   MDNS.addService("http", "tcp", 80);
   Serial.printf("HTTPUpdateServer ready! Open http://%s.local%s in your browser and login with username '%s' and password '%s'\n", host, update_path, update_username, update_password);

  delay(2000);
}

void toggleBoardLed(){
  // millis() This number will overflow (go back to zero), after approximately 50 days.
  if(millis() < lastRead){
    lastRead = millis();
  }

  if(millis() > lastRead + 50){
    TOGGLE = !TOGGLE;
    digitalWrite(BOARD_LED_RED, TOGGLE);
    digitalWrite(BOARD_LED_BLUE, RUNNING && TOGGLE);
    lastRead = millis();

    if(COUNTER < 100){
      COUNTER += 1;
    }else if(RUNNING){
      float magnitude = computeMagnitude3d(last_x, last_y, last_z);

      tone(SPEAKER_1, 300 + COUNTER);
      COUNTER = 5 + (COUNTER % 20000);

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
  String AP_NameString = "_Puck_" + macID;

  char AP_NameChar[AP_NameString.length() + 1];
  memset(AP_NameChar, 0, AP_NameString.length() + 1);

  for (int i=0; i<AP_NameString.length(); i++)
    AP_NameChar[i] = AP_NameString.charAt(i);

  WiFi.softAP(AP_NameChar, WiFiAPPSK);
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


  pinMode(BOARD_LED_RED, OUTPUT);
  pinMode(BOARD_LED_BLUE, OUTPUT);

  // Don't need to set ANALOG_PIN as input

  lis.setPowerStatus(LR_POWER_NORM);
  lis.setGRange(0x30); // 24g

}
