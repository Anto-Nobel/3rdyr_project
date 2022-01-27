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
#include <MQUnifiedsensor.h>

#define placa "ESP-32"
#define Voltage_Resolution 3.3
#define pin 4 //Analog input 0 of your arduino
#define type "MQ-135" //MQ135
#define ADC_Bit_Resolution 12 // For arduino UNO/MEGA/NANO
#define RatioMQ135CleanAir 3.6//RS / R0 = 3.6 ppm  
//#define calibration_button 13 //Pin to calibrate your sensor


// Insert your network credentials

#define WIFI_SSID "Arunkumar M"

#define WIFI_PASSWORD "arun_2002"

#define DHTPIN 4

#define DHTTYPE DHT11

// Insert Firebase project API Key

#define API_KEY "AIzaSyAcSi97LtykSiVO3VPBLmXA_hj5HXOsLLA"



// Insert RTDB URLefine the RTDB URL */

#define DATABASE_URL "https://espdemofirebase-default-rtdb.asia-southeast1.firebasedatabase.app/" 

MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);

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

  MQ135.setRegressionMethod(1); 
  MQ135.init();
  MQ135.setRL(1); 
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ135.update(); // Update data, the arduino will be read the voltage on the analog pin
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0/10);
  Serial.println("  done!.");
  
  if(isinf(calcR0)) {Serial.println("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply"); while(1);}

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


//comment
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

    FirebaseJson json3;
    MQ135.update();
    MQ135.setA(110.47); MQ135.setB(-2.862); 
    float CO2 = MQ135.readSensor()+415.08;
    if(!(Firebase.RTDB.getJSON(&fbdo,"sensor_1/co2/"+currDate)))
    {
      json3.set(currDate+"/"+timeStamp,CO2);
      Serial.printf("Set json... %s\n", Firebase.RTDB.set(&fbdo, F("/sensor_1/co2"), &json3) ? "ok" : fbdo.errorReason().c_str());
    }
    else
    {
      json3.add(timeStamp,CO2);
      Serial.printf("Update node... %s\n", Firebase.RTDB.updateNode(&fbdo, "sensor_1/co2/"+currDate, &json3) ? "ok" : fbdo.errorReason().c_str());
    }
  }

}
