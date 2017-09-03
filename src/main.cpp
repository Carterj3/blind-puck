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

struct accel_data
{
  unsigned long time;
  float x;
  float y;
  float z;
};
typedef struct accel_data Accel_data;


ESP8266WebServer server(80);
ESP8266HTTPUpdateServer httpUpdater;
LIS331 lis;

const char* host = "esp8266-webupdate";
const char* update_path = "/firmware";
const char* update_username = "admin";
const char* update_password = "admin";

const char WiFiAPPSK[] = "sparkfun";

int ACCEL_CURRENT_INDEX = -1;
bool ACCEL_OVERRUN = false;
int ACCEL_ROW_STARTED = 0;
const int ACCEL_RATE_MS = 200;
const int ACCEL_MAX_INDEX = 1 * 60 * (1000 / ACCEL_RATE_MS);
Accel_data ACCEL_ARRAY[ACCEL_MAX_INDEX];
Accel_data ACCEL_MAX;


bool LED_TOGGLE = LOW;
bool LED_LAST_TOGGLED = 0;
const bool LED_TOGGLE_RATE_MS = 450;

bool SPEAKER_RUNNING = false;
bool SPEAKER_TOGGLE = LOW;
bool SPEAKER_LAST_TOGGLED = 0;
const bool SPEAKER_OFF_DURATION_MS = 400;
const bool SPEAKER_ON_DURATION_MS = 100;

void initHardware();
void setupWiFi();
void setupServer();

void toggleBoardLed();
void toggleSpeaker();
void handleAccelData();
void incrementAccelIndex();

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
  if(millis() < LED_LAST_TOGGLED){
    LED_LAST_TOGGLED = millis();
  }

  if(millis() > LED_LAST_TOGGLED + LED_TOGGLE_RATE_MS){
    LED_TOGGLE = !LED_TOGGLE;
    digitalWrite(BOARD_LED_RED, LED_TOGGLE);
    digitalWrite(BOARD_LED_BLUE, SPEAKER_RUNNING && LED_TOGGLE);
    LED_LAST_TOGGLED = millis();
  }
}

void toggleSpeaker(){
  if(millis() < SPEAKER_LAST_TOGGLED){
    SPEAKER_LAST_TOGGLED = millis();
    SPEAKER_TOGGLE = LOW;
    digitalWrite(SPEAKER_1, LOW);
  }

  if(SPEAKER_TOGGLE && millis() > SPEAKER_LAST_TOGGLED + SPEAKER_ON_DURATION_MS){
    SPEAKER_LAST_TOGGLED = millis();
    SPEAKER_TOGGLE = LOW;
    digitalWrite(SPEAKER_1, SPEAKER_TOGGLE);
  }

  if(millis() > SPEAKER_LAST_TOGGLED + SPEAKER_OFF_DURATION_MS){
    SPEAKER_LAST_TOGGLED = millis();
    SPEAKER_TOGGLE = SPEAKER_RUNNING && HIGH;
    digitalWrite(SPEAKER_1, SPEAKER_TOGGLE);
  }
}

void incrementAccelIndex(){
  ACCEL_CURRENT_INDEX = (ACCEL_CURRENT_INDEX + 1);
  if(ACCEL_CURRENT_INDEX >= ACCEL_MAX_INDEX)
  {
    ACCEL_OVERRUN = true;
    ACCEL_CURRENT_INDEX = 0;
  }
}

