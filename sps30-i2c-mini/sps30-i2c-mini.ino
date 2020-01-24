/* SPS30 simple test program for Arduino (with modified Wire.h lib)
 *
 * 
 * Code is a variation of Michael Pruefer's code for use on the Senseron SPS30 particulate matter sensor
 * Copyright (c) 2019, Michael Pruefer
 * Revision by Nathan Peterson 4/24/2019
 * 
 * All rights reserved.
 *
 *
 * * For details see vendors documentation: 
 *   https://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/0_Datasheets/Particulate_Matter/Sensirion_PM_Sensors_SPS30_Datasheet.pdf
 * 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
//#define BUFFER_LENGTH 64 in Wire.h before

#include "Wire.h"
#include "DS3231.h"
#include <SPI.h>
#include <SD.h>

#define SD_CSPIN 4
#define ADDRESS 0x69 // device address of SPS30 (fixed).
#define SAMPLE_PERIOD 10000 //sample period in ms
#define IDLE_SW 8
#define SERIAL_ENABLE 9
#define NEW_FILE_SW 10

DS3231 rtc;
//SdFat SD;
bool Century=false;
//bool sdMissing = false;
bool h12;
bool PM;
String timestampFile = "NULL";
//File root;

void(* resetFunc) (void) = 0;
 
void setup() {

  //Declare dip switch pinmodes
  pinMode(SD_CSPIN, OUTPUT);
  pinMode(IDLE_SW, INPUT);
  pinMode(SERIAL_ENABLE, INPUT);
  pinMode(NEW_FILE_SW, INPUT);

  bool serialEn = bool(digitalRead(SERIAL_ENABLE));
  
  if(!digitalRead(IDLE_SW)) {
    if(serialEn) Serial.println(F("IDLE MODE ENABLED (checks every 5 sec)"));
    delay(5000); //5 second delay to check for idle mode
    resetFunc();
  }
  
  Wire.begin(); // Initiate the Wire library

  if(serialEn) {
  Serial.begin(9600);
  while(!Serial) {
    delay(100);
  }
  Serial.print(F("UART CONNECTION: SUCCESSFUL\n"));

    String mdy =  String(rtc.getMonth(Century)) + "/" + String(rtc.getDate()) + "/" + String(rtc.getYear());
    String timeStamp = String(rtc.getHour(h12, PM)) + ":" + String(rtc.getMinute()) + ":" + String(rtc.getSecond());
    Serial.println(mdy);
    Serial.println(timeStamp);
  Serial.println(F("Initializing SD card..."));
  }
  // see if the card is present and can be initialized:
  if(!SD.begin(SD_CSPIN)) {
    if(serialEn) Serial.println("Card failed, or not present...restarting");
    // don't do anything more:
    delay(2000);
    resetFunc();
  }
  if(serialEn) Serial.println("card initialized.");

  //Figure out what to stamp the new filename as
  //if(digitalRead(NEW_FILE_SW)) {
    //timestampFile = String(rtc.getMonth(Century)) + "_" + String(rtc.getDate()) + "_" + String(rtc.getYear()) + "_";
    //timestampFile += String(rtc.getHour(h12, PM)) + "_" + String(rtc.getMinute()) + ".csv";
    File root = SD.open("/");
    //root.rewindDirectory();
    String filenum = printnewfileNum(root);
    if(serialEn) Serial.print("file number: ");
    if(serialEn) Serial.println(filenum);
    int tempVal = filenum.toInt();
    if(tempVal + 1 <= 999) tempVal += 1;
    if(tempVal < 10) filenum = "00" + String(tempVal);
    else if(tempVal < 100) filenum = "0" + String(tempVal);
    else filenum = String(tempVal);
    timestampFile = "data" + filenum + ".CSV";
    //open the file 5 times to make sure it exisits
    for(int i = 0; i < 5; i++) {
      File dataFile = SD.open(timestampFile, FILE_WRITE);
      dataFile.close();
      delay(50);
    } 
    if(serialEn) { 
      Serial.print(F("created new file "));
      Serial.println(timestampFile);
    }
  //}
//  else {
//    //write to ongoing csv file
//    //Check to see if datafile exists
//    if (SD.exists("dataALL.csv")) {
//      if(serialEn) Serial.println(F("dataALL.csv exists."));
//    } else {
//      if(serialEn) {
//        Serial.println(F("dataALL.csv doesn't exist."));
//        //create file if it does not exist
//        Serial.println(F("Creating dataALL.csv..."));
//      }
//      File dataFile = SD.open("dataALL.csv", FILE_WRITE);
//      delay(50);
//      dataFile.close();
//    }
//  }
  
//  Serial.println("SPS-30 reset...");
//  delay(1000);
//  SetPointer(0xD3, 0x04);
//  delay(1000);
  if(serialEn) {
    Serial.print(F("Initializing SPS30-PM Sensor...."));
    Serial.println(F("sensor initialized."));
  }
}




void loop() {
/*
Serial.print("DIP 1 val: ");
Serial.println(digitalRead(IDLE_SW));
delay(1000);
return;
*/
/*
// RESET device
  delay(1000);
  SetPointer(0xD3, 0x04);
  delay(1000);

String valTypes[] = {"PM1.0","PM2.5","PM4.0","PM10",
                     "PM0.5","PM1.0","PM2.5","PM4.0","PM10",
                     "Particle Size"};  
                     */
 byte ND[60];
 byte w1, w2,w3;
 long tmp;
 float measure;
 float pm25_1 = 0, pm25_2 = 0, avg_size = 0;
 bool serialEn = bool(digitalRead(SERIAL_ENABLE));
 //File dataFile;

