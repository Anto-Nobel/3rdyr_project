#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <SPI.h>
#include <Wire.h> 
#include <ArduinoJson.h>
#include <LoRa.h>
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
#include "time.h"

#define ss 5
#define rst 14
#define dio0 2

#define WIFI_SSID "nobel"
#define WIFI_PASSWORD "sarobert"

#define API_KEY "AIzaSyBA-pMWgqxB4EnSzo8WSan63KxDjbim5f8"
#define DATABASE_URL "https://eyantra-a53dd-default-rtdb.asia-southeast1.firebasedatabase.app/"  

#define USER_EMAIL "dqwrqreho@gmail.com"
#define USER_PASSWORD "ImITa@2021"

char array[7];
char *smsg[2]; 
char *ptr=NULL; 

const int ms=27; 
int mod=0;
char a[10];

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK=false;
String uid;

//NTP setup
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 3600;

float temp,pres;

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
    while(!Serial);
    LoRa.setPins(ss, rst, dio0);
    while (!LoRa.begin(433E6)) {
    Serial.println(".");
    delay(500);
    }
    LoRa.setSyncWord(0xF3);
    Serial.println("LoRa Initializing OK!");
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

void loop(){
    int packetSize = LoRa.parsePacket();
  if (packetSize) {
    while(LoRa.available()){
    strcpy(array,LoRa.readString().c_str());
    ptr=strtok(array,"="); 
    smsg[0]=ptr; 
    ptr=strtok(NULL,"=");
    if(ptr!=NULL){smsg[1]=ptr;} 
    Serial.println(smsg[0]); 
    Serial.println(smsg[1]);
    temp = smsg[0].toFloat();
    pres = smsg[1].toFloat();
    Serial.println(temp); 
    Serial.println(pres);
    }/*
    if (Firebase.ready() && signupOK){
    sendDataPrevMillis = millis();
    String timeStamp=tellTime();
    String currDate=tellDate();
    FirebaseJson json1;
    if(!(Firebase.RTDB.getJSON(&fbdo,"sensor_1/temperature/"+currDate)))
    {
      json1.set(currDate+"/"+timeStamp,temp);
      Serial.printf("Set json... %s\n", Firebase.RTDB.updateNode(&fbdo, F("/sensor_1/temperature"), &json1) ? "ok" : fbdo.errorReason().c_str());
    }
    else
    {
      json1.add(timeStamp,temp);
      Serial.printf("Update node... %s\n", Firebase.RTDB.updateNode(&fbdo, "sensor_1/temperature/"+currDate, &json1) ? "ok" : fbdo.errorReason().c_str());
    }

    FirebaseJson json2;
    if(!(Firebase.RTDB.getJSON(&fbdo,"sensor_1/pressure/"+currDate)))
    {
      json2.set(currDate+"/"+timeStamp,pres);
      Serial.printf("Set json... %s\n", Firebase.RTDB.updateNode(&fbdo, F("/sensor_1/pressure"), &json2) ? "ok" : fbdo.errorReason().c_str());
    }
    else
    {
      json2.add(timeStamp,pres);
      Serial.printf("Update node... %s\n", Firebase.RTDB.updateNode(&fbdo, "sensor_1/pressure/"+currDate, &json2) ? "ok" : fbdo.errorReason().c_str());
    }
  }*/
  }
}