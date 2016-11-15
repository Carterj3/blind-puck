// WiFi
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>

// I2C
#include <Wire.h>
// Accel
#include <LIS331.h>

// OTA
#include <WiFiUdp.h>
#include <ArduinoOTA.h>


#include <WifiHelpers.h>

const char WiFiAPPSK[] = "sparkfun";

/////////////////////
// Pin Definitions //
/////////////////////

bool TOGGLE = LOW;
unsigned long lastRead = 0;
const int BOARD_LED = 5; // 5 -> ESP8266 #5

// GPIO for speaker
const int SPEAKER_1 = 0; // 0 -> ESP8266 #0
const int SPEAKER_2 = 4; // 4 -> ESP8266 #4

// I2C for accel
LIS331 lis;

// ADC for battery voltage
const int ANALOG_PIN = A0;

// ESP8266WebServer server(80);
WiFiServer server(80);

int16_t x,y,z;

void initHardware();
void setupWiFi();

void setup()
{
  initHardware();
  setupWiFi();

  server.begin();

  //
  // server.on("/version", [](){
  //   server.send(200, "application/json", "0.1.1.13");
  // });
  //
  // server.on("/adc", [](){
  //   int average = 0;
  //   for(int i=1; i <= 1000; i++){
  //     // int average(int rollingAverage, int index, int newValue, int scale);
  //     average = computeRollingAverage(average, i, analogRead(ANALOG_PIN), 10);
  //     // average = (average*(i-1))/i + (10*analogRead(ANALOG_PIN))/i;
  //   }
  //
  //   server.send(200, "application/json", String(average/10));
  // });
  //
  // server.on("/accel", [](){
  //
  //   lis.getXValue(&x);
  //   lis.getYValue(&y);
  //   lis.getZValue(&z);
  //
  //   String response = "";
  //   response += "{ x: "+String(x);
  //   response += ", y: "+String(y);
  //   response += ", z: "+String(z);
  //   response += "}";
  //
  //   server.send(200, "application/json", response);
  // });
  //
  // server.on("/speaker", [](){
  //
  //   int frequency = 0;
  //   int delay = 0;
  //   int repeats = 0;
  //   String useTone = "true";
  //
  //   for(int i=0; i<server.args(); i++){
  //     if(server.argName(i) == "delay"){
  //       delay = server.arg(i).toInt();
  //     }else if(server.argName(i) == "repeats"){
  //       repeats = server.arg(i).toInt();
  //     }else if(server.argName(i) == "frequency"){
  //       frequency = server.arg(i).toInt();
  //     }else if(server.argName(i) == "tone"){
  //       useTone = server.arg(i);
  //     }
  //   }
  //
  //   if(useTone == "true"){
  //     for(int i=0; i<repeats; i++){
  //
  //       delayMicroseconds(delay/2);
  //
  //       tone(SPEAKER_1, frequency);
  //       tone(SPEAKER_2, frequency);
  //
  //       delayMicroseconds(delay/2);
  //
  //       noTone(SPEAKER_1);
  //       noTone(SPEAKER_2);
  //     }
  //   }else{
  //     for(int i=0; i<repeats; i++){
  //
  //       delayMicroseconds(delay/2);
  //
  //       digitalWrite(SPEAKER_1, HIGH);
  //       digitalWrite(SPEAKER_2, HIGH);
  //
  //       delayMicroseconds(delay/2);
  //
  //       digitalWrite(SPEAKER_1, LOW);
  //       digitalWrite(SPEAKER_2, LOW);
  //     }
  //   }
  //
  //   String response = "";
  //   response += "{ repeats: "+String(repeats);
  //   response += ", delay: "+String(delay);
  //   response += ", frequency: "+String(frequency);
  //   response += ", tone: "+String(useTone == "true");
  //
  //   response += "}";
  //
  //
  //   server.send(200, "application/json", response);
  // });
  //
  // server.begin();
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

  WiFiClient client = server.available();
  if (client) {
    handleWiFiClient(client); // NOTE: this is a blocking call but should resolve quickly
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

  // lis.setPowerStatus(LR_POWER_NORM);
  // lis.setXEnable(true);
  // lis.setYEnable(true);
  // lis.setZEnable(true);


  digitalWrite(SPEAKER_1, HIGH);
  digitalWrite(SPEAKER_2, HIGH);

  delay(2);

  digitalWrite(SPEAKER_1, LOW);
  digitalWrite(SPEAKER_2, LOW);
}
