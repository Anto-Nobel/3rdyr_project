#ifdef ESP32
  #include <WiFi.h>
#endif
#include <esp_now.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "time.h"

#define WIFI_SSID "Arunkumar M"
#define WIFI_PASSWORD "arun_2002"

#define API_KEY "AIzaSyAcSi97LtykSiVO3VPBLmXA_hj5HXOsLLA"
#define DATABASE_URL "https://espdemofirebase-default-rtdb.asia-southeast1.firebasedatabase.app/" 

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

String success;

uint8_t clgEspAddr[]={0xFC,0xF5,0xC4,0x0F,0x84,0x2C};

float recvd_temperature;
float recvd_humidity;

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 3600;


typedef struct trigger_message{
    float trig;
}trigger_message;

typedef struct recvd_message{
    float temp;
    float hum;
}recvd_message;

trigger_message trigger;
recvd_message sensor1_data;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  if (status ==0){
    success = "Delivery Success :)";
  }
  else{
    success = "Delivery Fail :(";
  }
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&sensor1_data, incomingData, sizeof(sensor1_data));
  Serial.print("Bytes received: ");
  Serial.println(len);
  recvd_temperature = sensor1_data.temp;
  recvd_humidity = sensor1_data.hum;
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

void setup()
{
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
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

    if(esp_now_init()!=ESP_OK)
    {
        Serial.println("Error: cannot initialize ESP-NOW");
        return;
    }

    esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, clgEspAddr, 6);
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;
  
  // Add peer        
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
}

void loop()
{
    trigger.trig=true;
    esp_err_t result = esp_now_send(clgEspAddr, (uint8_t *) &trigger, sizeof(trigger));
    if(result == ESP_OK)
    {
        Serial.println("Trigger sent successfully");
    }
    else
    {
        Serial.println("Trigger not sent");
    }

    delay(30000); 

    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 30000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    String timeStamp=tellTime();
    String currDate=tellDate();
    FirebaseJson json1;
    if(!(Firebase.RTDB.getJSON(&fbdo,"sensor_1/temperature/"+currDate)))
    {
      json1.set(currDate+"/"+timeStamp,recvd_temperature);
      Serial.printf("Set json... %s\n", Firebase.RTDB.set(&fbdo, F("/sensor_1/temperature"), &json1) ? "ok" : fbdo.errorReason().c_str());
    }
    else
    {
      json1.add(timeStamp,recvd_temperature);
      Serial.printf("Update node... %s\n", Firebase.RTDB.updateNode(&fbdo, "sensor_1/temperature/"+currDate, &json1) ? "ok" : fbdo.errorReason().c_str());
    }

    FirebaseJson json2;
    if(!(Firebase.RTDB.getJSON(&fbdo,"sensor_1/humidity/"+currDate)))
    {
      json2.set(currDate+"/"+timeStamp,recvd_humidity);
      Serial.printf("Set json... %s\n", Firebase.RTDB.set(&fbdo, F("/sensor_1/humidity"), &json2) ? "ok" : fbdo.errorReason().c_str());
    }
    else
    {
      json2.add(timeStamp,recvd_humidity);
      Serial.printf("Update node... %s\n", Firebase.RTDB.updateNode(&fbdo, "sensor_1/humidity/"+currDate, &json2) ? "ok" : fbdo.errorReason().c_str());
    }
  }
}