void handleAccelData(){
    int16_t x,y,z;

    if(millis() < ACCEL_ROW_STARTED){
      ACCEL_ROW_STARTED = millis();
      incrementAccelIndex();
    }

  if(   lis.statusHasZDataAvailable()
    &&  lis.statusHasXDataAvailable()
    &&  lis.statusHasYDataAvailable()

    &&  lis.getZValue(&z)
    &&  lis.getXValue(&x)
    &&  lis.getYValue(&y))
  {

    if(millis() > ACCEL_ROW_STARTED + ACCEL_RATE_MS){
      ACCEL_ROW_STARTED = millis();
      incrementAccelIndex();

      ACCEL_ARRAY[ACCEL_CURRENT_INDEX].time = millis();
      ACCEL_ARRAY[ACCEL_CURRENT_INDEX].x = float(x)*G_SCALE;
      ACCEL_ARRAY[ACCEL_CURRENT_INDEX].y = float(y)*G_SCALE;
      ACCEL_ARRAY[ACCEL_CURRENT_INDEX].z = float(z)*G_SCALE;
    }else{

      ACCEL_ARRAY[ACCEL_CURRENT_INDEX].x += float(x)*G_SCALE;
      ACCEL_ARRAY[ACCEL_CURRENT_INDEX].y += float(y)*G_SCALE;
      ACCEL_ARRAY[ACCEL_CURRENT_INDEX].z += float(z)*G_SCALE;
    }

    ACCEL_MAX.x = max(abs(float(x)*G_SCALE), abs(ACCEL_MAX.x));
    ACCEL_MAX.y = max(abs(float(y)*G_SCALE), abs(ACCEL_MAX.y));
    ACCEL_MAX.z = max(abs(float(z)*G_SCALE), abs(ACCEL_MAX.z));
  }
}

void loop()
{

  toggleBoardLed();
  toggleSpeaker();
  handleAccelData();

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
  String AP_NameString = "_P_v2_" + macID;

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
    server.send(200, "application/json", getVersion());
  });

  server.on("/restart", [](){
     server.send(200, "application/json", getTimeLeft());
     ESP.restart();
  });

  server.on("/enable", [](){
    SPEAKER_RUNNING = true;
    server.send(200, "application/json", "true");
  });

  server.on("/disable", [](){
    SPEAKER_RUNNING = false;
    server.send(200, "application/json", "false");
  });

  server.on("/adc", [](){
    server.send(200, "application/json", String(getAdc()));
  });

  server.on("/life", [](){
    server.send(200, "application/json", getTimeLeft());
  });

  server.on("/accelDump", [](){
    String response = "[";
    int lastIndex = ACCEL_OVERRUN ? ACCEL_MAX_INDEX : ACCEL_CURRENT_INDEX;

    for(int i=0; i < lastIndex; i++){
      response += "{ \"time\": " + String(ACCEL_ARRAY[i].time);;
      response += ", \"x\": " + String(ACCEL_ARRAY[i].x);;
      response += ", \"y\": " + String(ACCEL_ARRAY[i].y);;
      response += ", \"z\": " + String(ACCEL_ARRAY[i].z);;

      if( (i+1) == lastIndex ){
        response += "}";
      }else{
        response += "},";
      }
    }
    response += "]";

    ACCEL_OVERRUN = false;
    ACCEL_ROW_STARTED = 0;
    ACCEL_CURRENT_INDEX = -1;

    server.send(200, "application/json", response);
  });

  server.on("/accelMax", [](){
    String response = "";
    response += "{ \"max_x\": " + String(ACCEL_MAX.x);
    response += ", \"last_x\": " + String(ACCEL_ARRAY[ACCEL_CURRENT_INDEX].x);
    response += ", \"max_y\": " + String(ACCEL_MAX.y);
    response += ", \"last_y\": " + String(ACCEL_ARRAY[ACCEL_CURRENT_INDEX].y);
    response += ", \"max_z\": " + String(ACCEL_MAX.z);
    response += ", \"last_z\": " + String(ACCEL_ARRAY[ACCEL_CURRENT_INDEX].z);
    response += ", \"magnitude_last\": " + String(computeMagnitude3d(ACCEL_ARRAY[ACCEL_CURRENT_INDEX].x, ACCEL_ARRAY[ACCEL_CURRENT_INDEX].y, ACCEL_ARRAY[ACCEL_CURRENT_INDEX].z));
    response += ", \"magnitude_max\": " + String(computeMagnitude3d(ACCEL_MAX.x, ACCEL_MAX.y, ACCEL_MAX.z));
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
