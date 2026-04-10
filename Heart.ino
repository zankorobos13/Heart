#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
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

  File file = LittleFS.open("/config.txt", "r");

  if (!file) {
    Serial.println("Ошибка открытия файла");
    return;
  }

  while (file.available()) {
    Serial.write(file.read());
  }

  file.close();

  WiFi.softAP(ssid, password);
  Serial.println(WiFi.softAPIP());

  setupRoutes(server);

  server.begin();
}

void loop() {
  server.handleClient();
}