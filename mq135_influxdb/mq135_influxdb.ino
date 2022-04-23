#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#define DEVICE "ESP32_1"

#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>
#include <MQUnifiedsensor.h>

#define WIFI_SSID "NaN"
#define WIFI_PASSWORD "antonobel"

//InfluxDB
#define INFLUXDB_URL "https://europe-west1-1.gcp.cloud2.influxdata.com"
#define INFLUXDB_TOKEN "KlmcztU1dEWZ04JzoiK3eg2eBtZxH-LbbYauBEH0UGpdmfjqxGvwIpaAIXv8EqTwIiXD-Hs0O3Konu3NjyrQfA=="
#define INFLUXDB_ORG "nobel2002@gmail.com"
#define INFLUXDB_BUCKET "nobel2002's Bucket"
#define TZ_INFO "IST"

//MQ135
#define placa "ESP-32"
#define Voltage_Resolution 3.3
#define pin 4
#define type "MQ-135"
#define ADC_Bit_Resolution 12
#define RatioMQ135CleanAir 3.6


MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, pin, type);
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

Point sensor1("pollutionReading");
Point sensor2("pollutionReading");
Point sensor3("pollutionReading");

void setup() {
  Serial.begin(115200);

  MQ135.setRegressionMethod(1);
  MQ135.init();
  MQ135.setRL(1); 
  Serial.print("Calibrating please wait.");
  float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
  {
    MQ135.update();
    calcR0 += MQ135.calibrate(RatioMQ135CleanAir);
    Serial.print(".");
  }
  MQ135.setR0(calcR0/10);
  Serial.println("  done!.");
  
  if(isinf(calcR0)) {Serial.println("Warning: Conection issue founded, R0 is infite (Open circuit detected) please check your wiring and supply"); while(1);}
  if(calcR0 == 0){Serial.println("Warning: Conection issue founded, R0 is zero (Analog pin with short circuit to ground) please check your wiring and supply"); while(1);}


  // Setup wifi
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to wifi");
  while (wifiMulti.run() != WL_CONNECTED) {
    Serial.print(".");
    delay(100);
  }
  Serial.println();

  sensor1.addTag("sensor", DEVICE);
  sensor1.addTag("pollutant", "CO2");

  sensor2.addTag("sensor", DEVICE);
  sensor2.addTag("pollutant", "CO");

  sensor3.addTag("sensor", DEVICE);
  sensor3.addTag("pollutant", "NH4");

  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }
}

void loop() {
  sensor1.clearFields();
  sensor2.clearFields();
  sensor3.clearFields();
  MQ135.update();
  MQ135.setA(605.18); MQ135.setB(-3.937);
  float CO = MQ135.readSensor();
  MQ135.setA(77.255); MQ135.setB(-3.18);
  float Alcohol = MQ135.readSensor();
  MQ135.setA(110.47); MQ135.setB(-2.862);
  float CO2 = MQ135.readSensor();
  MQ135.setA(44.947); MQ135.setB(-3.445);
  float Tolueno = MQ135.readSensor();
  MQ135.setA(102.2 ); MQ135.setB(-2.473);
  float NH4 = MQ135.readSensor();
  MQ135.setA(34.668); MQ135.setB(-3.369); 
  float Acetona = MQ135.readSensor();

  sensor1.addField("reading",CO2);
  Serial.print("Writing: ");
  Serial.println(sensor1.toLineProtocol());
  if (!client.writePoint(sensor1)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  sensor2.addField("reading",CO);
  Serial.print("Writing: ");
  Serial.println(sensor2.toLineProtocol());
  if (!client.writePoint(sensor2)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  sensor3.addField("reading",NH4);
  Serial.print("Writing: ");
  Serial.println(sensor3.toLineProtocol());
  if (!client.writePoint(sensor3)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  Serial.println("Wait 10s");
  delay(5000);
}
