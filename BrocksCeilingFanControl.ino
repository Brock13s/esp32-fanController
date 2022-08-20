// Firmware to control my ceiling fan from webpage, alexa and by ntp time.
// Author Brock Mills

#include <RCSwitch.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include "time.h"
#include "wifi_credentials.h"

#define red_led 26
#define white_led 23
#define green_led 32
#define transmitterLed 22
#define transmitterPin 25
#define ldrPin 33

unsigned long previousTimes[2]={0};
bool blinkOn = false;
const char* message = "";

RCSwitch mySwitch = RCSwitch();
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

IPAddress local_IP(192, 168, 68, 25);
IPAddress gateway(192, 168, 68, 1);
IPAddress subnet(255, 255, 252, 0);
IPAddress DNS(202, 142, 142, 142);

/*
 *  12 hour to 24 hour reference chart :)
12:00 AM  00:00
01:00 AM  01:00
02:00 AM  02:00
03:00 AM  03:00
04:00 AM  04:00
05:00 AM  05:00
06:00 AM  06:00
07:00 AM  07:00
08:00 AM  08:00
09:00 AM  09:00
10:00 AM  10:00
11:00 AM  11:00
12:00 PM  12:00
01:00 PM  13:00
02:00 PM  14:00
03:00 PM  15:00
04:00 PM  16:00
05:00 PM  17:00
06:00 PM  18:00
07:00 PM  19:00
08:00 PM  20:00
09:00 PM  21:00
10:00 PM  22:00
11:00 PM  23:00
*/


void Wifi_disconnected(WiFiEvent_t event, WiFiEventInfo_t info){
  digitalWrite(red_led, HIGH);
  digitalWrite(green_led, LOW);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
}

void Wifi_connected(WiFiEvent_t event, WiFiEventInfo_t info){
  digitalWrite(red_led, LOW);
  digitalWrite(green_led, HIGH);
}



void onEventWS(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len){
  switch(type){
    case WS_EVT_CONNECT:
      Serial.printf("Websocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("Websocket client #%u disconnected\n", client->id());
      break;
    case WS_EVT_DATA:
      handleWebSocketMessage(arg, data, len);
      break;
    case WS_EVT_ERROR:
      break;
  }
}

void initWebsocket(){
  ws.onEvent(onEventWS);
  server.addHandler(&ws);
}


void webpageHandling(){
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
//    if(!request->authenticate(HTTP_USER, HTTP_PASS)){
//        return request->requestAuthentication();
//        
//      }
    request->send(SPIFFS, "/index.html", String(), false);
    
    previousTimes[0] = millis();
    blinkOn = true;
    });
    server.on("/css.css", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/css.css", "text/css");
    });

    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/script.js", "text/javascript");
    });

  server.begin();
  }

void transmitFanCode(const char* code){
    #define FAN_HIGH "000001011111" 
    #define FAN_MED "000001101111"
    #define FAN_LOW "000001110111"
    #define FAN_OFF "000001111101"
    #define CHANGE_LIGHT_COLOR "000001110110"
    #define FANLIGHT_ON_OFF "000001111110"
    int fanSpeedStatus=0;
    if(strcmp(code, FAN_OFF)==0){
      fanSpeedStatus = 1;
      } else if(strcmp(code, FAN_LOW)==0){
        fanSpeedStatus = 2;
      } else if(strcmp(code, FAN_MED)==0){
        fanSpeedStatus = 3;
      } else if(strcmp(code, FAN_HIGH)==0){
        fanSpeedStatus = 4;
      }
    
    digitalWrite(transmitterLed, HIGH);
    mySwitch.send(code);
    digitalWrite(transmitterPin, LOW);
    digitalWrite(transmitterLed, LOW);
    
  }

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len){
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if(info->final && info->index == 0 && info->len == len && info -> opcode == WS_TEXT){
    data[len]=0;
    message = (char*)data;
    if(strcmp(message, "FANCODE_MED") == 0){
      transmitFanCode(FAN_MED);
    }
    if(strcmp(message, "FANCODE_LOW") == 0){
      transmitFanCode(FAN_LOW);
    }
    if(strcmp(message, "FANCODE_OFF") == 0){
      transmitFanCode(FAN_OFF);
    }
    if(strcmp(message, "FANCODE_HIGH") == 0){
      transmitFanCode(FAN_HIGH);
    }
    if(strcmp(message, "FANCODE_CHANGECOLOR") == 0){
      transmitFanCode(CHANGE_LIGHT_COLOR);
    }
    if(strcmp(message, "FANCODE_LIGHTONOFF") == 0){
      transmitFanCode(FANLIGHT_ON_OFF);
    }
  }
}

bool chkTime(int thr, int tmin, int tsec){
  time_t n;
  struct tm * timeinfo;
  time(&n);
  timeinfo = localtime(&n);
  int int_hr = timeinfo->tm_hour;
  int int_min = timeinfo->tm_min;
  int int_sec = timeinfo->tm_sec;
  if(int_hr == thr && int_min == tmin && int_sec == tsec){
    return true;
  } else {
    return false;
    }
  }  

void setNTPzone(){
  configTime(0, 0, "pool.ntp.org");
  setenv("TZ", "AEST-10", 1);
  tzset();
}

void blinkWebConnectionLed(){
  if(blinkOn){
    digitalWrite(white_led, HIGH);
    if(millis() - previousTimes[0] >= 1000){
      digitalWrite(white_led, LOW);
      blinkOn = false;
      
    }
  }
}

void setup() {
  Serial.begin(115200);
  WiFi.disconnect(true);
  WiFi.onEvent(Wifi_connected, ARDUINO_EVENT_WIFI_STA_CONNECTED);
  WiFi.onEvent(Wifi_disconnected, ARDUINO_EVENT_WIFI_STA_DISCONNECTED); 
  if(!WiFi.config(local_IP, gateway, subnet, DNS)){
      Serial.println("STA failed to configure");
    }
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  if(!SPIFFS.begin(true)){
    Serial.println("Error occured while trying to mount spiffs :("); 
  }
  mySwitch.enableTransmit(transmitterPin);
  mySwitch.setProtocol(6);
  mySwitch.setPulseLength(328);
  pinMode(red_led, OUTPUT);
  pinMode(green_led, OUTPUT);
  pinMode(white_led, OUTPUT);
  pinMode(transmitterLed, OUTPUT);
  pinMode(ldrPin, INPUT);
  pinMode(transmitterPin, OUTPUT);
  initWebsocket();
  setNTPzone();
  transmitFanCode(FAN_OFF); // If the fan is on turn it off for the webpage to handle the button selection better or it will be messed up
  webpageHandling();
}

void loop() {
  //blinkWebConnectionLed();
  //int ldrVal = analogRead(ldrPin);
  if(chkTime(20, 30, 0)){
    transmitFanCode(FAN_LOW);
    transmitFanCode(FANLIGHT_ON_OFF);
  }

  if(chkTime(18, 00, 0)){
    transmitFanCode(FANLIGHT_ON_OFF);
  }

  if(chkTime(6, 30, 0)){
    transmitFanCode(FAN_OFF);
  }
  ws.cleanupClients();

}
