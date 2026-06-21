#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <OneButton.h>
#include <algorithm>
#include <vector>
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

std::vector<String> networks;

ESP8266WebServer server(80);  


OneButton button(BUTTON_PIN, false);
LED red_led = LED(RED_LED_PIN, flashing_period_ms, flashing_time_ms);
LED blue_led = LED(BLUE_LED_PIN, flashing_period_ms, flashing_time_ms);
LED green_led = LED(GREEN_LED_PIN, flashing_period_ms, flashing_time_ms);

int ttc_i = 0;
unsigned long long ttc_prev = 0;
int ttc_period = 10000;
bool ttc_isBegun = false;

void TryToConnect();

enum Mode {
  WaitingConnection,
  Connected,
  Settings,
  MessageReading
};

Mode mode = WaitingConnection;
Mode prev_mode = mode;

bool CanUpdateAvailableNetworks = true;

std::vector<String> available_networks;

void StartAPMode();
void StartSTAMode();
std::vector<String> FindNetworks();
std::vector<String> GetFullyAvailableNetworks(std::vector<String> networks_known, std::vector<String> networks_available);
void UpdateNetworks();
void ChangeMode(Mode new_mode);

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

  UpdateNetworks();

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
      if (CanUpdateAvailableNetworks)
      {
        available_networks = GetFullyAvailableNetworks(networks, FindNetworks());
        CanUpdateAvailableNetworks = false;
      }
      TryToConnect();
      break;
    case Connected:
      green_led.Blink(30000, 500);
      if (WiFi.status() != WL_CONNECTED){
        CanUpdateAvailableNetworks = true;
        ChangeMode(WaitingConnection);
      }        
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
      StartSTAMode();
      UpdateNetworks();
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
      StartAPMode();
      break;
    case WaitingConnection:
      ChangeMode(Settings);
      StartAPMode();
      break;
    default:
      break;
  }
}

void StartAPMode(){
  WiFi.disconnect(true);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.println(WiFi.softAPIP());
}

void StartSTAMode(){
  WiFi.softAPdisconnect(true);
  WiFi.disconnect(true);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
}

std::vector<String> FindNetworks() {
    std::vector<std::pair<int, String>> found;

    int n = WiFi.scanNetworks();

    if (n <= 0) {
        Serial.println("Networks not found");
        return {};
    }

    for (int i = 0; i < n; i++) {
        found.push_back({
            WiFi.RSSI(i),
            WiFi.SSID(i)
        });
    }

    std::sort(
        found.begin(),
        found.end(),
        [](const auto& a, const auto& b) {
            return a.first > b.first;
        }
    );

    std::vector<String> result;

    for (const auto& net : found) {
        result.push_back(net.second);
        // Serial.println("FOUND: " + net.second);
    }

    return result;
}

std::vector<String> GetFullyAvailableNetworks(std::vector<String> networks_known, std::vector<String> networks_available){
  std::vector<String> result;
  for (int i = 0; i < networks_available.size(); i++){
    for (int j = 0; j < networks_known.size(); j+=2){
      if (networks_available[i] == networks_known[j]){
        result.push_back(networks_known[j]);
        result.push_back(networks_known[j+1]);
        
      }
    }
  }
  return result;
}

void UpdateNetworks(){
  File file = LittleFS.open("/config.txt", "r");

  if (!file) {
    Serial.println("Ошибка открытия файла config.txt");
  }

  while (file.available()) {
    String line = file.readStringUntil('\n');
    line.trim();

    if (line.length() > 0) {
        networks.push_back(line);
    }
  }

  for (int i = 0; i < networks.size(); i+=2){
    Serial.print(networks[i]);
    Serial.print(" - ");
    Serial.println(networks[i + 1]);
  }

  file.close();
}

void ChangeMode(Mode new_mode){
  green_led.On();
  red_led.Off();
  blue_led.Off();
  mode = new_mode;
  delay(500);
  green_led.Off();
}

void TryToConnect(){
  if (available_networks.size() == 0){
    CanUpdateAvailableNetworks = true;
    return;
  }

  if (WiFi.status() == WL_CONNECTED){
    Serial.println("CONNECTED!");
    ChangeMode(Connected);
    return;
  }
   
  if (millis() - ttc_prev > ttc_period){
    if (ttc_i + 2 < available_networks.size())
      ttc_i += 2;
    else{
      ttc_i = 0;
      CanUpdateAvailableNetworks = true;
    }
    ttc_isBegun = false;
    ttc_prev = millis();
  }

  if (!ttc_isBegun){
    WiFi.disconnect();
    WiFi.begin(available_networks[ttc_i], available_networks[ttc_i + 1]);
    Serial.println("Trying: " + available_networks[ttc_i] + " " + available_networks[ttc_i + 1]);
    ttc_isBegun = true;
  }
}

