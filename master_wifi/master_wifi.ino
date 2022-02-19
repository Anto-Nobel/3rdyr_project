#include <HardwareSerial.h>
#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

#include <ArduinoJson.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "time.h" 

#define WIFI_SSID "nobel"
#define WIFI_PASSWORD "sarobert"

#define API_KEY "AIzaSyCSSeqZ3GJCPUK14CsNfE9P3I4BJUcQNOQ"
#define DATABASE_URL "https://espsensorjs-default-rtdb.asia-southeast1.firebasedatabase.app/"  

#define USER_EMAIL "dqwrqreho@gmail.com"
#define USER_PASSWORD "ImITa@2021"

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

bool signupOK=false;
String uid;
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


void setup()
{
    Serial.begin(115200);
    Serial2.begin(115200);
    
    initWiFi();

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
    float t,h;
    if(Serial2.available())
    {
        String rx=""; 
        String sarr[2];
        while(Serial2.available())
        {
          delay(2); 
          char ch=Serial2.read(); 
          rx+=ch;
        }
        int strStart=0;
        int arrInd=0; 
        int i;
        for(i=0;i<rx.length();i++)
        {
            if(rx[i]==',')
            {
               sarr[0]=""; 
               sarr[0]=rx.substring(strStart,i);
               strStart=i+1;  
            }
            else if(rx[i]=='\0')
            {
                break;
            }
        }
        sarr[1]="";
        sarr[1]=rx.substring(strStart,i);
        Serial.println(sarr[0]);
        Serial.println(sarr[1]); 
        Serial.println("------------");
        t=sarr[0].toFloat();
        h=sarr[1].toFloat();
        if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 30000 || sendDataPrevMillis == 0)){
            sendDataPrevMillis = millis();
            String timeStamp=tellTime();
            String currDate=tellDate();
            FirebaseJson json1;
            if(!(Firebase.RTDB.getJSON(&fbdo,"sensor_1/temperature/"+currDate)))
            {
                json1.set(currDate+"/"+timeStamp,t);
                Serial.printf("Set json... %s\n", Firebase.RTDB.set(&fbdo, F("/sensor_1/temperature"), &json1) ? "ok" : fbdo.errorReason().c_str());
            }
            else
            {
                json1.add(timeStamp,t);
                Serial.printf("Update node... %s\n", Firebase.RTDB.updateNode(&fbdo, "sensor_1/temperature/"+currDate, &json1) ? "ok" : fbdo.errorReason().c_str());
            }

            FirebaseJson json2;
            if(!(Firebase.RTDB.getJSON(&fbdo,"sensor_1/humidity/"+currDate)))
            {
                json2.set(currDate+"/"+timeStamp,h);
                Serial.printf("Set json... %s\n", Firebase.RTDB.set(&fbdo, F("/sensor_1/humidity"), &json2) ? "ok" : fbdo.errorReason().c_str());
            }
            else
            {
                json2.add(timeStamp,h);
                Serial.printf("Update node... %s\n", Firebase.RTDB.updateNode(&fbdo, "sensor_1/humidity/"+currDate, &json2) ? "ok" : fbdo.errorReason().c_str());
            }
        }
    }
}
