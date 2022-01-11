#ifdef ESP32
  #include <WiFi.h>
#endif
#include <esp_now.h>
#include <Arduino.h>
#include <SPI.h> 
#include <Wire.h> 
#include <ArduinoJson.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE); 

uint8_t arunEspAddr[]={0x7C,0x9E,0xBD,0xE5,0x30,0x2C};

bool incoming;

String success;

typedef struct reply_message{
    float temp;
    float hum;
}reply_message;

typedef struct received_messsage{
    float trig;
}received_messsage;

reply_message dht_data;
received_messsage msg_got;

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
  memcpy(&msg_got, incomingData, sizeof(msg_got));
  Serial.print("Bytes received: ");
  Serial.println(len);
  incoming = msg_got.trig;
}

void setup()
{
    Serial.begin(115200);
    dht.begin();
    WiFi.mode(WIFI_STA);

    if(esp_now_init()!=ESP_OK)
    {
        Serial.println("Error: cannot initialize ESP-NOW");
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
  // Register for a callback function that will be called when data is received
  esp_now_register_recv_cb(OnDataRecv);
}

void loop()
{
    if(incoming)
    {   
        dht_data.temp=dht.readTemperature();
        dht_data.hum=dht.readHumidity();
        Serial.println("trigger from Central ESP recieved");
        esp_err_t result=esp_now_send(arunEspAddr,(uint8_t *) &dht_data,sizeof(dht_data));
        if(result == ESP_OK)
        {
            Serial.println("Data sent successfully");
        }
        else
        {
            Serial.println("Failed to send data");
        }
    }
    delay(15000);
}