#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);
int sensorPins[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

// Jumlah sensor
const int numSensors = 10;

// Variabel untuk menyimpan status sensor sebelumnya
int previousSensorStatus[10];

const int pmasuk = 33;

// tombil untuk mwembuka portal keluar
const int pkeluar = 27;

// Tombol Tiket Masuk
const int buttonPin = 26;            //  pin yang digunakan untuk tombol tiket masuk
int buttonState = HIGH;              // Inisialisasi ke HIGH agar tidak terbaca input palsu saat awalnya
unsigned long lastDebounceTime = 0;  // Waktu terakhir tombol ditekan
unsigned long debounceDelay = 50;    // Waktu debouncing, dalam milidetik
bool buttonPressed = false;          // Status tombol yang sedang ditekan

bool hitung = false;
String receivedMessage;

bool statusPortalMasuk = false;
bool statusPortalKeluar = false;

Servo servo1;
Servo servo2;

int pintumasukPrev = HIGH;   // Nilai awal adalah HIGH karena sensor tidak mendeteksi benda
int pintukeluarPrev = HIGH;  // Nilai awal adalah HIGH karena sensor tidak mendeteksi benda

const unsigned long interval = 2000;  // interval waktu dalam milidetik (2 detik)
unsigned long previousMillis = 0;     // menyimpan waktu terakhir pengecekan



void setup() {
  Serial.begin(9600);
  Serial2.begin(9600);  //RX2 = 17, TX2 = 16

  lcd.begin();
  lcd.backlight();
  lcd.clear();

  servo1.attach(24);
  servo2.attach(25);
  servo1.write(0);
  servo2.write(0);

  // Inisialisasi pin sensor sebagai input
  for (int i = 0; i < numSensors; i++) {
    pinMode(sensorPins[i], INPUT_PULLUP);
    previousSensorStatus[i] = HIGH;
  }

  // pinMode(pmasuk, INPUT_PULLUP);
  pinMode(pkeluar, INPUT_PULLUP);
  pinMode(buttonPin, INPUT_PULLUP);

  lcd.setCursor(2, 0);
  lcd.print("-Selamat Datang-");

  lcd.setCursor(2, 2);
  lcd.print("Parkir Tersedia:");
}


void loop() {
  int pintumasuk = debounceRead(pmasuk);
  int pintukeluar = debounceRead(pkeluar);

  int slotKosong;
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    int emptySlots = 0;

    // Variabel untuk menyimpan slot parkir yang kosong
    String emptySlotList = "";

    // Variabel untuk mendeteksi apakah ada perubahan status
    bool statusChanged = false;

    // Baca nilai dari setiap sensor
    for (int i = 0; i < numSensors; i++) {
      // Baca nilai dari sensor
      int sensorValue = digitalRead(sensorPins[i]);

      // Cek apakah sensor tidak mendeteksi benda (active high)
      if (sensorValue == HIGH) {
        emptySlots++;
        emptySlotList += String(i + 1) + " ";
      }

      // Cek perubahan status sensor dari HIGH ke LOW atau LOW ke HIGH
      if ((sensorValue == LOW && previousSensorStatus[i] == HIGH) || (sensorValue == HIGH && previousSensorStatus[i] == LOW)) {
        statusChanged = true;
      }

      previousSensorStatus[i] = sensorValue;  // Perbarui status sensor sebelumnya
    }

    // Panggil sensorDetected jika ada perubahan status
    if (statusChanged) {
      slotKosong = emptySlots;
      sensorDetected(emptySlots, emptySlotList);
    }
  }

  int readPkeluar = digitalRead(pkeluar);
  if (readPkeluar == 0) {
    Serial.println("Portal Tertutup");
    bukaPortalKeluar();
    while (true) {
      int readPkeluarAgain = digitalRead(pkeluar);
      if (readPkeluarAgain == 0) {
        Serial.println("Tombol ditekan lagi, menutup portal");
        tutupPortalKeluar();
        break;
      }
    }
  }

  unsigned long currentTime = millis();
  int reading = digitalRead(buttonPin);
  // Pengecekan apakah ada perubahan pada tombol
  if (reading != buttonState) {
    // Memperbarui waktu terakhir ketika tombol berubah
    lastDebounceTime = currentTime;
  }

  // Pengecekan apakah telah berlalu waktu debounce sejak tombol terakhir kali ditekan
  if ((currentTime - lastDebounceTime) > debounceDelay) {
    // Membaca input tombol saat waktu debounce telah berlalu
    if (reading == LOW) {
      // Tombol sedang ditekan
      buttonPressed = true;
    } else {
      // Tombol dilepas
      if (buttonPressed) {
        // Jika tombol sebelumnya ditekan, kirim pesan
        if (slotKosong == 0) {
          Serial.println("Full");
          buttonPressed = false;  // Reset status tombol yang ditekan
        } else {
          String pesan = "OPEN\n";
          Serial2.print(pesan);
          Serial.println(pesan);
          buttonPressed = false;  // Reset status tombol yang ditekan
        }
      }
    }
  }
  // Memperbarui status tombol terakhir
  buttonState = reading;

  if (Serial2.available()) {
    char receivedChar = Serial2.read();
    receivedMessage += receivedChar;

    if (receivedChar == '\n') {
      if (receivedMessage.equals("OK\n")) {
        Serial.println(receivedMessage);
        bukaPortalMasuk();
        statusPortalMasuk = true;
      } else if (receivedMessage.equals("buka\n")) {
        bukaPortalKeluar();
        statusPortalKeluar = true;
      } else if (receivedMessage.equals("tutup\n")) {
        tutupPortalKeluar();
      }
      receivedMessage = "";  // Mengosongkan receivedMessage setelah pengolahan pesan
    }
  }

  if (statusPortalMasuk && pintumasuk == HIGH && pintumasukPrev == LOW) {
    Serial.println("Portal Tertutup");
    tutupPortalMasuk();
    statusPortalMasuk = false;
  }
  pintumasukPrev = pintumasuk;  // Simpan nilai sensor pintumasuk untuk perbandingan berikutnya

  if (statusPortalKeluar && pintukeluar == HIGH && pintukeluarPrev == LOW) {
    String pesan = "auto_close\n";
    Serial2.print(pesan);

    Serial.println("Portal Tertutup");
    tutupPortalKeluar();
    statusPortalKeluar = false;
  }
  pintukeluarPrev = pintukeluar;  // Simpan nilai sensor pintukeluar untuk perbandingan berikutnya
}

