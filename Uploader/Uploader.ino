#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>

ESP8266WebServer server(80);

File uploadFile;

const char* ssid = "ESP_UPLOADER";
const char* password = "12345678";

const char uploadPage[] PROGMEM = R"====(
<!DOCTYPE html>
<html>
<head>
<meta charset="utf-8">
<title>LittleFS Upload</title>
</head>
<body>
<h2>Upload file to LittleFS</h2>

<form method="POST" action="/upload" enctype="multipart/form-data">
<input type="file" name="data">
<input type="submit" value="Upload">
</form>

<hr>

<a href="/list">List files</a>

</body>
</html>
)====";

void handleUpload() {
  HTTPUpload& upload = server.upload();

  if (upload.status == UPLOAD_FILE_START) {

    String filename = upload.filename;

    if (!filename.startsWith("/"))
      filename = "/" + filename;

    Serial.println("UPLOAD START: " + filename);

    uploadFile = LittleFS.open(filename, "w");
  }

  else if (upload.status == UPLOAD_FILE_WRITE) {

    if (uploadFile) {
      uploadFile.write(upload.buf, upload.currentSize);
    }
  }

  else if (upload.status == UPLOAD_FILE_END) {

    if (uploadFile) {
      uploadFile.close();
    }

    Serial.printf(
      "UPLOAD END: %s (%u bytes)\n",
      upload.filename.c_str(),
      upload.totalSize
    );
  }
}

void handleList() {

  String html;

  html += "<html><head><meta charset='utf-8'></head><body>";
  html += "<h2>Files</h2><ul>";

  Dir dir = LittleFS.openDir("/");

  while (dir.next()) {

    html += "<li>";
    html += dir.fileName();
    html += " (";
    html += dir.fileSize();
    html += " bytes)";
    html += "</li>";
  }

  html += "</ul>";
  html += "<a href='/'>Back</a>";
  html += "</body></html>";

  server.send(200, "text/html; charset=UTF-8", html);
}

void setup() {

  Serial.begin(74880);

  if (!LittleFS.begin()) {
    Serial.println("LittleFS mount error");
    return;
  }

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  Serial.print("IP: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, []() {
    server.send_P(
      200,
      "text/html; charset=UTF-8",
      uploadPage
    );
  });

  server.on(
    "/upload",
    HTTP_POST,
    []() {
      server.send(
        200,
        "text/html; charset=UTF-8",
        "<h2>Файл загружен</h2><a href='/'>Назад</a>"
      );
    },
    handleUpload
  );

  server.on("/list", HTTP_GET, handleList);

  server.begin();

  Serial.println("Uploader started");
}

void loop() {
  server.handleClient();
}