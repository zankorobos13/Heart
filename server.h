#include <LittleFS.h>

bool IsSsidExists(const String& targetSsid) {
    File file = LittleFS.open("/wifi.txt", "r");

    if (!file) {
      return false;
    }

    while (file.available()) {

        String ssid = file.readStringUntil('\n');
        ssid.trim();

        if (ssid == targetSsid) {
            file.close();
            return true;
        }

        if (file.available()) {
            file.readStringUntil('\n');
        }
    }

    file.close();
    return false;
}

String IsValid(const String& str, bool isSSID = true) {
    String tmp = str;
    tmp.trim();

    if (tmp.length() == 0)
        return "void";

    for (size_t i = 0; i < str.length(); i++) {
        unsigned char c = str[i];

        if (c > 127) {  
            return "invalid";
        }
    }
    if (isSSID && IsSsidExists(str))
      return "same";
    return "OK";
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

  File file = LittleFS.open("/wifi.txt", "a");

  if (!file) {
    server.send(500, "text/plain", "File write error");
    return;
  }

  String is_ssid_valid = IsValid(ssid);
  String is_password_valid = IsValid(password, false);

  if (is_ssid_valid == "OK" && is_password_valid == "OK"){
    file.println(ssid);
    file.println(password);
    file.close();

    server.send(200, "text/html; charset=UTF-8", "<link rel=\"stylesheet\" href=\"/save.css\"><h2>Сохранено</h2><br><h3>SSID: " + ssid + "</h3><br><h3>PASSWORD:" + password + "</h3><br><a href='/'>Назад</a>");
  }
  else if (is_ssid_valid == "invalid" || is_password_valid == "invalid"){
    server.send(200, "text/html; charset=UTF-8", "<link rel=\"stylesheet\" href=\"/save.css\"><h2>SSID или пароль используют недопустимые символы!</h2><br><h3>Разрешена только латиница, цифры и специальные символы</h3><br><a href='/'>Назад</a>");
  }
  else if (is_ssid_valid == "void" || is_password_valid == "void"){
    server.send(200, "text/html; charset=UTF-8", "<link rel=\"stylesheet\" href=\"/save.css\"><h2>SSID или пароль не заполнены!</h2><br><h3>Пожалуйста, заполните поля \"SSID\" и \"Пароль\"</h3><br><a href='/'>Назад</a>");
  }
  else if (is_ssid_valid == "same"){
    server.send(200, "text/html; charset=UTF-8", "<link rel=\"stylesheet\" href=\"/save.css\"><h2>В списке уже есть Wi-Fi с таким SSID!</h2><br><h3>Если вы хотите изменить пароль для данного SSID - удалите его из списка и добавьте заново</h3><br><a href='/'>Назад</a>");
  }
}

void handleDeleteItem(ESP8266WebServer& server) {

  int deleteIndex = server.arg("index").toInt();

  File file = LittleFS.open("/wifi.txt", "r");

  if (!file) {
    server.send(500, "text/plain", "wifi.txt not found");
    return;
  }

  String newContent;

  int index = 0;

  while (file.available()) {

    String ssid = file.readStringUntil('\n');
    String password = file.readStringUntil('\n');

    ssid.trim();
    password.trim();

    if (index != deleteIndex) {
      newContent += ssid + "\n";
      newContent += password + "\n";
    }

    index++;
  }

  file.close();

  file = LittleFS.open("/wifi.txt", "w");

  if (!file) {
    server.send(500, "text/plain", "write error");
    return;
  }

  file.print(newContent);
  file.close();

  server.sendHeader("Location", "/list");
  server.send(303);
}

void handleDeletePage(ESP8266WebServer& server) {
  File file = LittleFS.open("/wifi.txt", "r");

  String html =
      "<html><head><meta charset='UTF-8'><link rel=\"stylesheet\" href=\"/list.css\"></head><body>"
      "<h2>Сохраненные сети</h2>";

  if (!file) {
    html += "<p>Файл не найден</p></body></html>";
    server.send(200, "text/html; charset=UTF-8", html);
    return;
  }

  int index = 0;

  while (file.available()) {
    String ssid = file.readStringUntil('\n');
    String password = file.readStringUntil('\n');

    ssid.trim();
    password.trim();

    html += "<div>";
    html += "<b>" + ssid + "</b> ";
    html += "<form style='display:inline' method='POST' action='/deleteItem'>";
    html += "<input type='hidden' name='index' value='" + String(index) + "'>";
    html += "<button type='submit'>Удалить</button>";
    html += "</form>";
    html += "</div><br>";

    index++;
  }

  file.close();

  html += "<br><a href='/'>Назад</a>";
  html += "</body></html>";

  server.send(200, "text/html; charset=UTF-8", html);
}

void handleConfigRead(ESP8266WebServer& server){
  File file = LittleFS.open("/config.json", "r");
  String content = file.readString();
  file.close();

  String html =
  "<!DOCTYPE html>"
  "<html>"
  "<head>"
  "<meta charset='UTF-8'>"
  "<title>Редактирование config.json</title>"
  "<link rel='stylesheet' href='/edit_config.css'>"
  "</head>"
  "<body>"
  "<div>"
  "<h2>Редактирование config.json</h2>"
  "<form action='/saveConfig' method='POST'>"
  "<textarea name='text'>"
  + content +
  "</textarea>"
  "<br>"
  "<input type='submit' value='Сохранить'>"
  "</form>"
  "<br>"
  "<a class='back-btn' href='/'>Назад</a>"
  "</div>"
  "</body>"
  "</html>";
  
  server.send(200, "text/html; charset=UTF-8", html);
}

void handleConfigSave(ESP8266WebServer& server){
  String content = String(server.arg("text"));

  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, content);

  bool invalid =
    error ||
    doc["ap_ssid"].isNull() ||
    doc["ap_password"].isNull() ||
    doc["url"].isNull() ||
    doc["path"].isNull() ||
    doc["private_password"].isNull() ||

    !doc["green_led_blink_period"].is<int>() ||
    !doc["green_led_blink_time"].is<int>() ||

    doc["green_led_blink_period"].as<int>() <= 0 ||
    doc["green_led_blink_time"].as<int>() <= 0 ||

    doc["green_led_blink_period"].as<int>() <=
        doc["green_led_blink_time"].as<int>();

  if (invalid) {
    Serial.println(error.c_str());
    File file = LittleFS.open("/default_config.json", "r");
    content = file.readString();

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);

    file.close();

    file = LittleFS.open("/config.json", "w");
    file.println(content);
    file.close();
  }
  else{
    File file = LittleFS.open("/config.json", "w");
    file.print(content);
    file.close();
  }

  server.sendHeader("Location", "/config");
  server.send(303);
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

  server.on("/list", HTTP_GET, [&server]() {
    handleDeletePage(server);
  });

  server.on("/deleteItem", HTTP_POST, [&server]() {
    handleDeleteItem(server);
  });

  server.on("/config", HTTP_GET, [&server](){
    handleConfigRead(server);
  });

  server.on("/saveConfig", HTTP_POST,[&server](){
    handleConfigSave(server);
  });

  server.on("/save.css", HTTP_GET, [&]() {
    handleFileRead("/save.css", server);
  });

  server.on("/list.css", HTTP_GET, [&]() {
    handleFileRead("/list.css", server);
  });
  
  server.on("/edit_config.css", HTTP_GET, [&]() {
    handleFileRead("/edit_config.css", server);
  });  
}