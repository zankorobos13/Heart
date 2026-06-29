#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

class RemoteServer{
  private:
    String url;
  public:
    RemoteServer(String url){
      this->url = url; 
    }
    
    JsonDocument GetMessage(String password){
      WiFiClient client;
      HTTPClient http;

      http.begin(client, url);

      http.addHeader("Content-Type", "application/x-www-form-urlencoded");
      http.addHeader("User-Agent", "Mozilla/5.0");

      String body = "mode=get_message&password=" + password;

      int httpCode = http.POST(body);
      String response;

      if (httpCode > 0) {
        response = http.getString();
      }
      else{
        response = "";
      }
      http.end();

      JsonDocument doc;
      deserializeJson(doc, response);

      return doc;
    }
};