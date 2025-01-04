#include <WiFiS3.h>
#include "WifiPribadi.h"
#include <SPI.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SSD1306_I2C_ADDRESS 0x3c
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Definisikan pin dan tipe sensor
#define DHTPIN 4          // Pin data DHT22
#define DHTTYPE DHT22     // Tipe DHT
DHT dht(DHTPIN, DHTTYPE); // Inisialisasi DHT

const int flameSensorPin = 7; // Pin digital untuk sensor api
int mq2Pin = A0; // Pin analog untuk sensor MQ-2

char ssid[] = SECRET_SSID;        //SSID Wifi
char pass[] = SECRET_PASS;    //Password Wifi
int status = WL_IDLE_STATUS;     //Status wifi

void setup() {
  pinMode(flameSensorPin, INPUT); // Pin sensor sebagai input
  Serial.begin(9600);
   // Mulai DHT sensor
  dht.begin();
  
  display.begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS);
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  
}

void loop() {
  // Membaca nilai dari sensor MQ-2
  int mq2Value = analogRead(mq2Pin);

   // Membaca nilai dari sensor api
  int flameSensorValue = digitalRead(flameSensorPin);

  // Membaca kelembapan dan suhu
  float h = dht.readHumidity();
  float t = dht.readTemperature(); // Celsius
  // float f = dht.readTemperature(true); // Fahrenheit

  IPAddress ip = WiFi.localIP();

  if (isnan(h) || isnan(t)) {
    Serial.println("Gagal membaca dari DHT sensor!");
    display.clearDisplay();
    display.setCursor(0, 18);
    display.println("Gagal membaca");
    display.display();
    return;
  }
  
  // Jika api terdeteksi (nilai HIGH)
  if (flameSensorValue == LOW) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Api terdeteksi!");
    display.setCursor(0, 9);
    display.print("MQ-2 Value: ");
    display.println(mq2Value);
    display.setCursor(0, 18);
    display.print("Kelembapan: ");
    display.print(h);
    display.println(" %");
    display.print("Suhu: ");
    display.print(t);
    display.println(" *C");
    display.display();
    delay(500);
  }
  else {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Tidak ada api!");
    display.setCursor(0, 9);
    display.print("MQ-2 Value: ");
    display.println(mq2Value);
    display.setCursor(0, 18);
    display.print("Kelembapan: ");
    display.print(h);
    display.println(" %");
    display.print("Suhu: ");
    display.print(t);
    display.println(" *C");
    display.display();
    delay(500);
  }
  Serial.print("IP Address: ");
  Serial.println(ip);
  // Menampilkan nilai sensor ke Serial Monitor
  Serial.print("Flame Sensor Value: ");
  Serial.println(flameSensorValue);
  Serial.print("MQ2 Sensor Value  : ");
  Serial.println(mq2Value);
  delay(1000); // Tunggu 1 detik sebelum membaca lagi
}