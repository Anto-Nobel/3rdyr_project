#include <Arduino.h>
#ifdef ESP32
  #include <WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

#include <SPI.h> 
#include <Wire.h> 
#include <ArduinoJson.h>
#include <DHT.h>
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
#include "time.h"




// Insert your network credentials

#define WIFI_SSID "Arunkumar M"

#define WIFI_PASSWORD "arun_2002"

#define DHTPIN 4

#define DHTTYPE DHT11

// Insert Firebase project API Key

#define API_KEY "AIzaSyAcSi97LtykSiVO3VPBLmXA_hj5HXOsLLA"



// Insert RTDB URLefine the RTDB URL */

#define DATABASE_URL "https://espdemofirebase-default-rtdb.asia-southeast1.firebasedatabase.app/" 



//Define Firebase Data object

FirebaseData fbdo;



FirebaseAuth auth;

FirebaseConfig config;

DHT dht(DHTPIN, DHTTYPE);

unsigned long sendDataPrevMillis = 0;


bool signupOK = false;



//NTP setup

const char* ntpServer = "pool.ntp.org";

const long gmtOffset_sec = 19800;

const int daylightOffset_sec = 3600;
unsigned long count1 = 0;
unsigned long count2 = 0;


void setup(){

  Serial.begin(115200);

  dht.begin();

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



  /* Assign the api key (required) */

  config.api_key = API_KEY;



  /* Assign the RTDB URL (required) */

  config.database_url = DATABASE_URL;



  /* Sign up */

  if (Firebase.signUp(&config, &auth, "", "")){

    Serial.println("ok");

    signupOK = true;

  }

  else{

    Serial.printf("%s\n", config.signer.signupError.message.c_str());

  }



  /* Assign the callback function for the long running token generation task */

  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  

  Firebase.begin(&config, &auth);

  Firebase.reconnectWiFi(true);

  

  //Time config.

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  

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

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 30000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    String timeStamp=tellTime();
    String currDate=tellDate();
    FirebaseJson json1;
    if(!(Firebase.RTDB.getJSON(&fbdo,"sensor_1/temperature/"+currDate)))
    {
      json1.set(currDate+"/"+timeStamp,dht.readTemperature());
      Serial.printf("Set json... %s\n", Firebase.RTDB.set(&fbdo, F("/sensor_1/temperature"), &json1) ? "ok" : fbdo.errorReason().c_str());
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
      Serial.printf("Set json... %s\n", Firebase.RTDB.set(&fbdo, F("/sensor_1/humidity"), &json2) ? "ok" : fbdo.errorReason().c_str());
    }
    else
    {
      json2.add(timeStamp,dht.readHumidity());
      Serial.printf("Update node... %s\n", Firebase.RTDB.updateNode(&fbdo, "sensor_1/humidity/"+currDate, &json2) ? "ok" : fbdo.errorReason().c_str());
    }
  }

}
