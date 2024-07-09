#include <HTTPClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>


const char *ssid = "Yos";
const char *password = "yoelmgs01";

const char *uri = "http://192.168.2.195:8000/api/print";
const char *status = "http://192.168.2.195:8000/api/status";
const char *update_status = "http://192.168.2.195:8000/api/update-status";

String receivedMessage;

unsigned long lastCheck = 0;
const int timeInterval = 3000;

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600);

  checkWiFiConnection();
}

void loop() {
  if (millis() - lastCheck > timeInterval) {
    checkWiFiConnection();
    cekStatus(status);
    lastCheck = millis();  // Catat waktu terakhir pembacaan data
  }

  if (Serial2.available()) {
    char receivedChar = Serial2.read();
    receivedMessage += receivedChar;

    if (receivedChar == '\n') {
      if (receivedMessage.equals("OPEN\n")) {
        Serial.println(receivedMessage);
        sendHttpRequest(uri);
      } else if (receivedMessage.equals("auto_close\n")) {
        Serial.println(receivedMessage);
        updateStatus(update_status);
      } else {
        Serial.print("Message : ");
        Serial.println(receivedMessage);
      }
      receivedMessage = "";  // Mengosongkan receivedMessage setelah pengolahan pesan
    }
  }
}

void updateStatus(const char *update_status) {
  HTTPClient http;
  http.begin(update_status);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");
  String status = "tutup";
  int httpCode = http.POST("status=" + String(status));
  if (httpCode > 0) {
    String response = http.getString();
    Serial.println("HTTP Code: " + String(httpCode));
    Serial.println("Response: " + response);

DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, response);
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.f_str());
      return;
    }

    String status = doc["status"];
    if (status == "tutup") {
      String pesan = "tutup\n";
      Serial2.print(pesan);
    }  

  } else if (httpCode == 404) {
    Serial.println("Not Found!");
  } else if (httpCode == 500) {
    Serial.println("Server Error");
  } else {
    Serial.println("Failed to make HTTP request");
  }
  http.end();
}

void cekStatus(const char *status) {
  HTTPClient http;
  http.begin(status);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpCode = http.GET();

  if (httpCode == 200 || httpCode == 201) {
    String response = http.getString();
    Serial.println("HTTP Code: " + String(httpCode));
    Serial.println("Response: " + response);

    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, response);
    if (error) {
      Serial.print("deserializeJson() failed: ");
      Serial.println(error.f_str());
      return;
    }

    String status = doc["status"];
    if (status == "buka") {
      String pesan = "buka\n";
      Serial2.print(pesan);
    } else if (status == "tutup") {
      String pesan = "tutup\n";
      Serial2.print(pesan);
    }
    // else if (status == "auto_close") {
    //   String pesan = "auto_close\n";
    //   Serial2.print(auto_close);
    // }


  } else if (httpCode == 404) {
    Serial.println("Not Found!");
  } else if (httpCode == 500) {
    Serial.println("Server Error");
  } else {
    Serial.println("Failed to make HTTP request");
  }
  http.end();
}


void sendHttpRequest(const char *uri) {
  HTTPClient http;
  http.begin(uri);
  http.addHeader("Content-Type", "application/x-www-form-urlencoded");

  int httpCode = http.GET();

  if (httpCode == 200 || httpCode == 201) {
    String response = http.getString();
    Serial.println("HTTP Code: " + String(httpCode));
    Serial.println("Response: " + response);

    String pesan = "OK\n";
    Serial2.print(pesan);

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