#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "time.h"

#define WIFI_SSID "NaN"
#define WIFI_PASSWORD "antonobel"
#define API_KEY "AIzaSyA-u_CqSZoLKyeHxeN4qNzZO77sv8LT-CI"

#define ss 5
#define rst 14 
#define dio0 2

// Insert RTDB URLefine the RTDB URL */

#define DATABASE_URL "https://rmkfootprintmonitoring-default-rtdb.asia-southeast1.firebasedatabase.app/"

#define USER_EMAIL "dqwrqreho@gmail.com"
#define USER_PASSWORD "ImITa@2021"


// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
int c=0;

char line[64];

bool signupOK = false;
String uid;
int pm10=0,pm25=0,CO2=0;

unsigned long sendDataPrevMillis = 0;

// NTP setup
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 3600;

void initWiFi()
{
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
        Serial.print(".");
        delay(300);
    }

    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
}

void setup()
{
    Serial.begin(9600);
    initWiFi();
    LoRa.setPins(ss, rst, dio0);
    while (!LoRa.begin(433E6)) {
    Serial.println(".");
    delay(500);
    }
    LoRa.setSyncWord(0xF3);
    Serial.println("LoRa Initializing OK!");
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    config.api_key = API_KEY;
    config.database_url = DATABASE_URL;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(4096);
    config.token_status_callback = tokenStatusCallback; // see addons/TokenHelper.h
    config.max_token_generation_retry = 5;
    Firebase.begin(&config, &auth);
    Serial.println("Getting User UID");
    while ((auth.token.uid) == "")
    {
        Serial.print('.');
        delay(1000);
    }
    signupOK = true;
    uid = auth.token.uid.c_str();
    Serial.print("User UID: ");
    Serial.print(uid); 
}



String convertToString(char *a, int size)
{
    int i;
    String s = "";
    for (i = 0; i < size; i++)
    {
        s = s + a[i];
    }
    return s;
}

String tellTime()
{
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    char a[22];
    strftime(a, 22, "%X", &timeinfo);
    String t = convertToString(a, 22);
    return t;
}

String tellDate()
{
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    char a[22];
    strftime(a, 22, "%d-%m-%Y", &timeinfo);
    String t = convertToString(a, 22);
    return t;
}

void loop()
{
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0))
    {
        sendDataPrevMillis = millis();
        String timeStamp = tellTime();
        String currDate = tellDate();
        
        int packetSize = LoRa.parsePacket();
        if(packetSize){
            strcpy(line,LoRa.readString().c_str());
            char * ptr = strtok(line,"=");
            ptr = strtok(NULL,";");
            pm25 = atoi(ptr);
            Serial.print("pm2.5= ");
            Serial.println(pm25);

            ptr =strtok(NULL,"=");
            ptr = strtok(NULL,";"); 
            pm10 = atoi(ptr); 
            Serial.print("pm10= ");
            Serial.println(pm10);

            ptr =strtok(NULL,"=");
            ptr = strtok(NULL,";"); 
            CO2 = atoi(ptr); 
            Serial.print("CO2= ");
            Serial.println(CO2);

        

        if(pm25!=0){
        FirebaseJson json1;
        if (!(Firebase.RTDB.getJSON(&fbdo, "sensor_1/pm25/" + currDate)))
        {
            json1.set(currDate + "/" + timeStamp, pm25);
            Serial.printf("Set json... %s\n", Firebase.RTDB.updateNode(&fbdo, F("/sensor_1/pm25"), &json1) ? "ok" : fbdo.errorReason().c_str());
        }
        else
        {
            json1.add(timeStamp, pm25);
            Serial.printf("Update node... %s\n", Firebase.RTDB.updateNode(&fbdo, "sensor_1/pm25/" + currDate, &json1) ? "ok" : fbdo.errorReason().c_str());
        }
        }
        if(pm10!=0){
        FirebaseJson json2;
        if (!(Firebase.RTDB.getJSON(&fbdo, "sensor_1/pm10/" + currDate)))
        {
            json2.set(currDate + "/" + timeStamp, pm10);
            Serial.printf("Set json... %s\n", Firebase.RTDB.updateNode(&fbdo, F("/sensor_1/pm10"), &json2) ? "ok" : fbdo.errorReason().c_str());
        }
        else
        {
            json2.add(timeStamp, pm10);
            Serial.printf("Update node... %s\n", Firebase.RTDB.updateNode(&fbdo, "sensor_1/pm10/" + currDate, &json2) ? "ok" : fbdo.errorReason().c_str());
        }
        }
        if(CO2!=0){
        FirebaseJson json3;
        if (!(Firebase.RTDB.getJSON(&fbdo, "sensor_1/CO2/" + currDate)))
        {
            json3.set(currDate + "/" + timeStamp, CO2);
            Serial.printf("Set json... %s\n", Firebase.RTDB.updateNode(&fbdo, F("/sensor_1/CO2"), &json3) ? "ok" : fbdo.errorReason().c_str());
        }
        else
        {
            json3.add(timeStamp, CO2);
            Serial.printf("Update node... %s\n", Firebase.RTDB.updateNode(&fbdo, "sensor_1/CO2/" + currDate, &json3) ? "ok" : fbdo.errorReason().c_str());
        }}}
    }
}
