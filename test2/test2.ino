#include <Arduino.h>
#include <WiFi.h>
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

#define API_KEY "AIzaSyBWYbQAyNRSdDtik6Ws_B3KOle4SHDOu9w"



// Insert RTDB URLefine the RTDB URL */

#define DATABASE_URL "https://espdemojs-default-rtdb.asia-southeast1.firebasedatabase.app/" 



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
unsigned long count = 0;


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

  strftime(a,22, "%d/%m/%Y", &timeinfo);

  String t=convertToString(a,22);

  return t;

} 



void loop(){

  

  //sending data

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 300000 || sendDataPrevMillis == 0)){

    sendDataPrevMillis = millis();
    String timeStamp=tellTime();
    String currDate=tellDate();
    FirebaseJson json;
    if(!(Firebase.RTDB.getInt(&fbdo,"test1/temperature/"+currDate)))
    {
      count=0;
    }
    if(count==0)
    {
      json.set("temperature/"+currDate+"/"+timeStamp,dht.readTemperature());
      Firebase.RTDB.set(&fbdo,F("test1"),&json);
      
    }
    else
    {
      json.add(timeStamp,dht.readTemperature());
      Firebase.RTDB.updateNode(&fbdo,F("test1/temperature/"+currDate),&json);
    }  
    count++;
  }

}
