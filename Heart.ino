#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "config.h"

WiFiData wifi_data;
const char* SSID = wifi_data.SSID;
const char* PASSWORD = wifi_data.PASSWORD;
const char* SERVER_URL = wifi_data.SERVER_URL;

void setup() {
  Serial.begin(74880);
  
  WiFi.begin(SSID, PASSWORD);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.println("WiFi подключен");
  Serial.print("IP адрес: ");
  Serial.println(WiFi.localIP());
}
void loop() {
}
