#include <esp_now.h>
#include <WiFi.h>
#include <SPI.h> 
#include <Wire.h> 
#include <DHT.h>

uint8_t broadcastAddress[] = {0xFC,0xF5,0xC4,0x0F,0x84,0x2C};

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

typedef struct struct_message {
  float temp;
  float humi;
} struct_message;

struct_message myData;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}
 

void setup() {
  Serial.begin(115200);
 
  WiFi.mode(WIFI_STA);

  dht.begin();

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

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
}
 
void loop() {
  myData.temp = dht.readTemperature();
  myData.humi = dht.readHumidity();

  // Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &myData, sizeof(myData));
   
  if (result == ESP_OK) {
    Serial.println("Sent with success");
  }
  else {
    Serial.println("Error sending the data");
  }
  delay(1000);
}