//dipswitch checks
if(!digitalRead(IDLE_SW)) {
  if(serialEn) Serial.println(F("IDLE MODE ENABLED (checks every 5 sec)"));
//  if(sdMissing) {
//    if(!SD.begin(SD_CSPIN)) {
//      if(serialEn) Serial.println(F("SD Card Not Inserted...Continuing IDLE MODE"));
//    }
//    else {
//      sdMissing = false;
//      return;
//    }
//  }
  delay(5000);
  return;
}
 

//Start Measurement
  Wire.beginTransmission(ADDRESS);
  Wire.write(0x00);
  Wire.write(0x10);
  Wire.write(0x03);
  Wire.write(0x00);
  uint8_t data[2]={0x03, 0x00};
  Wire.write(CalcCrc(data));
  Wire.endTransmission();

  if(serialEn) Serial.print(F("Warm up PM sensor (10 sec)"));
  for(int i = 0; i < 10; i++) {
    delay(1000);
    if(serialEn) Serial.print(".");
  }
  if(serialEn) Serial.println(F("Starting measurements"));
/*
//Start Fan Cleaning
  Serial.println("clean");
  Start fan cleaning
  SetPointer(0x56, 0x07);
  delay(12000);
  Serial.println("clean end");
  delay(100);
*/
 while(1){
  //if idle mode enabled, go to idle mode
  if(!digitalRead(IDLE_SW)) return;
  //check if serial mode is changed
  serialEn = bool(digitalRead(SERIAL_ENABLE));

  if(serialEn) Serial.println(F(" ---------- "));
  //Read data ready flag
  int j = 0;
  SetPointer(0x02, 0x02);
  Wire.requestFrom(ADDRESS, 3);
  w1=Wire.read();
  w2=Wire.read();
  w3=Wire.read();
  if (w2==0x01){              //0x01: new measurements ready to read
    SetPointer(0x03,0x00);
    Wire.requestFrom(ADDRESS, 60);
    for(int i=0;i<60;i++) { ND[i]=Wire.read(); }                       
    for(int i=0;i<60;i++) { 
       if ((i+1)%3==0)
       {
         byte datax[2]={ND[i-2], ND[i-1]};
         //Serial.print("crc: ");Serial.print(CalcCrc(datax),HEX);
         //Serial.print("  "); Serial.println(ND[i],HEX);
         if(tmp==0) {
           tmp= ND[i-2]; 
           tmp= (tmp<<8) + ND[i-1]; 
          }
         else{
           tmp= (tmp<<8)+ ND[i-2];
           tmp= (tmp<<8) + ND[i-1];
           //Serial.print(tmp,HEX);Serial.print(" ");
           measure= (*(float*) &tmp);
           if(j == 1)
              pm25_1 = measure;
           if(j == 6)
              pm25_2 = measure;
           if(j == 9)
              avg_size = measure;
           tmp=0;
           j++;
         }
         
       }
      }
      
  //delay(50);
  
  String mdy =  String(rtc.getMonth(Century)) + "," + String(rtc.getDate()) + "," + String(rtc.getYear()) + ",";
  String timeStamp = String(rtc.getHour(h12, PM)) + "," + String(rtc.getMinute()) + "," + String(rtc.getSecond()) + ",";
  String dataBlock = mdy + timeStamp + data2string(pm25_1,pm25_2,avg_size);
  
  //if(digitalRead(NEW_FILE_SW)) {
    File dataFile = SD.open(timestampFile, FILE_WRITE);
  //}
  //else {
    //dataFile = SD.open("data.csv", FILE_WRITE);
  //}
  //if(dataFile) {
    dataFile.println(dataBlock);
    dataFile.close();
  //}
  //Check if card is still inserted
//  if(!SD.begin(SD_CSPIN)) {
//    if(serialEn) Serial.println("Card failed, or not present...IDLE MODE UNTIL INSERT");
//    sdMissing = true;
//    // don't do anything more:
//    delay(1000);
//    return;
//  }
  if(serialEn) Serial.println(dataBlock);
  }
  delay(SAMPLE_PERIOD);
}

//  Stop Meaurement
//  SetPointer(0x01, 0x04);
 
}

void SetPointer(byte P1, byte P2)
{
  Wire.beginTransmission(ADDRESS);
  Wire.write(P1);
  Wire.write(P2);
  Wire.endTransmission();
}

String data2string(float pm25_1,float pm25_2,float avg_size) {
  String out = String(pm25_1) + "," + String(pm25_2) + "," + String(avg_size);
  return out;
}

String printnewfileNum(File dir) {
  String prevEntry = "data000.CSV";
  int counter = 0;
  
  while (true) {
    
    File entry =  dir.openNextFile();
    if (! entry) {
      // no more files
      if(counter == 0) prevEntry = "data000.CSV";
      prevEntry = prevEntry.substring(4,7);
      entry.close();
      return prevEntry;
    }
    counter++;
    //Serial.println(String(entry.name()));
    //Serial.println(String(entry.name()).charAt(0));
    if(String(entry.name()).charAt(0) != 'S') {
      if((prevEntry.substring(4,7)).toInt() < (String(entry.name()).substring(4,7)).toInt()) prevEntry = entry.name();
    }
    delay(20);
    //Serial.println(prevEntry.substring(4,7));
    entry.close();
  }
}

// from datasheet:
byte CalcCrc(byte data[2]) {
  byte crc = 0xFF;
  for(int i = 0; i < 2; i++) {
    crc ^= data[i];
    for(byte bit = 8; bit > 0; --bit) {
      if(crc & 0x80) {
      crc = (crc << 1) ^ 0x31u;
      } else {
        crc = (crc << 1);
       }
     }
   }
  return crc;
}
