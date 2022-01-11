#include <esp_now.h>
#ifdef ESP32
    #include <WiFi.h>
#endif 
#include <Arduino.h>
#include <SPI.h> 
#include <Wire.h> 
#include <ArduinoJson.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE); 

uint8_t broadcastAddress[] = {0x7C, 0x9E, 0xBD, 0xE5, 0x30, 0x2C};

float temperature;
float humidity; 

bool incomingTrigger;

String success;

typedef struct struct_message {
    float temp;
    float hum;
} struct_message; 

typedef struct trigger_message{
    bool trig;
}trigger_message; 

struct_message DHT11Readings;
trigger_message incomingMessage;

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
  memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));
  Serial.print("Bytes received: ");
  Serial.println(len);
  incomingTrigger=incomingMessage.trig;
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  
  // Init DHT11 sensor
  dht.begin();
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  
  // Register peer
  esp_now_peer_info_t peerInfo;
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
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

void loop() {
  getReadings();
 
  // Set values to send
  DHT11Readings.temp = temperature;
  DHT11Readings.hum = humidity;
  if(incomingTrigger){
  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &DHT11Readings, sizeof(DHT11Readings));
  }
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(30000);
  incomingTrigger=false;
  //delay(10000);
} 

void getReadings(){
  temperature = dht.readTemperature();
  Serial.print("temperature : ");
  Serial.println(temperature);
  humidity = dht.readHumidity();
  Serial.print("humidity : ");
  Serial.println(humidity);
}