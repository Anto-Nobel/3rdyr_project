#include <SPI.h> 
#include <Wire.h> 
#include <DHT.h>
#include <MQ135.h>

#define MQPIN  34
#define DHTPIN 4
#define DHTTYPE DHT11 

DHT dht(DHTPIN, DHTTYPE);
MQ135 mq135_sensor(MQPIN);

void setup()
{
    Serial.begin(115200);
    dht.begin();
}

void loop()
{   
    float humidity = dht.readHumidity();
    float temperature = dht.readTemperature();

    Serial.println(humidity);
    Serial.printf("\n"); 
    Serial.println(temperature); 
    Serial.printf("\n"); 

    float rzero = mq135_sensor.getRZero();
    float correctedRZero = mq135_sensor.getCorrectedRZero(temperature, humidity);
    float resistance = mq135_sensor.getResistance();
    float ppm = mq135_sensor.getPPM();
    float correctedPPM = mq135_sensor.getCorrectedPPM(temperature, humidity);

    Serial.print("MQ135 RZero: ");
    Serial.print(rzero);
    Serial.print("\t Corrected RZero: ");
    Serial.print(correctedRZero);
    Serial.print("\t Resistance: ");
    Serial.print(resistance);
    Serial.print("\t PPM: ");
    Serial.print(ppm);
    Serial.print("ppm");
    Serial.print("\t Corrected PPM: ");
    Serial.print(correctedPPM);
    Serial.println("ppm");
    delay(5000);
}
