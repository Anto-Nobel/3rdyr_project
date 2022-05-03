#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

#include <SPI.h> 
#include <Wire.h> 
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <ArduinoJson.h>
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
#include "time.h"

#define ss 5
#define rst 14
#define dio0 2

#define BMP_SCK (13); 
#define BMP_MISO (12); 
#define BMP_MOSI (11); 
#define BMP_CS (10);

Adafruit_BMP280 bmp; 

// Insert your network credentials

#define WIFI_SSID "nobel"

#define WIFI_PASSWORD "sarobert"

// Insert Firebase project API Key

#define API_KEY "AIzaSyBA-pMWgqxB4EnSzo8WSan63KxDjbim5f8"



// Insert RTDB URLefine the RTDB URL */

#define DATABASE_URL "https://eyantra-a53dd-default-rtdb.asia-southeast1.firebasedatabase.app/"  

#define USER_EMAIL "dqwrqreho@gmail.com"
#define USER_PASSWORD "ImITa@2021"

//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK=false;
String uid;

DHT dht(DHTPIN, DHTTYPE);
unsigned long sendDataPrevMillis = 0;

//NTP setup
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 3600;

void initWiFi(){
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void setup(){
  Serial.begin(115200);
  
  initWiFi();

if (!bmp.begin(0x76)) {
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
  }

  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  config.api_key = API_KEY;
  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;
  // Assign the user sign in credentials
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  Firebase.reconnectWiFi(true);
  fbdo.setResponseSize(4096);

  // Assign the callback function for the long running token generation task
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  // Assign the maximum retry of token generation
  config.max_token_generation_retry = 5;

  // Initialize the library with the Firebase authen and config
  Firebase.begin(&config, &auth);

  // Getting the user UID might take a few seconds
  Serial.println("Getting User UID");
  while ((auth.token.uid) == "") {
    Serial.print('.');
    delay(1000);
  } 
  signupOK = true;
  // Print user UID
  uid = auth.token.uid.c_str();
  Serial.print("User UID: ");
  Serial.print(uid);
}

String convertToString(char* a, int size)
{
    int i;
    String s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
}

String tellTime()
{
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  char a[22];
  strftime(a,22, "%X", &timeinfo);
  String t=convertToString(a,22);
  return t;
}

String tellDate()
{
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  char a[22];
  strftime(a,22, "%d-%m-%Y", &timeinfo);
  String t=convertToString(a,22);
  return t;
} 

void loop()
{
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 30000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    String timeStamp=tellTime();
    String currDate=tellDate();
    FirebaseJson json1;
    if(!(Firebase.RTDB.getJSON(&fbdo,"sensor_1/temperature/"+currDate)))
    {
      json1.set(currDate+"/"+timeStamp,dht.readTemperature());
      Serial.printf("Set json... %s\n", Firebase.RTDB.updateNode(&fbdo, F("/sensor_1/temperature"), &json1) ? "ok" : fbdo.errorReason().c_str());
    }
    else
    {
      json1.add(timeStamp,dht.readTemperature());
      Serial.printf("Update node... %s\n", Firebase.RTDB.updateNode(&fbdo, "sensor_1/temperature/"+currDate, &json1) ? "ok" : fbdo.errorReason().c_str());
    }

    FirebaseJson json2;
    if(!(Firebase.RTDB.getJSON(&fbdo,"sensor_1/humidity/"+currDate)))
    {
      json2.set(currDate+"/"+timeStamp,dht.readHumidity());
      Serial.printf("Set json... %s\n", Firebase.RTDB.updateNode(&fbdo, F("/sensor_1/humidity"), &json2) ? "ok" : fbdo.errorReason().c_str());
    }
    else
    {
      json2.add(timeStamp,dht.readHumidity());
      Serial.printf("Update node... %s\n", Firebase.RTDB.updateNode(&fbdo, "sensor_1/humidity/"+currDate, &json2) ? "ok" : fbdo.errorReason().c_str());
    }

  }

}
