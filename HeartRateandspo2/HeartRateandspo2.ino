#if defined(ESP32) 
#include <WiFiMulti.h> 
WiFiMulti wifiMulti; 
#define DEVICE "ESP32" 
#elif defined(ESP8266) 
#include <ESP8266WiFiMulti.h> 
ESP8266WiFiMulti wifiMulti; 
#define MODEL "HeartRate" 
#define VERSION "1" 
#endif 
//------------------------------
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"
#include "spo2_algorithm.h"

MAX30105 particleSensor;

const byte RATE_SIZE = 4; //Increase this for more averaging. 4 is good.
byte rates[RATE_SIZE]; //Array of heart rates
byte rateSpot = 0;
long lastBeat = 0; //Time at which the last beat occurred

float beatsPerMinute;
int beatAvg;

int32_t spo2; // SpO2 value
int8_t validSPO2; // Indicator to show if the SpO2 calculation is valid
int32_t heartRate; // Heart rate value
int8_t validHeartRate; // Indicator to show if the heart rate calculation is valid

uint32_t irBuffer[100]; // Buffer for IR sensor data
uint32_t redBuffer[100]; // Buffer for Red sensor data
//---------------------------------------------
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 32  // OLED display height, in pixels

#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
 
//------------------------------------------------------ 
#include <InfluxDbClient.h> 
#include <InfluxDbCloud.h> 
#include <Ticker.h>

#include <TimeLib.h>
#include <Time.h>

#include <DNSServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#include <EEPROM.h>
ESP8266WebServer    server(80);

struct settings {
  char ssid[30];
  char password[30];
  char email[30];
} user_wifi = {};


const char* HOST = "cpeiot.thesaban.org";
const int PORT = 80;
const char* URL = "/iot/HeartRate";

Ticker  count_time1;
Ticker  count_time2;

const int interruptPin = D2;
const int modePin = D0;

const int bluePin = D1;
const int  redPin = D4;
const int  greenPin = D3;

volatile bool oldsos = true;
volatile bool inttimer = false;
volatile bool inttimer2 = false;

bool bluevalue = 0;
bool greenvalue = 1;
bool redvalue = 0;

int modeButton = 1;

int RUNMODE = 0;

int timezone = 7 * 3600;
int dst = 0; 

String datetime = "";

byte mac[6];
String str_mac = ""; 

// Change INFLUX CONFIG
#define INFLUXDB_URL "https://eu-central-1-1.aws.cloud2.influxdata.com"
#define INFLUXDB_TOKEN "pUSqccneRAiNvKTtiTdnhnlbYynyUcPLtffybhAaZ3Wp6FvlZz15IPu201jKaXfPctbW4iJGeaFZm4mrxgZuGw=="
#define INFLUXDB_ORG "e266c442dcee6ac5"
#define INFLUXDB_BUCKET "THESABAN"
  
// Time zone info 
#define TZ_INFO "UTC7" 
 
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert); 
 
// Declare Data point 
Point sensor("HeartRate"); 

void c_time(){
 inttimer = true;
}

void c2_time(){
 inttimer2 = true;
}

void update(){
  WiFiClient client;
  Serial.println(F("Update start now!"));
  t_httpUpdate_return ret = ESPhttpUpdate.update(client, HOST, PORT, URL, VERSION);
    
    switch (ret) {
      case HTTP_UPDATE_FAILED:
        Serial.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str()); 
        break;
      case HTTP_UPDATE_NO_UPDATES:
        Serial.println("Your code is up to date!");
        break;
      case HTTP_UPDATE_OK:
        Serial.println("HTTP_UPDATE_OK");
        delay(1000); // Wait a second and restart
        ESP.restart();
        break;
    }
}

