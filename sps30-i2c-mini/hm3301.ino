#include <stdlib.h>
#include "DS3231.h"
#include <SPI.h>
#include <SD.h>
#include "Seeed_HM330X.h"

#ifdef  ARDUINO_SAMD_VARIANT_COMPLIANCE
  #define SERIAL SerialUSB
#else
  #define SERIAL Serial
#endif


#define SD_CSPIN 4


File myFile;

HM330X sensor;
u8 buf[30];

err_t parse_result_value(u8 *data)
{
    if(NULL==data)
        return ERROR_PARAM;
 
    //Write the value of data[6]

    u8 sum=0;
    for(int i=0;i<28;i++)
    {
        sum+=data[i];
    }
    if(sum!=data[28])
    {
        SERIAL.println("wrong checkSum!!!!");
    }
    return NO_ERROR;
}


/*30s*/
void setup()
{
    SERIAL.begin(9600);
    delay(100);
    SERIAL.println("Serial start");

    if(!SD.begin(SD_CSPIN)) {
        SERIAL.println("Card not present...restarting.");
        delay(1000);
        return;
    }
    if(serialEn) Serial.println("Card initialized.");

    if(sensor.init())
    {
        SERIAL.println("HM330X init failed!!!");
        while(1);
    }

    myFile = SD.open("data.CSV",FILE_WRITE);
    // String filenum = printnewfileNum(root);
    // if(serialEn) Serial.print("file number: ");
    // if(serialEn) Serial.println(filenum);
    // int tempVal = filenum.toInt();
    // if(tempVal + 1 <= 999) tempVal += 1;
    // if(tempVal < 10) filenum = "00" + String(tempVal);
    // else if(tempVal < 100) filenum = "0" + String(tempVal);
    // else filenum = String(tempVal);
    // timestampFile = "data" + filenum + ".CSV";
    // File dataFile = SD.open(timestampFile, FILE_WRITE);

}



void loop()
{
    if(sensor.read_sensor_value(buf,29))
    {
        SERIAL.println("HM330X read result failed!!!");
    }
    parse_result_value(buf);
    delay(5000);
}