void sensorDetected(int emptySlots, String emptySlotList) {
  Serial.print("Jumlah Slot Kosong: ");
  Serial.println(emptySlots);
  Serial.print("Slot Parkir Yang Kosong: ");
  Serial.println(emptySlotList);
  Serial.println("");

  if (emptySlots == 0) {
    lcd.setCursor(8, 3);
    lcd.print("FULL");

    lcd.setCursor(0, 1);
    lcd.print("  ");
  } else {
    lcd.clear();

    lcd.setCursor(2, 0);
    lcd.print("-Selamat Datang-");

    lcd.setCursor(2, 2);
    lcd.print("Parkir Tersedia:");

    lcd.setCursor(0, 1);
    lcd.print(emptySlotList);
    lcd.setCursor(9, 3);
    lcd.print(emptySlots);
  }

  if (emptySlots != 0) {
    // String message = String(emptySlots) + "|" + String(emptySlotList);
    String message = String(emptySlots) + "\n";
    Serial.println(message);
    Serial2.print(message);
  }
}


void bukaPortalKeluar() {
  servo2.attach(25);
  servo2.write(90);
  delay(500);
  servo2.detach();
}

void tutupPortalKeluar() {
  servo1.attach(25);
  servo1.write(0);
  delay(500);
  servo1.detach();
}

void bukaPortalMasuk() {
  servo1.attach(24);
  servo1.write(90);
  delay(500);
  servo1.detach();
}

void tutupPortalMasuk() {
  servo1.attach(24);
  servo1.write(0);
  delay(500);
  servo1.detach();
}

int debounceRead(int pin) {
  int reading = digitalRead(pin);
  delay(50);
  int newReading = digitalRead(pin);
  if (reading == newReading) {
    return reading;
  }
  return debounceRead(pin);  // Recurse until stable
}
