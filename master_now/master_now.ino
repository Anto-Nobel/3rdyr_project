#include <esp_now.h>
#include <WiFi.h> 
#include <HardwareSerial.h> 

//HardwareSerial Serial2(2); 

typedef struct struct_message{
    float temp;
    float humi;
}struct_message;


struct_message data; 

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len){
    memcpy(&data, incomingData, sizeof(data));
    Serial.println(data.humi);
    Serial.println(data.temp); 
    if(isnan(data.temp)||isnan(data.humi))
    {
        return;
    }
    else{
        char buff[7],m[15];
        String te,hu,me,t,h;
        t=dtostrf(data.temp,3,2,buff);
        h=dtostrf(data.humi,3,2,buff); 
        te=t;
        hu=h;
        me=te+","+hu; 
        Serial2.write(me.c_str());
    }
}

void setup(){
    Serial.begin(115200);
    Serial2.begin(115200);
    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
    }
    esp_now_register_recv_cb(OnDataRecv);
}

void loop(){ 

}
