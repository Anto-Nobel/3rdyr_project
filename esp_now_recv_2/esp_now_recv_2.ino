#include <esp_now.h>
#ifdef ESP32
    #include <WiFi.h>
#endif 
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include "time.h" 

static const char* ssid     = "Arunkumar M";
static const char* password = "arun_2002";
// Structure to keep the temperature and humidity data
// Is also required in the client to be able to save the data directly
typedef struct temp_humidity {
  float temperature;
  float humidity;
};
// Create a struct_message called myData
temp_humidity dhtData;

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
bool signupOK = false;

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;
const int daylightOffset_sec = 3600;

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



// callback function executed when data is received
void OnRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&dhtData, incomingData, sizeof(dhtData));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Temperature: ");
  Serial.println(dhtData.temperature);
  Serial.print("Humidity: ");
  Serial.println(dhtData.humidity); 
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 30000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    String timeStamp=tellTime();
    String currDate=tellDate();
    FirebaseJson json1;
    if(!(Firebase.RTDB.getJSON(&fbdo,"sensor_1/temperature/"+currDate)))
    {
      json1.set(currDate+"/"+timeStamp,dhtData.temperature);
      Serial.printf("Set json... %s\n", Firebase.RTDB.set(&fbdo, F("/sensor_1/temperature"), &json1) ? "ok" : fbdo.errorReason().c_str());
    }
    else
    {
      json1.add(timeStamp,dhtData.temperature);
      Serial.printf("Update node... %s\n", Firebase.RTDB.updateNode(&fbdo, "sensor_1/temperature/"+currDate, &json1) ? "ok" : fbdo.errorReason().c_str());
    }

    FirebaseJson json2;
    if(!(Firebase.RTDB.getJSON(&fbdo,"sensor_1/humidity/"+currDate)))
    {
      json2.set(currDate+"/"+timeStamp,dhtData.humidity);
      Serial.printf("Set json... %s\n", Firebase.RTDB.set(&fbdo, F("/sensor_1/humidity"), &json2) ? "ok" : fbdo.errorReason().c_str());
    }
    else
    {
      json2.add(timeStamp,dhtData.humidity);
      Serial.printf("Update node... %s\n", Firebase.RTDB.updateNode(&fbdo, "sensor_1/humidity/"+currDate, &json2) ? "ok" : fbdo.errorReason().c_str());
    }
  }
}
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_AP_STA);
  WiFi.begin(ssid, password);
  //check wi-fi is connected to wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  
  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("There was an error initializing ESP-NOW");
    return;
  }
  
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

  // Once the ESP-Now protocol is initialized, we will register the callback function
  // to be able to react when a package arrives in near to real time without pooling every loop.
  esp_now_register_recv_cb(OnRecv);
}
void loop() {
}
