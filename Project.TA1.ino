#include <WiFiS3.h>
#include "WifiPribadi.h"
#include <SPI.h>
#include <Wire.h>
#include <DHT.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Firebase.h>

//Mendefinisikan variabel untuk Oled
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define SSD1306_I2C_ADDRESS 0x3c
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Definisikan pin dan tipe sensor DHT22
#define DHTPIN 4          // Pin data DHT22
#define DHTTYPE DHT22     // Tipe DHT
DHT dht(DHTPIN, DHTTYPE); // Inisialisasi DHT

// Pin digital untuk sensor api
const int flameSensorPin = 7;

// Definisikan pin dan tipe sensor MQ2
int mq2Pin = A0; // Pin analog untuk sensor MQ-2
#define RL_VALUE (5) //Menentukan resistansi dalam ohm
#define RO_CLEAN_AIR_FACTOR (9.83) //Nilai Udara bersih
#define GAS_LPG (0)
#define GAS_CO (1)
#define GAS_SMOKE (2)
float LPGCurve[3] = {2.3,0.21,-0.47};
float COCurve[3] = {2.3,0.72,-0.34};
float SmokeCurve[3] = {2.3,0.53,-0.44};
float Ro = 10;

//Menginput data wifi
char ssid[] = SECRET_SSID;        //SSID Wifi
char pass[] = SECRET_PASS;    //Password Wifi
int status = WL_IDLE_STATUS;     //Status wifi

//Menginput data Firebase
char host[] = FIREBASE_HOST;
char auth[] = FIREBASE_AUTH;
Firebase fb(host,auth);

void setup() {
  pinMode(flameSensorPin, INPUT); // Pin sensor sebagai input
  Serial.begin(9600);
  while(!Serial){
    ;
  }
  // Menghubungkan ke Firebase
  WiFi.begin(ssid, pass);

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
  float h = dht.readHumidity();    //Data Kelembapan dalam bentu Persen
  float t = dht.readTemperature(); //Data Suhu dalam bentu Celcius
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
    display.print("Suhu      : ");
    display.print(t);
    display.println(" C");
    display.print("Kelembapan: ");
    display.print(h);
    display.println(" %");
    display.print("MQ-2 Value: ");
    display.println(mq2Value);
    display.print("LPG       : ");
    display.print(MQGetGasPercentage(MQRead(mq2Pin)/Ro,GAS_LPG) );
    display.println(" ppm");
    display.print("CO        : "); 
    display.print(MQGetGasPercentage(MQRead(mq2Pin)/Ro,GAS_CO));
    display.println(" ppm");
    display.print("SMOKE     : "); 
    display.print(MQGetGasPercentage(MQRead(mq2Pin)/Ro,GAS_SMOKE));
    display.println(" ppm");
    display.println("Terdeteksi Api!");
    display.display();
    delay(100);
  } else {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Suhu      : ");
    display.print(t);
    display.println(" C");
    display.print("Kelembapan: ");
    display.print(h);
    display.println(" %");
    display.print("MQ-2 Value: ");
    display.println(mq2Value);
    display.print("LPG       : ");
    display.print(MQGetGasPercentage(MQRead(mq2Pin)/Ro,GAS_LPG));
    display.println(" ppm");
    display.print("CO        : "); 
    display.print(MQGetGasPercentage(MQRead(mq2Pin)/Ro,GAS_CO));
    display.println(" ppm");
    display.print("SMOKE     : "); 
    display.print(MQGetGasPercentage(MQRead(mq2Pin)/Ro,GAS_SMOKE));
    display.println(" ppm");
    display.println("Tidak Ada Api!");
    display.display();
    delay(100);
  }
  Serial.print("IP Address        : ");
  Serial.println(ip);
  // Menampilkan nilai sensor ke Serial Monitor
  Serial.print("Flame Sensor Value: ");
  Serial.println(flameSensorValue);
  Serial.print("MQ-2 Value        : ");
  Serial.println(mq2Value);
  Serial.print("LPG               : "); 
  Serial.print(MQGetGasPercentage(MQRead(mq2Pin)/Ro,GAS_LPG));
  Serial.println(" ppm");
  Serial.print("CO                : "); 
  Serial.print(MQGetGasPercentage(MQRead(mq2Pin)/Ro,GAS_CO));
  Serial.println(" ppm");
  Serial.print("SMOKE             : "); 
  Serial.print(MQGetGasPercentage(MQRead(mq2Pin)/Ro,GAS_SMOKE));
  Serial.println(" ppm");
  Serial.print("Suhu              : ");
  Serial.print(t);
  Serial.println(" C");
  Serial.print("Kelembapan        : ");
  Serial.print(h);
  Serial.println(" %");
  Serial.println("============================");
  delay(500); // Tunggu 1 detik sebelum membaca lagi
}

float MQResistanceCalculation(int raw_adc){
    return ( ((float)RL_VALUE*(1023-raw_adc)/raw_adc));
  } 
  float MQCalibration(int mq2Pin){
    int i;
    float val=0;
    for (i=0;i<50;i++) {            //take multiple samples
      val += MQResistanceCalculation(analogRead(mq2Pin));
      delay(500);
    }
    val = val/50;                   //Mengkalkulasi nilai rata-rata
    val = val/RO_CLEAN_AIR_FACTOR;  //divided by RO_CLEAN_AIR_FACTOR yields the Ro according to the chart in the datasheet 
    return val; 
  } 
  float MQRead(int mq2Pin){
    int i;
    float rs=0;
    for (i=0;i<5;i++) {
      rs += MQResistanceCalculation(analogRead(mq2Pin));
      delay(50);
    }
    rs = rs/5;
    return rs;  
  }
  int MQGetGasPercentage(float rs_ro_ratio, int gas_id){
    if ( gas_id == GAS_LPG ) {
      return MQGetPercentage(rs_ro_ratio,LPGCurve);
    } else if ( gas_id == GAS_CO ) {
      return MQGetPercentage(rs_ro_ratio,COCurve);
    } else if ( gas_id == GAS_SMOKE ) {
      return MQGetPercentage(rs_ro_ratio,SmokeCurve);
    }    
    return 0;
  }
  int  MQGetPercentage(float rs_ro_ratio, float *pcurve){
    return (pow(10,( ((log(rs_ro_ratio)-pcurve[1])/pcurve[2]) + pcurve[0])));
  }
