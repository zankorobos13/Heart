#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include "server.h"
#include "LED.h"

#define RED_LED_PIN 12
#define BLUE_LED_PIN 14
#define GREEN_LED_PIN 13

const int flashing_period_ms = 5000;
const int flashing_time_ms = 500;

const char* ssid = "HEART"; 
const char* password = "12345678";

ESP8266WebServer server(80);  

LED red_led;

void setup() {
  Serial.begin(74880);

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  red_led = LED(RED_LED_PIN, 2000, 500);
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
  red_led.Blink();
  server.handleClient();
}