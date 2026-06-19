#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <OneButton.h>
#include "server.h"
#include "LED.h"

#define RED_LED_PIN 12
#define BLUE_LED_PIN 13
#define GREEN_LED_PIN 14

#define BUTTON_PIN 4

const int flashing_period_ms = 2000;
const int flashing_time_ms = 500;

const char* ssid = "HEART"; 
const char* password = "12345678";

ESP8266WebServer server(80);  


OneButton button(BUTTON_PIN, false);
LED red_led = LED(RED_LED_PIN, flashing_period_ms, flashing_time_ms);
LED blue_led = LED(BLUE_LED_PIN, flashing_period_ms, flashing_time_ms);
LED green_led = LED(GREEN_LED_PIN, flashing_period_ms, flashing_time_ms);

enum Mode {
  WaitingConnection,
  Connected,
  Settings,
  MessageReading
};

Mode mode = WaitingConnection;
Mode prev_mode = mode;

void setup() {
  Serial.begin(74880);

  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(BLUE_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT);

  button.attachClick(SingleClick);
  button.attachLongPressStart(LongPress);
  button.setPressTicks(5000);

  if (!LittleFS.begin()) {
    Serial.println("FS ERROR");
  }

  File file = LittleFS.open("/config.txt", "r");

  if (!file) {
    Serial.println("Ошибка открытия файла config.txt");
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
  button.tick();
  if (mode != prev_mode){
    Serial.println("Режим: " + String(mode));
    prev_mode = mode;
  }

  green_led.Off();
  red_led.Off();
  blue_led.Off();

  switch(mode){
    case WaitingConnection:
      red_led.Blink(3000, 500);
      break;
    case Connected:
      green_led.Blink(30000, 500);
      break;
    case Settings:
      red_led.Blink(3000, 500);
      green_led.Blink(3000, 500);
      server.handleClient();
      break;
    default:
      break;

  }
  
}

void SingleClick(){
  Serial.println("single_click");
  switch (mode){
    case Connected:
      ChangeMode(MessageReading);
      break;
    case Settings:
      ChangeMode(WaitingConnection);
    default:
      break;
  }
}

void LongPress(){
  Serial.println("long_press");
  switch (mode){
    case MessageReading:
      ChangeMode(Connected);
      break;
    case Connected:
      ChangeMode(Settings);
      break;
    case WaitingConnection:
      ChangeMode(Settings);
      break;
    default:
      break;
  }
}

void ChangeMode(Mode new_mode){
  green_led.On();
  red_led.Off();
  blue_led.Off();
  mode = new_mode;
  delay(500);
  green_led.Off();
}