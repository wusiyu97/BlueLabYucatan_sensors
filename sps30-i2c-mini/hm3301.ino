#include "Wire.h"
#include "Seeed_HM330X.h"
#include "DS3231.h"
#include <SPI.h>
#include <SD.h>

#ifdef  ARDUINO_SAMD_VARIANT_COMPLIANCE
  #define SERIAL SerialUSB
#else
  #define SERIAL Serial
#endif

#define SD_CSPIN 4
#define SAMPLE_PERIOD 10000
#define IDLE_SW 8
#define SERIAL_ENABLE 9
#define NEW_FILE_SW 10
#define DEFAULT_IIC_ADDR 0x40


DS3231 rtc;
//SdFat SD;
bool Century=false;
//bool sdMissing = false;
bool h12;
bool PM;
String timestampFile = "NULL";

HM330X sensor;
u8 buf[30];

void(* resetFunc) (void) = 0;


const char *str[]={"sensor num: ","PM1.0 concentration(CF=1,Standard particulate matter,unit:ug/m3): ",
                    "PM2.5 concentration(CF=1,Standard particulate matter,unit:ug/m3): ",
                    "PM10 concentration(CF=1,Standard particulate matter,unit:ug/m3): ",
                    "PM1.0 concentration(Atmospheric environment,unit:ug/m3): ",
                    "PM2.5 concentration(Atmospheric environment,unit:ug/m3): ",
                    "PM10 concentration(Atmospheric environment,unit:ug/m3): ",
                    };

err_t print_result(const char* str,u16 value)
{
    if(NULL==str)
        return ERROR_PARAM;
    SERIAL.print(str);
    SERIAL.println(value);
    return NO_ERROR;
}

/*parse buf with 29 u8-data*/
err_t parse_result(u8 *data)
{
    u16 value=0;
    err_t NO_ERROR;
    if(NULL==data)
        return ERROR_PARAM;
    for(int i=1;i<8;i++)
    {
         value = (u16)data[i*2]<<8|data[i*2+1];
         print_result(str[i-1],value);

    }
}

//Function to change/replace
err_t parse_result_value(u8 *data)
{
    if(NULL==data)
        return ERROR_PARAM;
    for(int i=0;i<28;i++)
    {
        SERIAL.print(data[i],HEX);
        SERIAL.print("  ");
        if((0==(i)%5)||(0==i))
        {
            SERIAL.println(" ");
        }
    }
    u8 sum=0;
    for(int i=0;i<28;i++)
    {
        sum+=data[i];
    }
    if(sum!=data[28])
    {
        SERIAL.println("wrong checkSum!!!!");
    }
    SERIAL.println(" ");
    SERIAL.println(" ");
    return NO_ERROR;
}


/*30s*/
void setup()
{
    //Declare dip switch pinmodes
    pinMode(SD_CSPIN, OUTPUT);
    pinMode(IDLE_SW, INPUT);
    pinMode(SERIAL_ENABLE, INPUT);
    pinMode(NEW_FILE_SW, INPUT);

    bool serialEn = bool(digitalRead(SERIAL_ENABLE));

    if(!digitalRead(IDLE_SW)) {
        if(serialEn) SERIAL.println(F("IDLE MODE ENABLED (checks every 5 sec)"));
        delay(5000); //5 second delay to check for idle mode
        resetFunc();
    }
    
    Wire.begin(); // Initiate the Wire library
    
    if(serialEn){
        SERIAL.begin(115200);
        while(!Serial) {
            delay(100);
        }
        SERIAL.print(F("UART CONNECTION: SUCCESSFUL\n"));

        String mdy =  String(rtc.getMonth(Century)) + "/" + String(rtc.getDate()) + "/" + String(rtc.getYear());
        String timeStamp = String(rtc.getHour(h12, PM)) + ":" + String(rtc.getMinute()) + ":" + String(rtc.getSecond());
        SERIAL.println(mdy);
        SERIAL.println(timeStamp);
        SERIAL.println(F("Initializing SD card..."));
    }


    // see if the card is present and can be initialized:
    if(!SD.begin(SD_CSPIN)) {
        if(serialEn) SERIAL.println("Card failed, or not present...restarting");
        // don't do anything more:
        delay(2000);
        resetFunc();
    }
    if(serialEn) SERIAL.println("card initialized.");

    //===========================================Haven't changed below code yet


}



void loop()
{
    if(sensor.read_sensor_value(buf,29))
    {
        SERIAL.println("HM330X read result failed!!!");
    }
    parse_result_value(buf);
    parse_result(buf);
    SERIAL.println(" ");
    SERIAL.println(" ");
    SERIAL.println(" ");
    delay(5000);
}