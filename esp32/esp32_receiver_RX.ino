#include <HTTPClient.h>
#include <WiFi.h>

const char *ssid = "Yos";
const char *password = "yoelmgs01";

const char *uri = "http://192.168.2.195:8000/api/print";
String receivedMessage;

unsigned long lastSensorReadTime = 0;
const int sendHttpReadInterval = 3000;

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600);

  checkWiFiConnection();

}

void loop() {

   if (millis() - lastSensorReadTime > sendHttpReadInterval) {
     
    checkWiFiConnection();
     
    lastSensorReadTime = millis();          // Catat waktu terakhir pembacaan data
  }

  if (Serial2.available()) {
    char receivedChar = Serial2.read();
    receivedMessage += receivedChar;

    if (receivedChar == '\n') {
      if (receivedMessage.equals("TES\n")) {
        Serial.println(receivedMessage);
        sendHttpRequest(uri);
      } else {
        Serial.print("Message : ");
        Serial.println(receivedMessage);
      }
      receivedMessage = "";  // Mengosongkan receivedMessage setelah pengolahan pesan
    }
  }
}

void sendHttpRequest(const char *uri) {
  HTTPClient http;
  http.begin(uri);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpCode = http.GET();

  if (httpCode == 200) {
    String response = http.getString();
    Serial.println("HTTP Code: " + String(httpCode));
    Serial.println("Response: " + response);
  } else if (httpCode == 404) {
    Serial.println("Not Found!");
  } else if (httpCode == 500) {
    Serial.println("Server Error");
  } else {
    Serial.println("Failed to make HTTP request");
  }
  http.end();
}

void checkWiFiConnection() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Not connected to WiFi, trying to connect...");
    WiFi.begin(ssid, password);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED) {
      if (millis() - start >= 5000) {
        Serial.println("Gagal Menghubungkan ke Wifi!");
        Serial.println("Mode Offline");
        break;
      }
      Serial.println("Connecting to WiFi...");
      delay(500);
    }
    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Successfully connected to wifi");
    } else {
      Serial.println("Failed to connect to WiFi");
    }
  }
}