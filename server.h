#include <LittleFS.h>

bool IsValid(const String& str) {
    String tmp = str;
    tmp.trim();

    if (tmp.length() == 0)
        return false;

    for (size_t i = 0; i < str.length(); i++) {
        unsigned char c = str[i];

        if (c > 127) {  
            return false;
        }
    }

    return true;
}

void handleFileRead(String path, ESP8266WebServer& server) {
  Serial.println("Запрос: " + path);

  if (path.endsWith("/")) path += "index.html";

  String contentType = "text/plain";
  if (path.endsWith(".html")) contentType = "text/html";
  else if (path.endsWith(".css")) contentType = "text/css";

  if (LittleFS.exists(path)) {
    File file = LittleFS.open(path, "r");
    server.streamFile(file, contentType);
    file.close();
    Serial.println("OK: " + path);
  } else {
    Serial.println("404: " + path);
    server.send(404, "text/plain", "File NOT FOUND");
  }
}

void handleSave(ESP8266WebServer& server) {
  String ssid = server.arg("ssid");
  String password = server.arg("password");

  Serial.println("SSID: " + ssid);
  Serial.println("PASS: " + password);

  File file = LittleFS.open("/config.txt", "a");

  if (!file) {
    server.send(500, "text/plain", "File write error");
    return;
  }

  if (IsValid(ssid) && IsValid(password)){
    file.println(ssid);
    file.println(password);
    file.close();

    server.send(200, "text/html; charset=UTF-8", "<h2>Сохранено</h2><h3>SSID: " + ssid + "</h3><h3>PASSWORD:" + password + "</h3><a href='/'>Назад</a>");
  }
  else{
    server.send(200, "text/html; charset=UTF-8", "<h2>SSID или пароль используют недопустимые символы или пусты!</h2><h3>Разрешена только латиница, цифры и специальные символы</h3><a href='/'>Назад</a>");
  }
}

void setupRoutes(ESP8266WebServer& server) {
  server.on("/", [&server]() {
    handleFileRead("/index.html", server);
  });

  server.on("/save", HTTP_POST,[&server](){
    handleSave(server);
  });

  server.onNotFound([&server]() {
    handleFileRead(server.uri(), server);
  });
}