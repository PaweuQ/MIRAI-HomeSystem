//BASIC INFORMATION
// Program: External weather station
// Author:  Paweł Bartkiewicz 
// email:   15g.pawel.bart@gmail.com
// github:  PaweuQ
// licence: MIT 

//ADDITIONAL INFORMATION
//Libraries to install: OneWire, DallasTemperature, DHT sensor library by Adafruit

#include <WiFi.h>
#include "time.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"
#include <SPI.h>
#include <SD.h>

// MicroSD card module settings
#define SD_CS 5
// MISO - GPIO 19
// MOSI - GPIO 23
// SCK - GPIO 18
// CS - GPIO 5
File dataFile;

// ==== Logging interval ====
unsigned long lastLogTime = 0;
const unsigned long logInterval = 60000; // 1 minute in milliseconds

// WiFi settings
const char* ssid     = "xyz";
const char* password = "xyz";

// NTP server and time settings
const char* ntpServer = "pool.ntp.org"; // NTP server address
const long  gmtOffset_sec = 3600;       // Offset from GMT in seconds (example: +1 hour)
const int   daylightOffset_sec = 3600;  // Daylight saving offset in seconds (if applicable)

// Rain module settings 
const int RAIN_analog = 34; // Analog input from sensor (A0 on module -> GPIO34 on ESP32)
const int RAIN_digital = 25; // Digital input from sensor (D0 on module -> GPIO25 on ESP32)

// DS18B20 settings
OneWire one(13);
DallasTemperature temp(&one);
float tempDS = 0;

// DHT settings 
#define DHTPIN 22        // GPIO pin for DTH data
#define DHTTYPE DHT11    // sensor type
DHT dht(DHTPIN, DHTTYPE); 

char filename[20];
void generateFilename(struct tm timeinfo);

void setup() {

  Serial.begin(115200);
  
  temp.begin();
  dht.begin();
  pinMode(RAIN_digital, INPUT);

  // Connect to Wi-Fi
  Serial.printf("Connecting to %s", ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");

  // Configure time via NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("Time synchronized with NTP server.");

  SD.begin(SD_CS);
  if (!SD.begin(SD_CS)) {
    Serial.println("SD card initialization failed!");
    while (1);
  }
  Serial.println("SD card initialized.");
  // Create file if it doesn't exist
  if (!SD.exists("/data.csv")) {
    dataFile = SD.open("/data.csv", FILE_WRITE);
    if (dataFile) {
      dataFile.println("Temperature");
      dataFile.close();
    } else {
      Serial.println("Failed to create file!");
    }
  }

  
}

void loop() {

  if (millis() - lastLogTime >= logInterval) {
    lastLogTime = millis();
    logData();
  }
}

void logData() {

  // Create a time structure
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to get time");
    return;
  }

  // Get the current time
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    delay(2000);
    return;
  }
  generateFilename(timeinfo);

  int DTH_hum = dht.readHumidity();     // %
  float DTH_temp = dht.readTemperature(); // default °C

  temp.requestTemperatures();
  tempDS = temp.getTempCByIndex(0);

  int RAIN_analog_value= analogRead(RAIN_analog);  // Read analog value (0 - 4095 for ESP32 ADC)
  int RAIN_digital_value = digitalRead(RAIN_digital); // Read digital value from comparator (1 or 0 depending on water detection)

  // monitoring for DTH sensor failures 
  if (isnan(DTH_hum) || isnan(DTH_temp)) {
    Serial.println(F("❌  Failed to read from DHT sensor!"));
    return;
  }

  // Print formatted time
  Serial.printf("%04d-%02d-%02d %02d:%02d:%02d\n",
                timeinfo.tm_year + 1900,
                timeinfo.tm_mon + 1,
                timeinfo.tm_mday,
                timeinfo.tm_hour,
                timeinfo.tm_min,
                timeinfo.tm_sec);

  Serial.print(F("DHT Humidity: "));
  Serial.print(DTH_hum, 1);
  Serial.print(F(" %\t"));
  Serial.print(F("DHT Temperature: "));
  Serial.print(DTH_temp, 1);
  Serial.println(F(" °C")); 

  Serial.print("DS18B20 Temperature: ");
  Serial.print(tempDS);
  Serial.println("°C");

  Serial.print("Analog: ");
  Serial.print(RAIN_analog_value);
  Serial.print("  |  Digital: ");
  Serial.print(RAIN_digital_value);

  // Weather condition based on analog value
  if (RAIN_analog_value > 3500) {
    Serial.println("| It is not raining.");
  } 
  else if (RAIN_analog_value <= 3500 && RAIN_analog_value > 2000) {
    Serial.println("| Single drops of water detected.");
  } 
  else if (RAIN_analog_value <= 2000) {
    Serial.println("| It is raining.");
  }
  Serial.println(); // Blank line for readability

  // Open file in append mode
  dataFile = SD.open(filename, FILE_APPEND);
  if (dataFile) {
    dataFile.printf("%04d-%02d-%02d;%02d:%02d:%02d;%.2f;%.2f;%2d;%d\n",
                    timeinfo.tm_year + 1900,
                    timeinfo.tm_mon + 1,
                    timeinfo.tm_mday,
                    timeinfo.tm_hour,
                    timeinfo.tm_min,
                    timeinfo.tm_sec,
                    tempDS,
                    DTH_temp,
                    DTH_hum,
                    RAIN_analog_value);
    dataFile.close();
    /*
    Serial.print("Logged: ");
    Serial.print(DTH_temp);
    Serial.println(" °C");
    */
  }  else {
    Serial.println("Error opening data.csv");
  }
  

  delay(5000);
}

void generateFilename(struct tm timeinfo) {
  sprintf(filename, "/%04d-%02d-%02d.csv",
          timeinfo.tm_year + 1900,
          timeinfo.tm_mon + 1,
          timeinfo.tm_mday);
}