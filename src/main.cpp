#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>

#include "config.h"


AsyncWebServer server(80);


#define HISTORY_SIZE 4800

int prevRead = 0;
boolean pulse = false;
unsigned long pulseCounter = 0;
unsigned long lastUpdate = 0;
unsigned long pulseBegin = 0;
unsigned long pulseLog[HISTORY_SIZE];
int pulseN = 0;

int pulsesInLast(int secs) {
  int t_pulses = 0;
  for (int i = 0; i < HISTORY_SIZE; i++)
  {
    if(pulseLog[i] > 0 && millis() - pulseLog[i] < secs * 1000) {
      t_pulses++;
    }
  }
  return t_pulses;
}

double pulsesToKWH(int pulses) {
  return pulses / 1600.;
}

double KWHtoKW(double kwh, int secs) {
  return kwh * (3600 / secs);
}

String processor(const String &var)
{
  if (var == "pulseCounter")
  {
    return String(pulseCounter);
  }
   if (var == "pulses60")
  {
    return String(pulsesInLast(60));
  }
   if (var == "pulses300")
  {
    return String(pulsesInLast(300));
  }


  if (var == "kwh60") {
    char buf[20];
    snprintf(buf, sizeof(buf), "%5f", pulsesToKWH(pulsesInLast(60)));
    return String(buf);
  }

    if (var == "kwh300") {
    char buf[20];
    snprintf(buf, sizeof(buf), "%5f", pulsesToKWH(pulsesInLast(300)));
    return String(buf);
  }

  
  if (var == "kw60") {
    char buf[20];
    snprintf(buf, sizeof(buf), "%5f", KWHtoKW(pulsesToKWH(pulsesInLast(60)), 60));
    return String(buf);
  }

    if (var == "kw300") {
    char buf[20];
    snprintf(buf, sizeof(buf), "%5f", KWHtoKW(pulsesToKWH(pulsesInLast(300)), 300));
    return String(buf);
  }



  return String();
}



void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);


  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi Failed!\n");
    return;
  }

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  SPIFFS.begin();

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", String(), false, processor);
  });

  server.on("/json", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/json.json", String(), false, processor);
  });

  ArduinoOTA.begin();
  server.begin();

}




void loop() {
  // put your main code here, to run repeatedly:
  

  int curRead = analogRead(A0);

  int delta = curRead - prevRead;

  if(prevRead > 0) {
    if(delta > 25) {
        if(!pulse) {
          pulse = true;
          pulseBegin = millis();
          pulseCounter++;
          pulseLog[pulseN] = millis();
        }
    }
    if(delta < -25) {
        pulse = false;
    }
  }

  if(pulse && millis() - pulseBegin > 1500) {
    pulse = false;
    prevRead = 0;
  }
  

  prevRead = curRead;
  pulseN++;
  if(pulseN >= HISTORY_SIZE) {
    pulseN = 0;
  }

  delay(10);
   ArduinoOTA.handle();

  /*if(millis() - lastUpdate > 3000) {
    Serial.printf("pulses: %ld\n", pulseCounter);
    Serial.printf("pulses last 30 secs: %d\n", pulsesInLast(30));
    Serial.printf("pulses last 5 min: %d\n", pulsesInLast(300));
    lastUpdate = millis();
  }*/
 
}