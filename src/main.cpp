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

int accel_index = -1;
bool accel_overrun = false;
const int accel_max_index = 1*60*5;
Accel_data accel_array[accel_max_index];
Accel_data accel_max;

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

  if(millis() > lastRead + 250){
    TOGGLE = !TOGGLE;
    digitalWrite(BOARD_LED_RED, TOGGLE);
    digitalWrite(BOARD_LED_BLUE, RUNNING && TOGGLE);
    lastRead = millis();

    if(RUNNING){

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

  if(   lis.statusHasZDataAvailable()
    &&  lis.statusHasXDataAvailable()
    &&  lis.statusHasYDataAvailable()

    &&  lis.getZValue(&z)
    &&  lis.getXValue(&x)
    &&  lis.getYValue(&y))
  {
    accel_index = (accel_index + 1);
    if(accel_index >= accel_max_index)
    {
      accel_overrun = true;
      accel_index = 0;
    }

    accel_array[accel_index].time = millis();
    accel_array[accel_index].x = float(x)*G_SCALE;
    accel_array[accel_index].y = float(y)*G_SCALE;
    accel_array[accel_index].z = float(z)*G_SCALE;

    accel_max.x = max(abs(accel_array[accel_index].x), abs(accel_max.x));
    accel_max.y = max(abs(accel_array[accel_index].y), abs(accel_max.y));
    accel_max.z = max(abs(accel_array[accel_index].z), abs(accel_max.z));
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
      response += "{ \"pitch\": " +  String(pitch);
      response += "}";

      server.send(200, "application/json", response);
      return;
    }

    String newPitch_String = server.arg("pitch");
    int newPitch = newPitch_String.toInt();

    if(newPitch < 0 || newPitch > 22000){
      response += "{ \"newPitch\": " + String(pitch);
      response += ", \"pitch\": " + String(newPitch);
      response += "}";

      server.send(200, "application/json", response);
    }

    response += "{ \"newPitch\": " + String(newPitch);
    response += ", \"oldPitch\": " + String(pitch);
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

  server.on("/accelDump", [](){
    String response = "[";
    int lastIndex = accel_overrun ? accel_max_index : accel_index;

    for(int i=0; i < lastIndex; i++){
      response += "{ \"time\": " + String(accel_array[i].time);;
      response += ", \"x\": " + String(accel_array[i].x);;
      response += ", \"y\": " + String(accel_array[i].y);;
      response += ", \"z\": " + String(accel_array[i].z);;

      if( (i+1) == lastIndex ){
        response += "}";
      }else{
        response += "},";
      }
    }
    response += "]";

    accel_overrun = true;
    accel_index = -1;

    server.send(200, "application/json", response);
  });

  server.on("/accelMax", [](){
    String response = "";
    response += "{ \"max_x\": " + String(accel_max.x);
    response += ", \"last_x\": " + String(accel_array[accel_index].x);
    response += ", \"max_y\": " + String(accel_max.y);
    response += ", \"last_y\": " + String(accel_array[accel_index].y);
    response += ", \"max_z\": " + String(accel_max.z);
    response += ", \"last_z\": " + String(accel_array[accel_index].z);
    response += ", \"magnitude_last\": " + String(computeMagnitude3d(accel_array[accel_index].x, accel_array[accel_index].y, accel_array[accel_index].z));
    response += ", \"magnitude_max\": " + String(computeMagnitude3d(accel_max.x, accel_max.y, accel_max.z));
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
