// Firmware to control my ceiling fan from webpage, alexa and by ntp time.
// Author Brock Mills

#include <AsyncTCP.h>
#include <OneWire.h>
#include <RCSwitch.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <ESP32Ping.h>
#include <SPIFFS.h>
#include "time.h"
#include "wifi_credentials.h"

#define red_led 26
#define white_led 23
#define green_led 32
#define transmitterLed 22
#define transmitterPin 25
#define buttonPin 34
#define temperatureSensor 33

unsigned long previousTimes[8] = {0};
uint8_t fanlightOnTime[3] = {18, 00, 0};
uint8_t fanOnTime[3] = {20, 30, 0};
uint8_t fanlightOffTime[3] = {07, 50, 0};
unsigned long days = 1;
bool blinkOn = false;
String message = "FANCODE_OFF";

bool lightState = false;
bool ktState = false;

RCSwitch mySwitch = RCSwitch();
OneWire ts(19);
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

IPAddress local_IP(192, 168, 68, 25);
IPAddress gateway(192, 168, 68, 1);
IPAddress subnet(255, 255, 252, 0);
IPAddress DNS(202, 142, 142, 142);

void Wifi_disconnected(WiFiEvent_t event, WiFiEventInfo_t info) {
  digitalWrite(red_led, HIGH);
  digitalWrite(green_led, LOW);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void Wifi_connected(WiFiEvent_t event, WiFiEventInfo_t info) {
  digitalWrite(red_led, LOW);
  digitalWrite(green_led, HIGH);
}

int outputTemp() {
  byte tempData[12] = {};
  byte addr[8] = {0x28, 0x6A, 0x32, 0x8E, 0x81, 0x22, 0x0B, 0x60};
  ts.reset();
  ts.select(addr);
  //ts.write(0x4E);
  ts.write(0x00);
  ts.write(0x00);
  ts.reset();
  ts.select(addr);
  ts.write(0x44, 1);
  ts.reset();
  ts.select(addr);
  ts.write(0xBE);
  for (int x = 0; x < 9; x++) {
    tempData[x] = ts.read();
  }

  byte MSB = tempData[1];
  byte LSB = tempData[0];
  float tempred = ((MSB << 8) | LSB);
  int TemperatureSum = tempred / 16;
  if (TemperatureSum > 0) {
    return TemperatureSum;
  }

}


void onEventWS(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  switch (type) {
    case WS_EVT_CONNECT:
      previousTimes[0] = millis();
      blinkOn = true;
      ws.textAll("TEM" + String(outputTemp()));
      ws.textAll("LS" + String(lightState));
      ws.textAll("DAY" + String(days));
      ws.textAll(message);
      break;
    case WS_EVT_DISCONNECT:
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_ERROR:
      break;
  }
}

void initWebsocket() {
  ws.onEvent(onEventWS);
  server.addHandler(&ws);
}


void webpageHandling() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    //    if(!request->authenticate(HTTP_USER, HTTP_PASS)){
    //        return request->requestAuthentication();
    //
    //      }
    request->send(SPIFFS, "/index.html", String(), false);
  });
  server.on("/css.css", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/css.css", "text/css");
  });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/script.js", "text/javascript");
  });

  server.on("/bulb-off", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/bulb-off.png", "image/png");
  });

  server.on("/bulb-on", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/bulb-on.png", "image/png");
  });

  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest * request){
    request->send(SPIFFS, "/favicon.png", "image/png");
  });

  server.begin();
}

void transmitFanCode(const char* code) {
#define FAN_HIGH "000001011111"
#define FAN_MED "000001101111"
#define FAN_LOW "000001110111"
#define FAN_OFF "000001111101"
#define CHANGE_LIGHT_COLOR "000001110110"
#define FANLIGHT_ON_OFF "000001111110"
  digitalWrite(transmitterLed, HIGH);
  mySwitch.send(code);
  digitalWrite(transmitterPin, LOW);
  digitalWrite(transmitterLed, LOW);

}


void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info -> opcode == WS_TEXT) {
    data[len] = 0;
    message = (char*)data;
    if (strcmp(message.c_str(), "FANCODE_MED") == 0) {
      transmitFanCode(FAN_MED);
      ws.textAll(message);
    }
    if (strcmp(message.c_str(), "FANCODE_LOW") == 0) {
      transmitFanCode(FAN_LOW);
      ws.textAll(message);
    }
    if (strcmp(message.c_str(), "FANCODE_OFF") == 0) {
      transmitFanCode(FAN_OFF);
      ws.textAll(message);
    }
    if (strcmp(message.c_str(), "FANCODE_HIGH") == 0) {
      transmitFanCode(FAN_HIGH);
      ws.textAll(message);
    }
    if (strcmp(message.c_str(), "FANCODE_CHANGECOLOR") == 0) {
      transmitFanCode(CHANGE_LIGHT_COLOR);
      ws.textAll(message);
    }
    if (strcmp(message.c_str(), "FANCODE_LIGHTONOFF") == 0) {
      lightState = ! lightState;
      transmitFanCode(FANLIGHT_ON_OFF);
      ws.textAll(message);

    }
  }
}

bool chkTime(int thr, int tmin, int tsec) {
  time_t n;
  struct tm * timeinfo;
  time(&n);
  timeinfo = localtime(&n);
  int int_hr = timeinfo->tm_hour;
  int int_min = timeinfo->tm_min;
  int int_sec = timeinfo->tm_sec;
  if (int_hr == thr && int_min == tmin && int_sec == tsec) {
    return true;
  } else {
    return false;
  }
}

