#include <Arduino.h>
#ifdef ESP32
#include <WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <HardwareSerial.h>
#include <SPI.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "time.h"

#define WIFI_SSID "NaN"
#define WIFI_PASSWORD "antonobel"
#define API_KEY "AIzaSyA-u_CqSZoLKyeHxeN4qNzZO77sv8LT-CI"

// Insert RTDB URLefine the RTDB URL */

#define DATABASE_URL "https://rmkfootprintmonitoring-default-rtdb.asia-southeast1.firebasedatabase.app/"

#define USER_EMAIL "dqwrqreho@gmail.com"
#define USER_PASSWORD "ImITa@2021"

// Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK = false;
String uid;
byte cmd[] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
byte buf[9];
uint16_t pm25, pm10;

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
    Serial2.begin(9600);
    initWiFi();
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
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 30000 || sendDataPrevMillis == 0))
    {
        sendDataPrevMillis = millis();
        String timeStamp = tellTime();
        String currDate = tellDate();
        Serial2.write(cmd[0]);
        Serial2.write(cmd[1]);
        Serial2.write(cmd[2]);
        Serial2.write(cmd[3]);
        Serial2.write(cmd[4]);
        Serial2.write(cmd[5]);
        Serial2.write(cmd[6]);
        Serial2.write(cmd[7]);
        Serial2.write(cmd[8]);
        Serial2.readBytes(buf, sizeof(buf));
        pm25 = (buf[2] << 8) | buf[3];
        Serial.print(" pm25=");
        Serial.print(pm25);
        pm10 = (buf[4] << 8) | buf[5];
        Serial.print("\n");

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
}
