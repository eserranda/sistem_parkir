#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

int parkir[] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };
LiquidCrystal_I2C lcd(0x27, 20, 4);

const int pmasuk = 22;
const int pkeluar = 23;

int jumlahSlotKosong = 0;
int slotParkir = 0;

bool hitung = false;


Servo servo1;
Servo servo2;

void setup() {
  Serial.begin(9600);
  lcd.begin();
  lcd.backlight();
  lcd.clear();

  servo1.attach(24);
  servo2.attach(25);
  servo1.write(0);
  servo2.write(0);


  for (int i = 0; i < 9; i++) {
    pinMode(parkir[i], INPUT);
  }

  pinMode(pmasuk, INPUT);
  pinMode(pkeluar, INPUT);
}

void loop() {
  int pintumasuk = digitalRead(pmasuk);
  int pintukeluar = digitalRead(pkeluar);

  jumlahSlotKosong = 0;

  int statusParkir[10];

  for (int i = 0; i < 10; i++) {
    statusParkir[i] = digitalRead(parkir[i]) == HIGH ? 1 : 0;
    if (statusParkir[i] == 1) {
      jumlahSlotKosong++;
    }  
  }

  // Serial.println("Slot Parkir yang Masih Kosong:");
  // for (int i = 0; i < 10; i++) {
  //   if (statusParkir[i] == 1) {
  //     Serial.print("Kosong : ");
  //     Serial.println(i + 1);
  //   }
  // }


  if (pintumasuk == LOW) {
    Serial.println("Pintu Masuk Terbuka!");
    hitung = true;
    servo1.attach(24);
    servo1.write(90);
    delay(500);
    servo1.detach();
  } else {
    servo1.attach(24);
    servo1.write(0);
    delay(500);
    servo1.detach();
  }

  if (pintukeluar == LOW) {
    servo2.attach(25);
    servo2.write(90);
    delay(500);
    servo2.detach();
  } else {
    servo2.attach(25);
    servo2.write(0);
    delay(500);
    servo2.detach();
  }

  Serial.println("");
  Serial.print("Slot Kosong Real : ");
  Serial.println(jumlahSlotKosong);

  Serial.print("Status Hitung    : ");
  Serial.println(hitung);

  if (hitung) {
    slotParkir = jumlahSlotKosong - 1;
    Serial.print("slotParkir       : ");
    Serial.println(slotParkir);
  } else if (!hitung) {
    slotParkir = jumlahSlotKosong ;
    Serial.print("slotParkir       : ");
    Serial.println(slotParkir);
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


  delay(1000);
}
