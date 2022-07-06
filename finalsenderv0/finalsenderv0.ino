#include <Arduino.h>
#include <HardwareSerial.h>
#include "MHZ19.h"
#include <SoftwareSerial.h>
#include <SPI.h>
#include <Wire.h>
#include <LoRa.h>

#define RX_PIN 12
#define TX_PIN 13
#define BAUDRATE 9600

#define ss 5
#define rst 14
#define dio0 2

#define LED 2

int c = 0;

MHZ19 myMHZ19;
SoftwareSerial mySerial(RX_PIN, TX_PIN);

byte cmd[] = {0xFF, 0x01, 0x86, 0x00, 0x00, 0x00, 0x00, 0x00, 0x79};
byte buf[9];
uint16_t pm25, pm10;
int CO2;

unsigned long sendDataPrevMillis = 0;

void setup()
{
    Serial.begin(9600);
    Serial2.begin(9600);
    pinMode(LED,OUTPUT);
    mySerial.begin(BAUDRATE);
    myMHZ19.begin(mySerial);
    myMHZ19.autoCalibration();

    LoRa.setPins(ss, rst, dio0);
    while (!LoRa.begin(433E6))
    {
        Serial.println(".");
        digitalWrite(LED,HIGH);
        delay(500);
        digitalWrite(LED,LOW);
    }
    LoRa.setSyncWord(0xF3);
    Serial.println("LoRa Initializing OK!");
}

void loop()
{

    Serial2.write(cmd[0]);
    Serial2.write(cmd[1]);
    Serial2.write(cmd[2]);
    Serial2.write(cmd[3]);
    Serial2.write(cmd[4]);
    Serial2.write(cmd[5]);
    Serial2.write(cmd[6]);
    Serial2.write(cmd[7]);
    Serial2.write(cmd[8]);
    for (int j = 0; j < 9; j++)
    {
        buf[j] = 0;
    }
    Serial2.readBytes(buf, sizeof(buf));
    int startpos = 0;
    c = 0;
    for (int j = 0; j < 9; j++)
    {
        c += buf[j];
    }
    while (buf[startpos] != 255 && c >= 255)
    {
        startpos += 1;
    }
    pm25 = (buf[(startpos + 2) % 9] << 4) | buf[(startpos + 3) % 9];
    Serial.print(" pm25=");
    Serial.print(pm25);
    pm10 = (buf[(startpos + 4) % 9] << 4) | buf[(startpos + 5) % 9];
    Serial.print("\n");
    Serial.print("pm10 : ");
    Serial.println(pm10);
    Serial.print("CO2 : ");
    CO2 = myMHZ19.getCO2();
    Serial.println(CO2);
    LoRa.beginPacket();
    LoRa.print("0=");
    LoRa.print(pm25);
    LoRa.print("; ");
    LoRa.print("1=");
    LoRa.print(pm10);
    LoRa.print("; ");
    LoRa.print("2=");
    LoRa.print(CO2);
    LoRa.print("\n");
    LoRa.endPacket();
    delay(3000);
}
