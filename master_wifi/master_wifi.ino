#include <HardwareSerial.h> 

char arr[17];

void setup()
{
    Serial.begin(115200);
    Serial2.begin(115200);
}

void loop()
{
    if(Serial2.available())
    {
        String rx=""; 
        String sarr[2];
        while(Serial2.available())
        {
          delay(2); 
          char ch=Serial2.read(); 
          rx+=ch;
        }
        int strStart=0;
        int arrInd=0; 

        for(int i=0;i<rx.length();i++)
        {
            if(rx[i]==',')
            {
               sarr[0]=""; 
               sarr[0]=rx.substring(strStart,i);
               strStart=i+1;  
               break; 
            }
        }
        sarr[1]="";
        sarr[1]=rx.substring(strStart,i);
        Serial.println(sarr[0]);
        Serial.println(sarr[1]); 
        Serial.println("------------");
    }
}
