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

unsigned long long check_msg_prev = 0;
int check_msg_period = 5000;

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
JsonDocument GetCurrMessageJson();

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

  if (error || (doc["ap_ssid"].isNull() || doc["ap_password"].isNull() || 
    doc["url"].isNull() || doc["path"].isNull() || 
    doc["private_password"].isNull())) {
    Serial.println(error.c_str());
    file = LittleFS.open("/default_config.json", "r");
    String content = file.readString();

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);

    file.close();

    file = LittleFS.open("/config.json", "w");
    file.println(content);
    file.close();
  }

  ssid = String(doc["ap_ssid"]);
  password = String(doc["ap_password"]);
  url = String(doc["url"]) + String(doc["path"]);
  private_password = String(doc["private_password"]);
  
  

  if (GetCurrMessageJson()["is_readed"])
    blue_led.Off();
  else
    blue_led.On();

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
  
  switch(mode){
    case WaitingConnection:
    {
      red_led.Blink(3000, 500);
      if (networks.size() != 0 && CanUpdateAvailableNetworks)
      {
        available_networks = GetFullyAvailableNetworks(networks, FindNetworks());
        CanUpdateAvailableNetworks = false;
      }
      TryToConnect();
      break;
    }
    case Connected:
    {
      green_led.Blink(3000, 500);
      if (WiFi.status() != WL_CONNECTED){
        ESP.restart();
        break;
      }

      if (millis() - check_msg_prev > check_msg_period){
        JsonDocument doc = remoteServer.GetMessage(private_password);
      
        if (String(doc["status"] == "success") && doc["data"]["timestamp"] != GetCurrMessageJson()["timestamp"]){
          JsonDocument message;
          message["message"] = doc["data"]["message"];
          message["timestamp"] = doc["data"]["timestamp"];
          message["is_readed"] = false;

          String message_str;
          serializeJson(message, message_str);

          Serial.println(message_str);

          File file = LittleFS.open("/message.json", "w");
          file.print(message_str);
          file.close();

          blue_led.On();
        }

        check_msg_prev = millis();
      }      

      break;
    }
    case Settings:
    {
      red_led.Blink(3000, 500);
      green_led.Blink(3000, 500);
      server.handleClient();
      break;
    }
    case MessageReading:
    {
      blue_led.Off();
      delay(500);
      JsonDocument curr_message = GetCurrMessageJson();
      blue_led.BlinkSequence(String(curr_message["message"]));
      delay(100);
      blue_led.Off();

      JsonDocument upd_message;
      upd_message["message"] = curr_message["message"];
      upd_message["timestamp"] = curr_message["timestamp"];
      upd_message["is_readed"] = true;

      String upd_message_str;
      serializeJson(upd_message, upd_message_str);

      File file = LittleFS.open("/message.json", "w");
      file.print(upd_message_str);
      file.close();

      ChangeMode(Connected);
      break;
    }
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
    case WaitingConnection:
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

JsonDocument GetCurrMessageJson(){
  File file = LittleFS.open("/message.json", "r");

  JsonDocument doc;
  deserializeJson(doc, file);

  file.close();

  return doc;
}


