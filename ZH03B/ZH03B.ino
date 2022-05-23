#include <HardwareSerial.h>

byte cmd[] ={0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; 
byte buf[9];

uint16_t pm25,pm10;

void setup() {
  // put your setup code here, to run once:
  Serial2.begin(9600);
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial2.write(cmd[0]);
  Serial2.write(cmd[1]);
  Serial2.write(cmd[2]);
  Serial2.write(cmd[3]);
  Serial2.write(cmd[4]);
  Serial2.write(cmd[5]);
  Serial2.write(cmd[6]);
  Serial2.write(cmd[7]); 
  Serial2.write(cmd[8]); 
  //Serial1.write(read[9]); 

  Serial2.readBytes(buf,sizeof(buf)); 
  for(int i=0;i<9;i++){
    Serial.print(buf[i]);
    Serial.print(" ");
  } 
  pm25 = (buf[2]<<8) | buf[3]; 
  Serial.print(" pm25=");
  Serial.print(pm25); 
  pm10 = (buf[4]<<8) | buf[5];
  Serial.print("\n");
  delay(2000);
}