void handlePortal() {

  if (server.method() == HTTP_POST) {

    strncpy(user_wifi.ssid,     server.arg("ssid").c_str(),     sizeof(user_wifi.ssid) );
    strncpy(user_wifi.password, server.arg("password").c_str(), sizeof(user_wifi.password) );
    strncpy(user_wifi.email, server.arg("email").c_str(), sizeof(user_wifi.email) );
    user_wifi.ssid[server.arg("ssid").length()] = user_wifi.password[server.arg("password").length()] = user_wifi.email[server.arg("email").length()] = '\0';
    
    EEPROM.put(0, user_wifi);
    EEPROM.commit();

    server.send(200,   "text/html",  "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>Wifi Setup</title><style>*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{border:1px solid transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1,p{text-align: center}</style> </head> <body><main class='form-signin'> <h1>Wifi Setup</h1> <br/> <p>Your settings have been saved successfully!<br />Please change mode and restart the device.</p></main></body></html>" );
  } else {

    server.send(200,   "text/html", "<!doctype html><html lang='en'><head><meta charset='utf-8'><meta name='viewport' content='width=device-width, initial-scale=1'><title>Wifi Setup</title> <style>*,::after,::before{box-sizing:border-box;}body{margin:0;font-family:'Segoe UI',Roboto,'Helvetica Neue',Arial,'Noto Sans','Liberation Sans';font-size:1rem;font-weight:400;line-height:1.5;color:#212529;background-color:#f5f5f5;}.form-control{display:block;width:100%;height:calc(1.5em + .75rem + 2px);border:1px solid #ced4da;}button{cursor: pointer;border:1px solid transparent;color:#fff;background-color:#007bff;border-color:#007bff;padding:.5rem 1rem;font-size:1.25rem;line-height:1.5;border-radius:.3rem;width:100%}.form-signin{width:100%;max-width:400px;padding:15px;margin:auto;}h1{text-align: center}</style> </head> <body><main class='form-signin'> <form action='/' method='post'> <h1 class=''>Wifi Setup ("+str_mac+") </h1><br/><div class='form-floating'><label>SSID</label><input type='text' class='form-control' name='ssid'> </div><div class='form-floating'><br/><label>Password</label><input type='password' class='form-control' name='password'></div><br/><div class='form-floating'><label>Email</label><input type='text' class='form-control' name='email'> </div><br/><br/><button type='submit'>Save</button><p style='text-align: right'><a href='https://www.thesaban.org' style='color: #32C5FF'>thesaban.org</a></p></form></main> </body></html>" );
  }
}

String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}

void influx() {
  if (spo2 != -999 && heartRate != -999) {
    sensor.clearFields();
    sensor.addField("rssi", WiFi.RSSI());
    sensor.addField("spo2", spo2);
    sensor.addField("heartRate", heartRate);

    // Display "Sending value" on the OLED
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.print("Sending value...");
    display.display();

    Serial.println(sensor.toLineProtocol());
    if (!client.writePoint(sensor)) {
      Serial.print("InfluxDB write failed: ");
      Serial.println(client.getLastErrorMessage());
    }
  }
}
  


void influxRegister() {
  sensor.clearFields();
  sensor.addField("register", 1);
  sensor.addField("rssi", WiFi.RSSI()); 

  Serial.println(sensor.toLineProtocol()); 
  if (!client.writePoint(sensor)) { 
    Serial.print("InfluxDB write failed: "); 
    Serial.println(client.getLastErrorMessage()); 
  }
}

String getDate() {
  time_t now = time(nullptr);
  struct tm* p_tm = localtime(&now);
  char buffer[40];
  sprintf(buffer, "%u/%u/%u %u:%u:%u", p_tm->tm_mday, (p_tm->tm_mon + 1), (p_tm->tm_year + 1900), p_tm->tm_hour, p_tm->tm_min, p_tm->tm_sec);
  return buffer;
}

// void setup() {
//   EEPROM.begin(sizeof(struct settings));
//   EEPROM.get(0, user_wifi);

//   Serial.begin(115200);
//   Serial.println("");

//   if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
//     Serial.println(F("SSD1306 allocation failed"));
//     for (;;)
//       ;  // Don't proceed, loop forever
//   }  

//   if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) { //Use default I2C port, 400kHz speed
//     Serial.println("MAX30105 was not found. Please check wiring/power.");
//     while (1);
//   }
//   Serial.println("Place your index finger on the sensor with steady pressure.");

//   particleSensor.setup(); //Configure sensor with default settings
//   particleSensor.setPulseAmplitudeRed(0x0A); //Turn Red LED to low to indicate sensor is running
//   particleSensor.setPulseAmplitudeGreen(0); //Turn off Green LED

//   pinMode(modePin, INPUT_PULLUP);
//   pinMode(interruptPin, INPUT_PULLUP);
  
//   delay(250);
//   Serial.print("Mode Read: ");
//   Serial.println(digitalRead(modePin));