void setNTPzone() {
  configTime(0, 0, "pool.ntp.org");
  setenv("TZ", "AEST-10", 1);
  tzset();
}

void blinkWebConnectionLed() {
  if (blinkOn) {
    digitalWrite(white_led, HIGH);
    if (millis() - previousTimes[0] >= 1000) {
      digitalWrite(white_led, LOW);
      blinkOn = false;
    }
  }
}

String readFile(fs::FS &fs, const char* path) {
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path);
  if (!file || file.isDirectory()) {
    Serial.println("FAILED TO OPEN FILE TO READ");
    return String();
  }
  String fileContent;
  while (file.available()) {
    fileContent = file.readStringUntil('\n');
    break;
  }

  return fileContent;
}

void writeFile(fs::FS &fs, const char * path, const char * message) {
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("ERROR: FAILED TO OPEN FILE FOR WRITING");
    return;
  }
  if (file.print(message)) {
    Serial.println("FILE WRITTEN");
  } else {
    Serial.println("WRITE FAILED!");
  }
}

void serialConfig() {
  bool stringComplete = false;
  bool loopStatus = true;
  String text = "";
  pinMode(transmitterLed, OUTPUT);
  if (digitalRead(buttonPin) == 1) {
    Serial.println("Going into programming mode!");
    if (!SPIFFS.begin(true)) {
      Serial.println("Error occured while trying to mount spiffs :(");
    }
    Serial.println("Sucessfully mounted spiffs");
    while (loopStatus) {
      digitalWrite(transmitterLed, millis() % 500 > 250);

      if (Serial.available()) {
        char inChar = (char)Serial.read();
        text += inChar;
        if (inChar == '\n') {
          //state = true;
          stringComplete = true;
        }
      }

      if (stringComplete) {
        text.trim();
        if (text.equals("#EXIT")) {
          loopStatus = false;
          Serial.println("Exiting programming mode");
          delay(1000);
        }
        if (text.equals("#SSID_")) {
          Serial.println("Testing ssid thing");
        }
        text = "";
        stringComplete = false;
      }
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT);
  //serialConfig();
  WiFi.disconnect(true);
  WiFi.onEvent(Wifi_connected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(Wifi_disconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
  if (!WiFi.config(local_IP, gateway, subnet, DNS)) {
    Serial.println("STA failed to configure");
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  if (!SPIFFS.begin(true)) {
    Serial.println("Error occured while trying to mount spiffs :(");
  }
  mySwitch.enableTransmit(transmitterPin);
  mySwitch.setProtocol(6);
  mySwitch.setPulseLength(328);
  pinMode(red_led, OUTPUT);
  pinMode(green_led, OUTPUT);
  pinMode(white_led, OUTPUT);
  pinMode(transmitterLed, OUTPUT);
  pinMode(transmitterPin, OUTPUT);

  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(red_led, millis() % 1000 > 500);
  }
  setNTPzone();
  transmitFanCode(FAN_OFF); // If the fan is on turn it off for the webpage to handle the button selection better or it will be messed up
  webpageHandling();
  initWebsocket();
}

void fanTemp(byte degree) {
#define TEMPFANHIGH  0
#define TEMPFANLOW 1
  static bool tempFanState[2] = {true};
  if (outputTemp() >= degree) {
    if (tempFanState[TEMPFANHIGH]) {
      transmitFanCode(FAN_MED);
      tempFanState[TEMPFANHIGH] = false;
      tempFanState[TEMPFANLOW] = true;
    }

  }

  else {
    if (tempFanState[TEMPFANLOW]) {
      //transmitFanCode(FAN_OFF);
      tempFanState[TEMPFANHIGH] = true;
      tempFanState[TEMPFANLOW] = false;
    }
  }
}

void loop() {
  blinkWebConnectionLed();
  if (millis() - previousTimes[7] >= 20000) {
    previousTimes[7] = millis();
    fanTemp(28);
  }

  if(chkTime(00, 00, 00)){days++; ws.textAll("DAY" + String(days));}



  static int lastTemp = 0;
  if (chkTime(fanOnTime[0], fanOnTime[1], fanOnTime[2]) && lightState) {
    transmitFanCode(FAN_LOW);
    transmitFanCode(FANLIGHT_ON_OFF);
    lightState = false;
  }

  if (chkTime(fanlightOnTime[0], fanlightOnTime[1], fanlightOnTime[2]) && !lightState) {
    transmitFanCode(FANLIGHT_ON_OFF);
    lightState = true;
  }

  if (chkTime(fanlightOffTime[0], fanlightOffTime[1], fanlightOffTime[2])) {
    transmitFanCode(FAN_OFF);
  }
  if (millis() - previousTimes[2] >= 1000) {
    previousTimes[2] = millis();
    ws.cleanupClients();
  }

  if (lastTemp != outputTemp() && millis() - previousTimes[4] >= 1000) {
    previousTimes[4] = millis();
    ws.textAll("TEM" + String(outputTemp()));
    lastTemp = outputTemp();
  }

  if (ktState != lightState) { //only send the websocket request when the state only gets changed otherwise will be too much sending traffic and can fuck up things
    ws.textAll("LS" + String(lightState));
    ktState = lightState;
  }


}
