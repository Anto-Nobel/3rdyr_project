#include <Arduino.h>
#include "MHZ19.h"                                        
#include <SoftwareSerial.h>                                

#define RX_PIN 16                                          
#define TX_PIN 17                                          
#define BAUDRATE 9600                                      

MHZ19 myMHZ19;                                             
SoftwareSerial mySerial(RX_PIN, TX_PIN);                   

unsigned long getDataTimer = 0;

void setup()
{
    Serial.begin(9600);                                     
    mySerial.begin(BAUDRATE);                               
    myMHZ19.begin(mySerial);                                
    myMHZ19.autoCalibration();                              

void loop()
{
    if (millis() - getDataTimer >= 2000)
    {
        int CO2; 
        CO2 = myMHZ19.getCO2();                             
        Serial.print("CO2 (ppm): ");                      
        Serial.println(CO2);                                
        int8_t Temp;
        Temp = myMHZ19.getTemperature();                     
        Serial.print("Temperature (C): ");                  
        Serial.println(Temp);                               
        getDataTimer = millis();
    }
}1