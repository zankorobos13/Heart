#include <LittleFS.h>

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

void setupRoutes(ESP8266WebServer& server) {
  server.on("/", [&server]() {
    handleFileRead("/index.html", server);
  });

  server.onNotFound([&server]() {
    handleFileRead(server.uri(), server);
  });
}