//   pinMode(bluePin, OUTPUT); 
//   pinMode(redPin, OUTPUT); 
//   pinMode(greenPin, OUTPUT); 

//   digitalWrite(bluePin, bluevalue); 
//   digitalWrite(redPin, redvalue); 
//   digitalWrite(greenPin, greenvalue); 

//   modeButton = digitalRead(modePin);

//   if (!modeButton) {
//     WiFi.macAddress(mac);
//     str_mac += macToStr(mac);

//     String APname = "CPEiot_" + str_mac;

//     Serial.println("WIFI AP MODE");
//     WiFi.softAP(APname, "mirot.digit");
    
//     Serial.println("HTTP server started");
//     server.on("/", handlePortal);
//     server.begin();
//     digitalWrite(bluePin, 1); 
//     digitalWrite(redPin, 1); 
//     digitalWrite(greenPin, 1); 
//     RUNMODE = 1;
//   } else {
//     WiFi.mode(WIFI_STA);
//     wifiMulti.addAP(user_wifi.ssid, user_wifi.password);
//     Serial.print(user_wifi.ssid);
//     Serial.print(user_wifi.password);
//     Serial.print("Connecting to wifi");
//     while (wifiMulti.run() != WL_CONNECTED) {
//       Serial.print(".");
//       for (int i = 0; i <= 4; i++) { 
//         digitalWrite(bluePin, 0); 
//         delay(200); 
//         digitalWrite(bluePin, 1); 
//         delay(200); 
//       }
//     } 
//     digitalWrite(bluePin, bluevalue);  
//     Serial.println();
//     timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

//     configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");

//     if (client.validateConnection()) {
//       Serial.print("Connected to InfluxDB: ");
//       Serial.println(client.getServerUrl());
//     } else {
//       Serial.print("InfluxDB connection failed: ");
//       Serial.println(client.getLastErrorMessage());
//     }

//     WiFi.macAddress(mac);
//     str_mac += macToStr(mac);
//     Serial.println(str_mac);
//     Serial.println(user_wifi.email);
//     sensor.addTag("model", MODEL);
//     sensor.addTag("version", VERSION);
//     sensor.addTag("device", str_mac);
//     sensor.addTag("email", user_wifi.email);

//     influxRegister();

//     count_time1.attach(2, c_time);     //Interrupt every 2 seconds
//     count_time2.attach(10, c2_time);   //Interrupt every 30 seconds
//   }
// }

