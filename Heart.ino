#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include "config.h"
#include "server.h"

ESP8266WebServer server(80);

const char* ssid = "HEART"; 
const char* password = "12345678";

void setup() {
  Serial.begin(74880);

  if (!LittleFS.begin()) {
    Serial.println("FS ERROR");
    return;
  }

  WiFi.softAP(ssid, password);
  Serial.println(WiFi.softAPIP());

  setupRoutes(server);

  server.begin();
}

void loop() {
  server.handleClient();
}