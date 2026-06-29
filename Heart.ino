#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <OneButton.h>
#include <algorithm>
#include <vector>
#include <ArduinoJson.h>
#include "server.h"
#include "LED.h"
#include "requests.h"

#define RED_LED_PIN 12
#define BLUE_LED_PIN 13
#define GREEN_LED_PIN 14

#define BUTTON_PIN 4

const int flashing_period_ms = 2000;
const int flashing_time_ms = 500;

String ssid = "HEART"; 
String password = "12345678";
String url;
String private_password;


std::vector<String> networks;

ESP8266WebServer server(80);  
RemoteServer remoteServer("");

OneButton button(BUTTON_PIN, false);
LED red_led = LED(RED_LED_PIN, flashing_period_ms, flashing_time_ms);
LED blue_led = LED(BLUE_LED_PIN, flashing_period_ms, flashing_time_ms);
LED green_led = LED(GREEN_LED_PIN, flashing_period_ms, flashing_time_ms);

int ttc_i = 0;
unsigned long long ttc_prev = 0;
int ttc_period = 15000;
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

  File file = LittleFS.open("/config.json", "r");

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);

  file.close();

  if (error) {
    Serial.println(error.c_str());
  }
  else{
   // Прописать парсинг дефолтного неизменяемого json`a 
  }

  ssid = String(doc["ap_ssid"]);
  password = String(doc["ap_password"]);
  url = String(doc["url"]) + String(doc["path"]);
  private_password = String(doc["private_password"]);

  remoteServer = RemoteServer(url);

  UpdateNetworks();
  StartSTAMode();

  WiFi.persistent(false);

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
      if (networks.size() != 0 && CanUpdateAvailableNetworks)
      {
        available_networks = GetFullyAvailableNetworks(networks, FindNetworks());
        CanUpdateAvailableNetworks = false;
      }
      TryToConnect();
      break;
    case Connected:
    {
      green_led.Blink(30000, 500);
      if (WiFi.status() != WL_CONNECTED){
        ESP.restart();
        break;
      }
      JsonDocument doc = remoteServer.GetMessage(private_password);
      String json;
      serializeJson(doc, json);
      Serial.println(json);
      break;
    }
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
      ESP.restart();
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

void StartAPMode() {
    WiFi.disconnect(false);
    WiFi.softAPdisconnect(true);

    delay(100);

    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);

    ttc_isBegun = false;
    ttc_i = 0;
}

void StartSTAMode()
{
    WiFi.mode(WIFI_OFF);
    delay(500);

    WiFi.mode(WIFI_STA);
    delay(500);

    WiFi.disconnect(false);

    ttc_isBegun = false;
    CanUpdateAvailableNetworks = true;

    Serial.println("STA READY");
}

std::vector<String> FindNetworks() {
    std::vector<std::pair<int, String>> found;

    WiFi.disconnect(false);
    delay(50);

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
  networks.clear();
  available_networks.clear();
  
  ttc_isBegun = false;
  ttc_i = 0;

  File file = LittleFS.open("/wifi.txt", "r");

  if (!file) {
    Serial.println("Ошибка открытия файла wifi.txt");
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
    if (millis() - ttc_prev > ttc_period){
      CanUpdateAvailableNetworks = true;
      ttc_prev = millis();
    }    
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
    WiFi.disconnect(false);
    delay(100);
    
    WiFi.begin(available_networks[ttc_i], available_networks[ttc_i + 1]);
    
    Serial.println("Trying: " + available_networks[ttc_i] + " " + available_networks[ttc_i + 1]);
    ttc_isBegun = true;
  }
}