void setup() {
  EEPROM.begin(sizeof(struct settings));
  EEPROM.get(0, user_wifi);

  Serial.begin(115200);
  Serial.println("");

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }

  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) { // Use default I2C port, 400kHz speed
    Serial.println("MAX30105 was not found. Please check wiring/power.");
    while (1);
  }
  Serial.println("Place your index finger on the sensor with steady pressure.");

  particleSensor.setup(); // Configure sensor with default settings
  particleSensor.setPulseAmplitudeRed(0x0A); // Turn Red LED to low to indicate sensor is running
  particleSensor.setPulseAmplitudeGreen(0); // Turn off Green LED

  pinMode(modePin, INPUT_PULLUP);
  pinMode(interruptPin, INPUT_PULLUP);

  delay(250);
  Serial.print("Mode Read: ");
  Serial.println(digitalRead(modePin));

  pinMode(bluePin, OUTPUT); 
  pinMode(redPin, OUTPUT); 
  pinMode(greenPin, OUTPUT); 

  digitalWrite(bluePin, bluevalue); 
  digitalWrite(redPin, redvalue); 
  digitalWrite(greenPin, greenvalue); 

  modeButton = digitalRead(modePin);

  if (!modeButton) {
    WiFi.macAddress(mac);
    str_mac += macToStr(mac);

    String APname = "CPEiot_" + str_mac;

    Serial.println("WIFI AP MODE");
    WiFi.softAP(APname, "mirot.digit");

    Serial.println("HTTP server started");
    server.on("/", handlePortal);
    server.begin();
    digitalWrite(bluePin, 1); 
    digitalWrite(redPin, 1); 
    digitalWrite(greenPin, 1); 
    RUNMODE = 1;
  } else {
    WiFi.mode(WIFI_STA);
    wifiMulti.addAP(user_wifi.ssid, user_wifi.password);
    
    // Display message on OLED while connecting to WiFi
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.print("Connecting to WiFi");
    display.display();

    Serial.print("Connecting to WiFi");
    while (wifiMulti.run() != WL_CONNECTED) {
      Serial.print(".");
      for (int i = 0; i <= 4; i++) { 
        digitalWrite(bluePin, 0); 
        delay(200); 
        digitalWrite(bluePin, 1); 
        delay(200); 
      }
    } 
    digitalWrite(bluePin, bluevalue);  
    
    // Clear display after connecting
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    display.print("Connected");
    display.display();

    delay(2000); // Display "Connected" message for 2 seconds

    // Update display with heart rate and SpO2 values
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);

    display.setCursor(0, 10);
    display.print("Heart: ");
    if (heartRate == -999 || heartRate == 0) {
      display.print("Processing..");
    } else {
      display.print(heartRate);
    }

    display.setCursor(0, 20);
    display.print("SpO2: ");
    if (spo2 == -999 || spo2 == 0 ) {
      display.print("Processing..");
    } else {
      display.print(spo2);
    }

    display.display();

    Serial.println();
    timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

    configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");

    if (client.validateConnection()) {
      Serial.print("Connected to InfluxDB: ");
      Serial.println(client.getServerUrl());
    } else {
      Serial.print("InfluxDB connection failed: ");
      Serial.println(client.getLastErrorMessage());
    }

    WiFi.macAddress(mac);
    str_mac += macToStr(mac);
    Serial.println(str_mac);
    Serial.println(user_wifi.email);
    sensor.addTag("model", MODEL);
    sensor.addTag("version", VERSION);
    sensor.addTag("device", str_mac);
    sensor.addTag("email", user_wifi.email);

    influxRegister();

    count_time1.attach(2, c_time);     // Interrupt every 2 seconds
    count_time2.attach(15, c2_time);   // Interrupt every 10 seconds
  }
}


void loop() {
  long irValue = particleSensor.getIR();

  // ตรวจสอบการมีนิ้วบนเซนเซอร์
  if (irValue < 50000) { // กำหนดขีดจำกัดที่เหมาะสมสำหรับการตรวจจับการวางนิ้ว
    heartRate = -999;
    spo2 = -999;
  } else {
    if (checkForBeat(irValue) == true) {
      long delta = millis() - lastBeat;
      lastBeat = millis();

      beatsPerMinute = 60 / (delta / 1000.0);

      if (beatsPerMinute < 255 && beatsPerMinute > 20) {
        rates[rateSpot++] = (byte)beatsPerMinute;
        rateSpot %= RATE_SIZE;

        beatAvg = 0;
        for (byte x = 0; x < RATE_SIZE; x++)
          beatAvg += rates[x];
        beatAvg /= RATE_SIZE;
      }
    }

    for (byte i = 0; i < 100; i++) {
      while (particleSensor.available() == false)
        particleSensor.check();

      redBuffer[i] = particleSensor.getRed();
      irBuffer[i] = particleSensor.getIR();
      particleSensor.nextSample();
    }

    maxim_heart_rate_and_oxygen_saturation(irBuffer, 100, redBuffer, &spo2, &validSPO2, &heartRate, &validHeartRate);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  display.setCursor(0, 10);
  display.print("Heart: ");
  if (heartRate == -999) {
    display.print(".....");
  } else {
    display.print(heartRate);
  }

  display.setCursor(0, 20);
  display.print("SpO2: ");
  if (spo2 == -999) {
    display.print(".....");
  } else {
    display.print(spo2);
  }

  display.display();

  if (RUNMODE == 1) {
    server.handleClient();
  } else {
    if (wifiMulti.run() != WL_CONNECTED) {
      Serial.println("WiFi connection lost");

      display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, 10);
        display.print("WiFi connection lost");
        display.display();
        
      for (int i = 0; i <= 4; i++) {
        digitalWrite(bluePin, 0);
        delay(200);
        digitalWrite(bluePin, 1);
        delay(200);
      }
    } else {
      digitalWrite(bluePin, 0);

      if (inttimer) {
        greenvalue = !greenvalue;
        digitalWrite(greenPin, greenvalue);
        inttimer = false;
      }

      if (inttimer2) {
        update();
        influx();
        inttimer2 = false;
      }
    }
  }

  
}
