// WiFi
#include <ESP8266WebServer.h>

// I2C, Accel
#include <Wire.h>
#include <LIS331.h>

// SPIFF
#include <FS.h>

#include <WifiHelpers.h>
#include <Constants.h>

typedef struct record
  {
      float min_magnitude;  // Gravity needed to trigger this record
      int loop_delay;       // delay(?) for loop()
      int tone_delay;       // delay between the last time tone() was allowed and next
      int tone_duration;    // delay(?) between tone and notone
      int frequency;        // frequency to pass to tone
  } record;

ESP8266WebServer server(80);
LIS331 lis;

const char WiFiAPPSK[] = "sparkfun";

float last_x, last_y, last_z;
float max_x, max_y, max_z;

bool RUNNING = false;

bool TOGGLE = LOW;
unsigned long lastRead = 0;

record* records;
int record_length = 0;

void initHardware();
void initRecords();
void setupWiFi();
void toggleBoardLed();

void setup()
{
  initHardware();
  initRecords();
  setupWiFi();

  // HTTP Server
  server.on("/version", [](){
    server.send(200, "application/json", getVersion());
  });

  server.on("/reset", [](){
    server.send(200, "application/json", getVersion());
    ESP.restart();
  });

  server.on("/cfg", [](){
    String response = "";

    response += "[";
    for(int i=0; i<record_length; i++){
      response += (i == 0) ? "" : ",";

      response += "{ 'min_magnitude': " + String(records[i].min_magnitude);
      response += ", 'loop_delay': " + String(records[i].loop_delay);
      response += ", 'tone_delay': " + String(records[i].tone_delay);
      response += ", 'tone_duration': " + String(records[i].tone_duration);
      response += ", 'frequency': " + String(records[i].frequency);
      response += "}";
    }
    response += "]";

    server.send(200, "application/json", response);
  });

  server.on("/spiffOpen", [](){
    String path;
    String mode;
    String content;

    bool path_valid, mode_valid, content_valid = false;

    String response = "";

    int mode_length = 6;
    String modes[6] = {"r", "w", "a", "r+", "w+", "a+"};

    for(int i=0; i<server.args(); i++){
      if(server.argName(i) == "path"){
        path = server.arg(i);
        path_valid = true;
      }else if(server.argName(i) == "mode"){
        mode = server.arg(i);
      }else if(server.argName(i) == "content"){
        content = server.arg(i);
        content_valid = true;
      }
    }

    for(int i=0; i<mode_length; i++){
      if(mode == modes[i]){
        mode_valid = true;
      }
    }

    if(!path_valid || !mode_valid || !content_valid){
      response += "{ 'error': Invalid parameters";
      response += ", 'path': " + path;
      response += ", 'mode': " + mode;
      response += ", 'content': " + content;
      response += "}";

      server.send(200, "application/json", response);
      return;
    }

    File f = SPIFFS.open(path, mode.c_str());
    if (!f) {
      response += "{ 'error': Invalid file";
      response += ", 'path': " + path;
      response += ", 'mode': " + mode;
      response += ", 'content': " + content;
      response += "}";

      server.send(200, "application/json", response);
      return;
    }

    f.print(content);

    response += "{ 'error': none";
    response += ", 'path': " + path;
    response += ", 'mode': " + mode;
    response += ", 'content': " + content;
    response += "}";

    server.send(200, "application/json", response);
    return;

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

  server.on("/speaker", [](){
    server.send(200, "application/json", getSpeaker(server));
  });

server.begin();

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

      tone(SPEAKER_2, 2200);

      delay(50);

      noTone(SPEAKER_2);
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

void initRecords(){
  File f;
  if(SPIFFS.exists("/puck.cfg") && (f = SPIFFS.open("/puck.cfg", "r"))){
    record_length = f.readStringUntil('\n').toInt();
    records = (record*) malloc(record_length * sizeof(record));
    for(int i=0; i<record_length; i++){
      records[i].min_magnitude = f.parseFloat();
      records[i].loop_delay = f.parseInt();
      records[i].tone_delay = f.parseInt();
      records[i].tone_duration = f.parseInt();
      records[i].frequency = f.parseInt();
    }
  }
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

  pinMode(SPEAKER_2, OUTPUT);
  pinMode(BOARD_LED, OUTPUT);

  // Don't need to set ANALOG_PIN as input

  lis.setPowerStatus(LR_POWER_NORM);
  lis.setGRange(0x30); // 24g

  digitalWrite(SPEAKER_2, HIGH);

  delay(2);

  digitalWrite(SPEAKER_2, LOW);
}
