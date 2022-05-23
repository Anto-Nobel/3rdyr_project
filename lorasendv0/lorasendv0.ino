#include <SPI.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>
#include <LoRa.h>

#define ss 5
#define rst 14
#define dio0 2


void setup()
{
    Serial.begin(115200);
    Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
  }

  /* Default settings from datasheet. */
    // Serial.println("LoRa Se")
    LoRa.setPins(ss, rst, dio0);
    while (!LoRa.begin(433E6))
    {
        Serial.println(".");
        delay(500);
        LoRa.setSyncWord(0xF3);
        Serial.println("LoRa Initializing OK!");
    }
}

void loop() {
  Serial.println("Sending packet: ");
  //Serial.println(counter);

  //Send LoRa packet to receiver
  LoRa.beginPacket();
  LoRa.print(bmp.readTemperature());
  LoRa.print(",");
  LoRa.println(bmp.readPressure());
  LoRa.endPacket();
  delay(3000);
}