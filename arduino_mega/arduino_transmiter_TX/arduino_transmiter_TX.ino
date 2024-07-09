#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

int parkir[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
LiquidCrystal_I2C lcd(0x27, 20, 4);

const int pmasuk = 33;
const int pkeluar = 22;

int jumlahSlotKosong = 0;
int slotParkir = 0;

// Tombol Tiket Masuk
const int buttonPin = 26;            // contoh pin yang digunakan untuk tombol
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

  for (int i = 0; i < 9; i++) {
    pinMode(parkir[i], INPUT_PULLUP);
  }

  pinMode(pmasuk, INPUT_PULLUP);
  pinMode(pkeluar, INPUT_PULLUP);
  pinMode(buttonPin, INPUT_PULLUP);
}


void loop() {
  int pintumasuk = debounceRead(pmasuk);
  int pintukeluar = debounceRead(pkeluar);

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    jumlahSlotKosong = 0;

    int statusParkir[10];

    for (int i = 0; i < 10; i++) {
      statusParkir[i] = digitalRead(parkir[i]) == HIGH ? 1 : 0;
      if (statusParkir[i] == 1) {
        jumlahSlotKosong++;
      }
    }

    // Serial.println("");
    // Serial.print("Slot Kosong Real : ");
    // Serial.println(jumlahSlotKosong);

    // Serial.print("Status Hitung    : ");
    // Serial.println(hitung);

    if (hitung) {
      slotParkir = jumlahSlotKosong - 1;
      // Serial.print("slotParkir       : ");
      // Serial.println(slotParkir);
    } else {
      slotParkir = jumlahSlotKosong;
      // Serial.print("slotParkir       : ");
      // Serial.println(slotParkir);
    }

    lcd.setCursor(2, 0);
    lcd.print("-Selamat Datang-");

    lcd.setCursor(2, 2);
    lcd.print("Parkir Tersedia:");

    if (jumlahSlotKosong == 0) {
      lcd.setCursor(8, 3);
      lcd.print("FULL");
    } else {
      lcd.setCursor(8, 3);
      lcd.print("    ");
      lcd.setCursor(9, 3);
      lcd.print(slotParkir);
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
        String pesan = "OPEN\n";
        Serial2.print(pesan);
        buttonPressed = false;  // Reset status tombol yang ditekan
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
