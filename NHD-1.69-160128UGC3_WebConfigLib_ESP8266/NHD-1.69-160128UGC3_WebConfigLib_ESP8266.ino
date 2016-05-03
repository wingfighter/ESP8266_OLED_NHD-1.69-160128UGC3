
//---------------------------------------------------------
/*
 NHD-1.69-160128UGC3_WebConfigLib_ESP8266.ino
 V 0.33
 Program for writing to Newhaven Display's 160x128 Graphic Color OLED with SEPS525 controller.
 Using WebConfig fpr MQTT, WIFI.... 
 
 Display NHD-1.69-160128UGC3
 ------> http://www.newhavendisplay.com/nhd169160128ugc3-p-5603.html

 ESP8266 Basic Library - WEBconfig/MQTT/OTA
 ------> https://github.com/Pfannex/ESP8266_Basic
 
 This code is written for the Arduino Uno R3.
 
 Modifyed to ESP8266-12E NodeMCU - RL
 ------> https://github.com/wingfighter/ESP8266_OLED_NHD-1.69-160128UGC3.git  
 
 Copyright (c) 2015 - Newhaven Display International, LLC.
 Copyright (c) 2016 - wingfighter
*/


#include <ESP8266WiFi.h>
#include <DHT.h>
#include <ESP8266WebServer.h>
#include <Time.h>
#include <TimeLib.h>
//#include <PubSubClient.h>
#include "ESP8266_Basic.h"
#include <FS.h>
#include "Font.h"
 
//---------------------------------------------------------

#define DHTTYPE DHT22
#define DHTPIN D6  //GPIO12  - Daten
#define DHTVDD D7  //GPIO13  - VDD


#define   SDI_PIN    2    // SDI (serial mode) signal connected to D4
#define   SCL_PIN    0    // SCL (serial mdoe) signal connected to D3
#define    RS_PIN    4    // RS (D/C) signal connected to          D2
#define   RES_PIN    5    // /RES signal connected to              D1
#define    CS_PIN   14    // /CS signal connected to               D5

#define    RED  0x0000FF
#define  GREEN  0x00FF00
#define   BLUE  0xFF0000
#define  WHITE  0xFFFFFF
#define  BLACK  0x000000
#define YELLOW  0x00FFEE

// Drivinig_Current
#define R10H 0x56         //Red
#define G11H 0x4D         //Green
#define B12H 0x46         //Blue

// #define LDC TRUE          // Helligkeitssteuerung über Fotowiederstand an A0
#ifdef LDC
  #define SENSOR A0       // ADC Pin
  unsigned int sv = 0;    // Sensor Value A0  
#endif

#define BUFFPIXEL 20

#define DBG_OUTPUT_PORT Serial




// Create espClient from Basic_lib
ESP8266_Basic espClient;

unsigned long ulReqcount;
unsigned long ulReconncount;

//NTP
/* Don't hardwire the IP address or we won't get the benefits of the pool.
 *  Lookup the IP address for the host name instead */
IPAddress timeServerIP;                        // time.nist.gov NTP server address
const char* ntpServerName = "time.nist.gov";
int gmt = 1;                                  // Zeitzone
const int NTP_PACKET_SIZE = 48;               // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE];          // buffer to hold incoming and outgoing packets
unsigned long previousMillisNTP = 0;          // will store last NTP was read
const unsigned long intervalNTP = 86400000;            // interval at which to sync Clock with NTP (86400000ms = 24h)
bool setNTP_OK = false;                       // true if set Time done

// Initialize DHT sensor 
// NOTE: For working with a faster than ATmega328p 16 MHz Arduino chip, like an ESP8266,
// you need to increase the threshold for cycle counts considered a 1 or 0.
// You can do this by passing a 3rd parameter for this threshold.  It's a bit
// of fiddling to find the right value, but in general the faster the CPU the
// higher the value.  The default for a 16mhz AVR is a value of 6.  For an
// Arduino Due that runs at 84mhz a value of 30 works.
// This is for the ESP8266 processor on ESP-01 
DHT dht(DHTPIN, DHTTYPE, 30);             // 30 works fine for ESP8266-12E

float humidity, temp_f;                   // Values read from sensor
float temp = 0.0;
float hum = 0.0;
float diff = 0.1; 

// Generally, you should use "unsigned long" for variables that hold time
unsigned long previousMillis = 0;        // will store last temp was read
const unsigned long interval = 2000;              // interval at which to read sensor

unsigned long previousMillisMQTT = 0;        // will store last MQTT was send


ESP8266WebServer FSBrowserServer(81);
File fsUploadFile;

/*********************************/
/******** WEATHER TABLE  *********/
/************* START *************/
/*********************************/

const char weatherIconFileName[30][35] = {     // Weather icons 30 Dateien, max 30 Zeichen

    {"clear-icon.bmp"},
    {"clear-night-icon.bmp"},
    {"clouds-icon.bmp"},
    {"clouds-night-icon.bmp"},
    {"few-clouds-icon.bmp"},
    {"few-clouds-night-icon.bmp"},
    {"freezing-rain-icon.bmp"},
    {"hail-icon.bmp"},
    {"many-clouds-icon.bmp"},
    {"showers-day-icon.bmp"},
    {"showers-icon.bmp"},
    {"showers-night-icon.bmp"},
    {"showers-scattered-day-icon.bmp"},
    {"showers-scattered-night-icon.bmp"},
    {"snow-icon.bmp"},
    {"snow-rain-icon.bmp"},
    {"snow-scattered-day-icon.bmp"},
    {"snow-scattered-icon.bmp"},
    {"snow-scattered-night-icon.bmp"},
    {"storm-day-icon.bmp"},
    {"storm-icon.bmp"},
    {"storm-night-icon.bmp"},
    {"Wind-Flag-Storm-icon.bmp"},
    {"sunny-icon.bmp"},
    {"sunny-icon.bmp"},
    {"sunny-icon.bmp"},
    {"sunny-icon.bmp"},
    {"sunny-icon.bmp"},
    {"sunny-icon.bmp"},
    {"sunny-icon.bmp"},
};

/*********************************/
/******** WEATHER TABLE  *********/
/************** END **************/
/*********************************/

/*===============================*/
/*========== NTP read ===========*/
/*=========== BEGIN =============*/
/*===============================*/


// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;            // Stratum, or type of clock
  packetBuffer[2] = 6;            // Polling Interval
  packetBuffer[3] = 0xEC;         // Peer Clock Precision
  
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  espClient.udp.beginPacket(address, 123); //NTP requests are to port 123
  espClient.udp.write(packetBuffer, NTP_PACKET_SIZE);
  espClient.udp.endPacket();
}

boolean isSchaltjahr( int year)
{
  if (year % 4 ==0)
    {
    if (year % 100==0)
      {
      if (year %400 ==0)
        return true;
      else
        return false;
      }
    else
      return true;
    }
  else
   return false;
}


int ntp_daysofyear(int y){
  if(isSchaltjahr(y)) 
    return(366);
  else 
    return(365);
}

unsigned char ntp_daysofmonth(int year,unsigned char m)
{
  unsigned char table[] = {0,31,28,31,30,31,30,31,31,30,31,30,31};

  if(m==2) // Februar
  {
    if(isSchaltjahr(year))
      return(29);
    else 
      return(28);
  }

  if (m>12 || m<1) return(0);
    return(table[m]);
}

boolean summertime_EU(int year, byte month, byte day, byte hour, byte tzHours)
// European Daylight Savings Time calculation by "jurs" for German Arduino Forum
// input parameters: "normal time" for year, month, day, hour and tzHours (0=UTC, 1=MEZ)
// return value: returns true during Daylight Saving Time, false otherwise
{
 if (month<3 || month>10) return false; // keine Sommerzeit in Jan, Feb, Nov, Dez
 if (month>3 && month<10) return true; // Sommerzeit in Apr, Mai, Jun, Jul, Aug, Sep
 if (month==3 && (hour + 24 * day)>=(1 + tzHours + 24*(31 - (5 * year /4 + 4) % 7)) || month==10 && (hour + 24 * day)<(1 + tzHours + 24*(31 - (5 * year /4 + 1) % 7)))
   return true;
 else
   return false;
}


void getNTPTime()
{
  int cb;
  //get a random server from the pool
  WiFi.hostByName(ntpServerName, timeServerIP); 

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  //milli_delay(1000);
  // max. 10s auf Antwort vom Zeitserver warten
  unsigned long now = millis();
  while ((!cb) && ((millis()-now) < 10000))
    cb = espClient.udp.parsePacket();
  if (!cb) {
    Serial.println("no packet yet");
  }
  else 
  {
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    espClient.udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    if (secsSince1900 == 0) {setNTP_OK = false; return;};  // Keine Zeit empfangen
    Serial.print("Seconds since Jan 1 1900 = " );
    
    // gmt verschiebung
    secsSince1900 += (gmt*3600);
    Serial.println(secsSince1900);
 
    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);
    
        //in Zeit umwandeln
    int hr = 0;
    int min = 0;
    int sec = 0;
    int day = 0;
    int month = 1;
    int year = 1970;
    
    // print the hour, minute and second:
    Serial.print("The MEZ time is ");       // UTC is the time at Greenwich Meridian (GMT)
    hr=(int)((epoch  % 86400L) / 3600);
    Serial.print(hr); // print the hour (86400 equals secs per day)

    Serial.print(':');
    if ( ((epoch % 3600) / 60) < 10 ) {
      // In the first 10 minutes of each hour, we'll want a leading '0'
      Serial.print('0');
    }
    min=(int)((epoch  % 3600) / 60);
    Serial.print(min); // print the minute (3600 equals secs per minute)

    Serial.print(':');
    if ( (epoch % 60) < 10 ) {
      // In the first 10 seconds of each minute, we'll want a leading '0'
      Serial.print('0');
    }
    sec=(int)(epoch % 60);
    Serial.println(sec); // print the second

    // Tage berechnen
    day = (epoch/86400)+1;

    // Jahr berechnen
    while((day-ntp_daysofyear(year))>0)
    {
      day -= ntp_daysofyear(year);
        year++;
    }

    // Monate und Tage ausrechnen
    while((day-ntp_daysofmonth(year,month))>0)
    {
      day -= ntp_daysofmonth(year,month);
        month++;
    }
    
    // print the day of month
    if ( (day) < 10 ) {
      // In the first 10 days of each month, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print(day); 
    Serial.print('.');
    
    // print the month of year
    if ( (month) < 10 ) {
      // In the first 10 month of each year, we'll want a leading '0'
      Serial.print('0');
    }
    Serial.print(month); 
    Serial.print('.');
    
    // print the year
    Serial.println(year); 

    // Sommer- Winterzeit-Umschaltung
    // Da dafÃ¼r das Datum benÃ¶tigt wird, kann es erst hier ermittelt werden
    if (summertime_EU(year, month, day, hr, gmt))
    {
        hr += 1;      // Sommerzeit
        Serial.println("Sommerzeit"); 
    }
    
    // set internal clock
    setTime(hr, min, sec, day, month, year);
    Serial.println("!new Time set");
    setNTP_OK = true;
  }
}

// Fuehrende '0' bei mmehrstelligen Zahlen ausgeben
String printDigits(int digits)
{
  // Utility function for digital clock display: prints leading 0
  String sDigit="";
  if(digits < 10)
    sDigit='0';
  sDigit += String(digits);
  return sDigit;
}
 
/*===============================*/
/*========== NTP read ===========*/
/*============ END ==============*/
/*===============================*/


/*===============================*/
/*========== DHT read ===========*/
/*=========== BEGIN =============*/
/*===============================*/

void getTemperature() {
  // Wait at least 2 seconds seconds between measurements.
  // if the difference between the current time and last time you read
  // the sensor is bigger than the interval you set, read the sensor
  // Works better than delay for things happening elsewhere also
  unsigned long currentMillis = millis();
 
  if(currentMillis - previousMillis >= interval) {
    // save the last time you read the sensor 
    previousMillis = currentMillis;   
 
    // Reading temperature for humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (it's a very slow sensor)
    humidity = dht.readHumidity();          // Read humidity (percent)
    temp_f = dht.readTemperature(false);     // Read temperature if trus as Fahrenheit, if false as Celsius
    // Check if any reads failed and exit early (to try again).
    if (isnan(humidity) || isnan(temp_f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }
  }
}

/*===============================*/
/*========== DHT read ===========*/
/*=========== BEGIN =============*/
/*===============================*/


/*********************************/
/****** LOW LEVEL FUNCTIONS ******/
/************* START *************/
/*********************************/

//wait without delay()
void milli_delay( unsigned long ms){
//  DBG_OUTPUT_PORT.print("delay: ");
//  DBG_OUTPUT_PORT.println(ms);
  unsigned long count=0;
  while (count < ms){
      delay(1);
      count++;
      espClient.handle_connections();
      FSBrowserServer.handleClient();
  }    
}

byte minValue( byte cR10h, byte cG11h, byte cB12h){
byte minimum = 0;   
  if (cR10h < cG11h) 
  {
    minimum = cR10h; 
    if (cB12h < cR10h)
    {
      minimum = cB12h;
    }
  }
  else 
  {
    minimum = cG11h;
    if (cB12h < cG11h)
    {
      minimum = cB12h;
    }
    
  }
  return minimum;
} 

byte maxValue( byte cR10h, byte cG11h, byte cB12h){
byte maximum = 0;   
  if (cR10h > cG11h) 
  {
    maximum = cR10h; 
    if (cB12h > cR10h)
    {
      maximum = cB12h;
    }
  }
  else 
  {
    maximum = cG11h;
    if (cB12h > cG11h)
    {
      maximum = cB12h;
    }
    
  }
  return maximum;
} 


//format bytes
String formatBytes(size_t bytes){
  if (bytes < 1024){
    return String(bytes)+"B";
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0)+"KB";
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0)+"MB";
  } else {
    return String(bytes/1024.0/1024.0/1024.0)+"GB";
  }
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
  return (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff) && (newValue != NAN);
}

void OLED_Command_160128RGB(unsigned char c)        // send command to OLED
{
    unsigned char i;
    unsigned char mask = 0x80;
    
            digitalWrite(CS_PIN, LOW);
            digitalWrite(RS_PIN, LOW);
            for(i=0;i<8;i++)
            {
                digitalWrite(SCL_PIN, LOW);
                if((c & mask) >> 7 == 1)
                {
                    digitalWrite(SDI_PIN, HIGH);
                }
                else
                {
                    digitalWrite(SDI_PIN, LOW);
                }
                digitalWrite(SCL_PIN, HIGH);
                c = c << 1;
            }
            digitalWrite(CS_PIN, HIGH);
}

void OLED_Data_160128RGB(unsigned char d)        // send data to OLED
{
    unsigned char i;
    unsigned char mask = 0x80;
    
            digitalWrite(CS_PIN, LOW);
            digitalWrite(RS_PIN, HIGH);
            for(i=0;i<8;i++)
            {
                digitalWrite(SCL_PIN, LOW);
                if((d & mask) >> 7 == 1)
                {
                    digitalWrite(SDI_PIN, HIGH);
                }
                else
                {
                    digitalWrite(SDI_PIN, LOW);
                }
                digitalWrite(SCL_PIN, HIGH);
                d = d << 1;
            }
            digitalWrite(CS_PIN, HIGH);
}

void OLED_SerialPixelData_160128RGB(unsigned char d)    // serial write for pixel data
{
    unsigned char i;
    unsigned char mask = 0x80;
    digitalWrite(CS_PIN, LOW);
    digitalWrite(RS_PIN, HIGH);
    for(i=0;i<6;i++)
    {
        digitalWrite(SCL_PIN, LOW);
        if((d & mask) >> 7 == 1)
        {
            digitalWrite(SDI_PIN, HIGH);
        }
        else
        {
            digitalWrite(SDI_PIN, LOW);
        }
        digitalWrite(SCL_PIN, HIGH);
        d = d << 1;
    }
    digitalWrite(CS_PIN, HIGH);
}

void OLED_SetColumnAddress_160128RGB(unsigned char x_start, unsigned char x_end)    // set column address start + end
{
    OLED_Command_160128RGB(0x17);
    OLED_Data_160128RGB(x_start);
    OLED_Command_160128RGB(0x18);
    OLED_Data_160128RGB(x_end);
}

void OLED_SetRowAddress_160128RGB(unsigned char y_start, unsigned char y_end)    // set row address start + end
{
    OLED_Command_160128RGB(0x19);
    OLED_Data_160128RGB(y_start);
    OLED_Command_160128RGB(0x1A);
    OLED_Data_160128RGB(y_end);
}

void OLED_WriteMemoryStart_160128RGB(void)    // write to RAM command
{
    OLED_Command_160128RGB(0x22);
}

void OLED_Pixel_160128RGB(unsigned long color)    // write one pixel of a given color
{
            OLED_SerialPixelData_160128RGB((color>>16));
            OLED_SerialPixelData_160128RGB((color>>8));
            OLED_SerialPixelData_160128RGB(color);
}

void OLED_SetPosition_160128RGB(unsigned char x_pos, unsigned char y_pos)    // set x,y address
{
    OLED_Command_160128RGB(0x20);
    OLED_Data_160128RGB(x_pos);
    OLED_Command_160128RGB(0x21);
    OLED_Data_160128RGB(y_pos);
}

void OLED_FillArea_160128RGB(unsigned char x_start, unsigned char x_end, unsigned char y_start, unsigned char y_end, unsigned long color)    // fill area with a given color
{
    unsigned int i, j;
    for(i=0;i<(y_end-y_start);i++)
    {
        OLED_SetPosition_160128RGB(x_start, y_start+i);
        OLED_WriteMemoryStart_160128RGB();
        for(j = 0; j < ((x_end-x_start)*3); j += 3)
        {
           OLED_Pixel_160128RGB(color);
        }
    }
}


void OLED_FillScreen_160128RGB(unsigned long color)    // fill screen with a given color
{
    unsigned int i;
    unsigned int x_start = 0;
    unsigned int x_end = 159;
    unsigned int y_start = 0;
    unsigned int y_end = 127;
    OLED_SetColumnAddress_160128RGB(0, 159);
    OLED_SetRowAddress_160128RGB(0, 127);
    milli_delay(2);
    OLED_SetPosition_160128RGB(x_start, y_start);
    OLED_WriteMemoryStart_160128RGB();
    for(i=0;i<((x_end - x_start + 1) * (y_end - y_start + 1 ));i++)
    {
        OLED_Pixel_160128RGB(color);
    }
//      OLED_FillArea_160128RGB(0, 159, 0, 127, BLACK);

}


void OLED_Driving_Current_160128RGB(byte r10h, byte g11h, byte b12h)      //OLED initialization
{
    OLED_Command_160128RGB(0x10);     // Set Driving Current of Red
    OLED_Data_160128RGB(r10h);        // Standard: 0x56

    OLED_Command_160128RGB(0x11);     // Set Driving Current of Green
    OLED_Data_160128RGB(g11h);        // Standard: 0x4D
 
    OLED_Command_160128RGB(0x12);     // Set Driving Current of Blue
    OLED_Data_160128RGB(b12h);        // Standard: 0x46
}

void OLED_FadeOut_160128RGB(unsigned int fadingTime)
{
#ifdef LDC
  // aktuelle Sensorwerte über A0
  byte cR10h = 223 * sv/100;
  byte cG11h = 197 * sv/100; 
  byte cB12h = 179 * sv/100;
#else
  byte cR10h = R10H; 
  byte cG11h = G11H;
  byte cB12h = B12H;
#endif

  int loop = minValue( cR10h, cG11h, cB12h);
  fadingTime = fadingTime/loop;

  cR10h = loop;
  cG11h = loop;
  cB12h = loop; 
  
  for(unsigned int i=0; i < loop; i++)
  {
     cR10h--;
     cG11h--;
     cB12h--;
     OLED_Driving_Current_160128RGB(cR10h, cG11h, cB12h);      //OLED set driving
     delay(fadingTime*i);
  }
}


void OLED_FadeIn_160128RGB(unsigned int fadingTime)
{
#ifdef LDC
  // aktuelle Sensorwerte über A0
  byte cR10h = 223 * sv/100;
  byte cG11h = 197 * sv/100; 
  byte cB12h = 179 * sv/100;
#else
  byte cR10h = R10H; 
  byte cG11h = G11H;
  byte cB12h = B12H;
#endif
  byte iR10h = 0;
  byte iG11h = 0;
  byte iB12h = 0;
 
  int loop = maxValue( cR10h, cG11h, cB12h);
  fadingTime = fadingTime/loop;
  
  for(unsigned int i=1; i < loop; i++)
  {
     if( iR10h < cR10h) iR10h++;
     if( iG11h < cG11h) iG11h++;
     if( iB12h < cB12h) iB12h++;
     OLED_Driving_Current_160128RGB(iR10h, iG11h, iB12h);      //OLED set driving
     delay(fadingTime/i);
  }
}


/*===============================*/
/*===== LOW LEVEL FUNCTIONS =====*/
/*============= END =============*/
/*===============================*/


/*********************************/
/***** HIGH LEVEL FUNCTIONS ******/
/************ START **************/
/*********************************/

void OLED_smallText_160128RGB(unsigned char x_pos, unsigned char y_pos, unsigned int letter, unsigned long textColor, unsigned long backgroundColor)  // function to show letter
{
    int row;                                     // Anzahl Zeilen=Zeichenhöhe Zähler
    int j;                                       // Bytes/Zeile Zähler
    int count;                                   // BitzÃ¤hler 
    int byteRow = (int)smallFontArrayInfo[letter][0];   // Bytes pro Zeile
    int rowChar = (int)smallFontArrayInfo[letter][1];   // Zeilen pro Zeichen
    int arrayPos = (int)smallFontArrayInfo[letter][2];  // Beginn-Position in bigArray
    unsigned char mask = 0x80;                   // Biteweise  ausmaskieren

    
    for(row=0;row <rowChar;row++)        // Zeichenhöhe
    {
        OLED_SetPosition_160128RGB(x_pos,y_pos);
        OLED_WriteMemoryStart_160128RGB();
        for (j=0;j < byteRow;j++)                     // Bytes/Zeile
        {
            for (count=0;count < 8;count++)            // Pixel/Byte
            {
                if((smallFontArray[arrayPos] & mask) == mask)
                    OLED_Pixel_160128RGB(textColor);
                else
                   OLED_Pixel_160128RGB(backgroundColor);
                mask = mask >> 1;
            }
            arrayPos++;
            mask = 0x80;
        }
        y_pos--;
    }
} 
 
void OLED_bigText_160128RGB(unsigned char x_pos, unsigned char y_pos, unsigned int letter, unsigned long textColor, unsigned long backgroundColor)  // function to show letter
{
    int row;                                     // Anzahl Zeilen=Zeichenhöhe Zähler
    int j;                                       // Bytes/Zeile Zähler
    int count;                                   // Bitzähler 
    int byteRow = (int)bigFontArrayInfo[letter][0];   // Bytes pro Zeile
    int rowChar = (int)bigFontArrayInfo[letter][1];   // Zeilen pro Zeichen
    int arrayPos = (int)bigFontArrayInfo[letter][2];  // Beginn-Position in bigArray
    unsigned char mask = 0x80;                   // Biteweise  ausmaskieren

    
    for(row=0;row<rowChar;row++)        // Zeichenhöhe
    {
        OLED_SetPosition_160128RGB(x_pos,y_pos);
        OLED_WriteMemoryStart_160128RGB();
        for (j=0;j<byteRow;j++)                     // Bytes/Zeile
        {
            for (count=0;count<8;count++)            // Pixel/Byte
            {
                if((bigFontArray[arrayPos] & mask) == mask)
                    OLED_Pixel_160128RGB(textColor);
                else
                   OLED_Pixel_160128RGB(backgroundColor);
                mask = mask >> 1;
            }
            arrayPos++;
            mask = 0x80;
        }
        y_pos--;
    }
}



int countPixel(const char array_of_string[])
{
  unsigned int xCharSpace = 0;  // Abstand zwischen den Zeichen
  int hPixel = 0;    
  unsigned int smallFontArrayPos = 0; 
    
    for (int i=0;i < strlen(array_of_string); i++){
       switch (array_of_string[i]) 
       {
         case ' ':
            //if ' ' then print '0'
            smallFontArrayPos = 11;
            break;
         case '*':
            //Array-Pos for '°'
            smallFontArrayPos = 139;
            break;
         default: 
            //Array-Pos for '0' to '9' 
            smallFontArrayPos = array_of_string[i]-32;
            break;
      }
      hPixel += (int)smallFontArrayInfo[smallFontArrayPos][0]*8  + xCharSpace; 
    }
 return hPixel;
}

void OLED_StringSmallFont_160128RGB(unsigned char x_pos, unsigned char y_pos, const char array_of_string[], unsigned long textColor, unsigned long backgroundColor)  // function to show Number in Verdana
{
    unsigned int xCharPos = 0;    // x-Position Zeichen auf der Zeile
    unsigned int xCharSpace = 1;  // Abstand zwischen den Zeichen
    unsigned int smallFontArrayPos = 0; 

    
    for (int i=0;i < strlen(array_of_string); i++)
    {                      // Buchstabenabstand, Zeile, wenn Dezimalpunkt - dann andere Stelle in Font-Array abfragen, sonst Zahlenwert-Position  
       switch (array_of_string[i]) 
       {
         case ' ':
            //if ' ' then print '0'
//            smallFontArrayPos = 11;
            break;
         case '*':
            //Array-Pos for '°'
//            smallFontArrayPos = 139;
//            xCharPos -= 3;
            break;
         default: 
            //Array-Pos for '0' to '9' 
            smallFontArrayPos = array_of_string[i] - 32;
            break;
     }
//  Serial.print("xCharPos :");
//  Serial.print(xCharPos); Serial.print("->");Serial.println(array_of_string[i]);
     
     OLED_smallText_160128RGB(x_pos + xCharPos, y_pos, smallFontArrayPos,  textColor, backgroundColor);
     xCharPos = xCharPos + (int)smallFontArrayInfo[smallFontArrayPos][0]*8 + xCharSpace;
    }      
}


void OLED_StringBigFont_160128RGB(unsigned int x_pos, unsigned int y_pos, const char array_of_string[], unsigned long textColor, unsigned long backgroundColor)  // function to show Number in Verdana
{
    unsigned int xCharPos = 0;    // x-Position Zeichen auf der Zeile
    unsigned int xCharSpace = 2;  // Abstand zwischen den Zeichen
    unsigned int bigFontArrayPos = 0; 

    
    for (int i=0;i < strlen(array_of_string); i++)
    {                      // Buchstabenabstand, Zeile, wenn Dezimalpunkt - dann andere Stelle in Font-Array abfragen, sonst Zahlenwert-Position  
       switch (array_of_string[i]) 
       {
         case ' ':
            //if ' ' then print '0'
            bigFontArrayPos = 11;
            break;
         case '*':
            //Array-Pos for '°'
            bigFontArrayPos = 139;
            xCharPos -= 3;
            break;
 //        case '0': 
 //           bmpDraw("0_30pt.bmp", x_pos + xCharPos, y_pos);
         default: 
            //Array-Pos for '0' to '9' 
            bigFontArrayPos = array_of_string[i]-37;
            break;
     }
     // if (array_of_string[i] != '0') 
     OLED_bigText_160128RGB(x_pos + xCharPos, y_pos, bigFontArrayPos,  textColor, backgroundColor);
     xCharPos = xCharPos + (int)bigFontArrayInfo[bigFontArrayPos][0]*8 + xCharSpace;   // 8 Bit * Array[0]
    }      
}


/*===============================*/
/*==== HIGH LEVEL FUNCTIONS =====*/
/*============= END =============*/
/*===============================*/

/*********************************/
/***** WRITE BMP TO DISPLAY ******/
/************ START **************/
/*********************************/

uint16_t read16(File &f) {
  uint16_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read(); // MSB
  return result;
}

uint32_t read32(File &f) {
  uint32_t result;
  ((uint8_t *)&result)[0] = f.read(); // LSB
  ((uint8_t *)&result)[1] = f.read();
  ((uint8_t *)&result)[2] = f.read();
  ((uint8_t *)&result)[3] = f.read(); // MSB
  return result;
}

void bmpDraw(const char *filename, uint8_t x, uint8_t y) {
  File     bmpFile;
  bool     readOK = false;
  uint16_t bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbufferLen = BUFFPIXEL * 3;
  uint8_t  sdbuffer[sdbufferLen]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sdbufferLen; // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  uint16_t w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0;



//    if((x >= tft.width()) || (y >= tft.height())) return;

  //clean FS, for testing
  //SPIFFS.format();
  
  //read configuration from FS json
  Serial.println("");
  Serial.println("mounting FS...for BMPFile");

  
  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/"+String(filename))) {
      //file exists, reading and loading
      Serial.println("reading BMPfile: /"+String(filename));
      bmpFile = SPIFFS.open("/"+String(filename), "r");
      // Parse BMP header
      if(read16(bmpFile) == 0x4D42) { // BMP signature
        read32(bmpFile);
        (void)read32(bmpFile); // Read & ignore creator bytes
        bmpImageoffset = read32(bmpFile); // Start of image data
        // Read DIB header
        read32(bmpFile);
        bmpWidth  = read32(bmpFile);
        bmpHeight = read32(bmpFile);
        if(read16(bmpFile) == 1) { // # planes -- must be '1'
          bmpDepth = read16(bmpFile); // bits per pixel
          if((bmpDepth == 24) && (read32(bmpFile) == 0)) { // 0 = uncompressed
            goodBmp = true; // Supported BMP format -- proceed!
            rowSize = (bmpWidth * 3 + 3) & ~3;// BMP rows are padded (if needed) to 4-byte boundary
            if (bmpHeight < 0) {
              bmpHeight = -bmpHeight;
              flip      = false;
            }
            // Crop area to be loaded
            w = bmpWidth;
            h = bmpHeight;
            if((x+w-1) >= 160) w = 160 - x;
            if((y+h-1) >= 128) h = 128 - y;
            for (row=0; row<h; row++) { // For each scanline...
              if (!flip){ // Bitmap is stored bottom-to-top order (normal BMP)
                pos = bmpImageoffset + (bmpHeight - 1 - row) * rowSize;
              } else {     // Bitmap is stored top-to-bottom
                pos = bmpImageoffset + row * rowSize;
              }
              if (bmpFile.position() != pos) { // Need seek?
                bmpFile.seek(pos,SeekSet);
                buffidx = sdbufferLen; // Force buffer reload
              }
              OLED_SetPosition_160128RGB(x, y+row);
              OLED_WriteMemoryStart_160128RGB();
              for (col=0; col<w; col++) { // For each pixel...
                // Time to read more pixel data?
                if (buffidx >= sdbufferLen) { // Indeed
                  bmpFile.read(sdbuffer, sdbufferLen);
                  buffidx = 0; // Set index to beginning
                }
                // Convert pixel from BMP to TFT format, push to display
                OLED_SerialPixelData_160128RGB(sdbuffer[buffidx++]);  // B
                OLED_SerialPixelData_160128RGB(sdbuffer[buffidx++]);  // G
                OLED_SerialPixelData_160128RGB(sdbuffer[buffidx++]);  // R
              } // end pixel
            } // end scanline
//            tft.endPushData();
          } // end goodBmp
        }
      }

      bmpFile.close();
      if(!goodBmp) {
 //       tft.setCursor(0,0);
 //       tft.print("file unrecognized!");
      }
    }else{
      Serial.println(filename + String(" does not exist")); 
    }
  } else {
    Serial.println("failed to mount FS");
  }   
}


/*===============================*/
/***** WRITE BMP TO DISPLAY ******/
/************* END ***************/
/*===============================*/

/*===============================*/
/***** SELECT WEATHER ICON *******/
/************* START *************/
/*===============================*/

/*
Code Description
0   tornado
1   tropical storm
2   hurricane
3   severe thunderstorms
4   thunderstorms
5   mixed rain and snow
6   mixed rain and sleet
7   mixed snow and sleet
8   freezing drizzle
9   drizzle
10  freezing rain
11  showers
12  showers
13  snow flurries
14  light snow showers
15  blowing snow
16  snow
17  hail
18  sleet
19  dust
20  foggy
21  haze
22  smoky
23  blustery
24  windy
25  cold
26  cloudy
27  mostly cloudy (night)
28  mostly cloudy (day)
29  partly cloudy (night)
30  partly cloudy (day)
31  clear (night)
32  sunny
33  fair (night)
34  fair (day)
35  mixed rain and hail
36  hot
37  isolated thunderstorms
38  scattered thunderstorms
39  scattered thunderstorms
40  scattered showers
41  heavy snow
42  scattered snow showers
43  heavy snow
44  partly cloudy
45  thundershowers
46  snow showers
47  isolated thundershowers
*/

void selectWeatherIcon(int day){  // 0=today

  DBG_OUTPUT_PORT.print("Yahoo WeatherCode: ");
  DBG_OUTPUT_PORT.println(atoi(espClient.MyWeatherIcon[day].weatherCode));
  
   switch (atoi(espClient.MyWeatherIcon[day].weatherCode)) 
  {
     case 32:                                      // sunny
     case 34:                                      // fair (day)
        bmpDraw(weatherIconFileName[0], 32, 16);   // {"clear-icon.bmp"}
        break;   
     case 33:                                      // fair (night)
     case 31:                                      // clear (night)
        bmpDraw(weatherIconFileName[1], 32, 16);   // {"clear-night-icon.bmp"}
        break;
     case 26:                                      // cloudy
     case 30:                                      // partly cloudy (day)
        bmpDraw(weatherIconFileName[2], 32, 16);   // {"clouds-icon.bmp"}
        break;
     case 29:                                      // partly cloudy (night)
        bmpDraw(weatherIconFileName[3], 32, 16);   // {"clouds-night-icon.bmp"},
        break;
     case 28:                                      // mostly cloudy (day)
        bmpDraw(weatherIconFileName[4], 32, 16);   // {"few-clouds-icon.bmp"}
        break;
     case 27:                                      // mostly cloudy (night)
        bmpDraw(weatherIconFileName[5], 32, 16);   // {"few-clouds-night-icon.bmp"},
        break;
     case 8:                                       // freezing drizzle
     case 10:                                      // freezing rain
        bmpDraw(weatherIconFileName[6], 32, 16);   // {"freezing-rain-icon.bmp"},
        break;
     case 6:                                       // mixed rain and sleet
     case 17:                                      // hail
     case 35:                                      // mixed rain and hail
        bmpDraw(weatherIconFileName[7], 32, 16);   // {"hail-icon.bmp"},
        break;
     case 19:                                      // dust
     case 20:                                      // foggy
     case 21:                                      // haze
     case 22:                                      // smoky     
        bmpDraw(weatherIconFileName[8], 32, 16);   // {"many-clouds-icon.bmp"},
        break;
     case 7:                                       // mixed snow and sleet
        bmpDraw(weatherIconFileName[9], 32, 16);   // {"showers-day-icon.bmp"},
        break;
     case 11:                                       // showers
        bmpDraw(weatherIconFileName[10], 32, 16);  // {"showers-icon.bmp"},
        break;
     case 12:                                       // showers
        bmpDraw(weatherIconFileName[11], 32, 16);  // {"showers-night-icon.bmp"},
        break;
     case 9:                                        // drizzle
        bmpDraw(weatherIconFileName[12], 32, 16);  // {"showers-scattered-day-icon.bmp"},
        break;
     case 40:                                      // scattered showers
        bmpDraw(weatherIconFileName[13], 32, 16);  // {"showers-scattered-night-icon.bmp"},
        break;
     case 15:                                      //   blowing snow
     case 16:                                      //   snow
     case 41:                                      //   heavy snow
     case 43:                                      //   heavy snow
        bmpDraw(weatherIconFileName[14], 32, 16);  // {"snow-icon.bmp"},
        break;
     case 18:                                      // sleet
     case 5:                                       // mixed rain and snow
        bmpDraw(weatherIconFileName[15], 32, 16);  // {"snow-rain-icon.bmp"},
        break;
     case 42:                                      // scattered snow showers
     case 46:                                      // scattered snow showers
        bmpDraw(weatherIconFileName[16], 32, 16);  // {"snow-scattered-day-icon.bmp"},
        break;
     case 13:                                      // snow flurries
     case 14:                                      // light snow showers
        bmpDraw(weatherIconFileName[17], 32, 16);  // {"snow-scattered-icon.bmp"},
        break;
     case 44:                                      // partly cloudy (day)
        bmpDraw(weatherIconFileName[18], 32, 16);  // {"snow-scattered-night-icon.bmp"},
        break;
     case 38:                                      // scattered thunderstorms
     case 39:                                      // scattered thunderstorms
        bmpDraw(weatherIconFileName[19], 32, 16);  // {"storm-day-icon.bmp"},
        break;
     case 3:                                       // severe thunderstorms
     case 4:                                       // thunderstorms
     case 37:                                      // isolated thunderstorms
     case 45:                                      // thunderstorms
        bmpDraw(weatherIconFileName[20], 32, 16);  // {"storm-icon.bmp"},
        break;
     case 47:                                      // isolated thunderstorms
        bmpDraw(weatherIconFileName[21], 32, 16);  // {"storm-night-icon.bmp"},
        break;
     case 23:                                      // blustery
     case 24:                                      // windy
     case 0:                                       // tornado
     case 1:                                       // tropical storm
     case 2:                                       // hurrican
        bmpDraw(weatherIconFileName[22], 32, 16);  //{"Wind-Flag-Storm-icon.bmp"},
        break;
     case 25:                                      // cold
        bmpDraw(weatherIconFileName[29], 32, 16);  // 
        break;
     case 36:                                      // hot
        bmpDraw(weatherIconFileName[29], 32, 16);  // 
        break;
    default:
        bmpDraw(weatherIconFileName[29], 32, 16);  // {"sunny-icon.bmp"},

    }
  
 }

/*===============================*/
/***** SELECT WEATHER ICON *******/
/************* END ***************/
/*===============================*/

/*********************************/
/*********** FSBrowser ***********/
/************ START **************/
/*********************************/


String getContentType(String filename){
  if(FSBrowserServer.hasArg("download")) return "application/octet-stream";
  else if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}

bool handleFileRead(String path){
  DBG_OUTPUT_PORT.println("handleFileRead: " + path);
  if(path.endsWith("/")) path += "ReadMe.txt";
  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  if(SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)){
    if(SPIFFS.exists(pathWithGz))
      path += ".gz";
    File file = SPIFFS.open(path, "r");
    size_t sent = FSBrowserServer.streamFile(file, contentType);
    file.close();
    return true;
  }
  return false;
}

void handleFileUpload(){
  if(FSBrowserServer.uri() != "/edit") return;
  HTTPUpload& upload = FSBrowserServer.upload();
  if(upload.status == UPLOAD_FILE_START){
    String filename = upload.filename;
    if(!filename.startsWith("/")) filename = "/"+filename;
    DBG_OUTPUT_PORT.print("handleFileUpload Name: "); DBG_OUTPUT_PORT.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
    filename = String();
  } else if(upload.status == UPLOAD_FILE_WRITE){
    //DBG_OUTPUT_PORT.print("handleFileUpload Data: "); DBG_OUTPUT_PORT.println(upload.currentSize);
    if(fsUploadFile)
      fsUploadFile.write(upload.buf, upload.currentSize);
  } else if(upload.status == UPLOAD_FILE_END){
    if(fsUploadFile)
      fsUploadFile.close();
    DBG_OUTPUT_PORT.print("handleFileUpload Size: "); DBG_OUTPUT_PORT.println(upload.totalSize);
    FSBrowserServer.send(200, "text/plain", "");
  }
}

void handleFileDelete(){
  if(FSBrowserServer.args() == 0) return FSBrowserServer.send(500, "text/plain", "BAD ARGS");
  String path = FSBrowserServer.arg(0);
  DBG_OUTPUT_PORT.println("handleFileDelete: " + path);
  if(path == "/")
    return FSBrowserServer.send(500, "text/plain", "BAD PATH");
  if(!SPIFFS.exists(path))
    return FSBrowserServer.send(404, "text/plain", "FileNotFound");
  SPIFFS.remove(path);
  FSBrowserServer.send(200, "text/plain", "");
  path = String();
}


void handleFileCreate(){
  if(FSBrowserServer.args() == 0)
    return FSBrowserServer.send(500, "text/plain", "BAD ARGS");
  String path = FSBrowserServer.arg(0);
  DBG_OUTPUT_PORT.println("handleFileCreate: " + path);
  if(path == "/")
    return FSBrowserServer.send(500, "text/plain", "BAD PATH");
  if(SPIFFS.exists(path))
    return FSBrowserServer.send(500, "text/plain", "FILE EXISTS");
  File file = SPIFFS.open(path, "w");
  if(file)
    file.close();
  else
    return FSBrowserServer.send(500, "text/plain", "CREATE FAILED");
  FSBrowserServer.send(200, "text/plain", "");
  path = String();
}


void handleFileList() {
  if(!FSBrowserServer.hasArg("dir")) {FSBrowserServer.send(500, "text/plain", "BAD ARGS"); return;}
  
  String path = FSBrowserServer.arg("dir");
  DBG_OUTPUT_PORT.println("handleFileList: " + path);
  Dir dir = SPIFFS.openDir(path);
  path = String();

  String output = "[";
  while(dir.next()){
    File entry = dir.openFile("r");
    if (output != "[") output += ',';
    bool isDir = false;
    output += "{\"type\":\"";
    output += (isDir)?"dir":"file";
    output += "\",\"name\":\"";
    output += String(entry.name()).substring(1);
    output += "\"}";
    entry.close();
  }
  
  output += "]";
  FSBrowserServer.send(200, "text/json", output);
}


void initFSBrowserServer() {
 //SERVER INIT
  //list directory
  FSBrowserServer.on("/list", HTTP_GET, handleFileList);
  //load editor
  FSBrowserServer.on("/edit", HTTP_GET, [](){
    if(!handleFileRead("/edit.htm")) FSBrowserServer.send(404, "text/plain", "FileNotFound");
  });
  //create file
  FSBrowserServer.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  FSBrowserServer.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  FSBrowserServer.on("/edit", HTTP_POST, [](){ FSBrowserServer.send(200, "text/plain", ""); }, handleFileUpload);

  //called when the url is not defined here
  //use it to load content from SPIFFS
  FSBrowserServer.onNotFound([](){
    if(!handleFileRead(FSBrowserServer.uri()))
      FSBrowserServer.send(404, "text/plain", "FileNotFound");
  });

  //get heap status, analog input value and all GPIO statuses in one json call
  //FSBrowserServer.on("/all", HTTP_GET, [](){
  //  String json = "{";
  //  json += "\"heap\":"+String(ESP.getFreeHeap());
  //  json += ", \"analog\":"+String(analogRead(A0));
  //  json += ", \"gpio\":"+String((uint32_t)(((GPI | GPO) & 0xFFFF) | ((GP16I & 0x01) << 16)));
  //  json += "}";
  //  FSBrowserServer.send(200, "text/json", json);
  //  json = String();
  // });
  
  FSBrowserServer.begin();
  DBG_OUTPUT_PORT.println("HTTP FSBrowserServer started");
}

/*********************************/
/*********** FSBrowser ***********/
/************ END **************/
/*********************************/

/*********************************/
/******** INITIALIZATION *********/
/************ START **************/
/*********************************/

void OLED_Init_160128RGB(void)      //OLED initialization
{
    digitalWrite(RES_PIN, LOW);
    delay(500);
    digitalWrite(RES_PIN, HIGH);
    delay(500);
    
    OLED_Command_160128RGB(0x04);// Set Normal Driving Current
    OLED_Data_160128RGB(0x03);// Disable Oscillator Power Down
    delay(2);
    OLED_Command_160128RGB(0x04); // Enable Power Save Mode
    OLED_Data_160128RGB(0x00); // Set Normal Driving Current
    delay(2); // Disable Oscillator Power Down
    OLED_Command_160128RGB(0x3B);
    OLED_Data_160128RGB(0x00);
    OLED_Command_160128RGB(0x02);
    OLED_Data_160128RGB(0x01); // Set EXPORT1 Pin at Internal Clock
    // Oscillator operates with external resister.
    // Internal Oscillator On
    OLED_Command_160128RGB(0x03);
    OLED_Data_160128RGB(0x90); // Set Clock as 90 Frames/Sec
    OLED_Command_160128RGB(0x80);
    OLED_Data_160128RGB(0x01); // Set Reference Voltage Controlled by External Resister
    OLED_Command_160128RGB(0x08);// Set Pre-Charge Time of Red
    OLED_Data_160128RGB(0x04);
    OLED_Command_160128RGB(0x09);// Set Pre-Charge Time of Green
    OLED_Data_160128RGB(0x05);
    OLED_Command_160128RGB(0x0A);// Set Pre-Charge Time of Blue
    OLED_Data_160128RGB(0x05);
    OLED_Command_160128RGB(0x0B);// Set Pre-Charge Current of Red
    OLED_Data_160128RGB(0x9D);
    OLED_Command_160128RGB(0x0C);// Set Pre-Charge Current of Green
    OLED_Data_160128RGB(0x8C);
    OLED_Command_160128RGB(0x0D);// Set Pre-Charge Current of Blue
    OLED_Data_160128RGB(0x57);
    OLED_Driving_Current_160128RGB(R10H, G11H, B12H);      //OLED initialization
    OLED_Command_160128RGB(0x13);
    OLED_Data_160128RGB(0xA0); // Set Color Sequence
    OLED_Command_160128RGB(0x14);
    OLED_Data_160128RGB(0x01); // Set MCU Interface Mode
    OLED_Command_160128RGB(0x16);
    OLED_Data_160128RGB(0x76); // Set Memory Write Mode
    OLED_Command_160128RGB(0x28);
    OLED_Data_160128RGB(0x7F); // 1/128 Duty (0x0F~0x7F)
    OLED_Command_160128RGB(0x29);
    OLED_Data_160128RGB(0x00); // Set Mapping RAM Display Start Line (0x00~0x7F)
    OLED_Command_160128RGB(0x06);
    OLED_Data_160128RGB(0x01); // Display On (0x00/0x01)
    OLED_Command_160128RGB(0x05); // Disable Power Save Mode
    OLED_Data_160128RGB(0x00); // Set All Internal Register Value as Normal Mode
    OLED_Command_160128RGB(0x15);
    OLED_Data_160128RGB(0x00); // Set RGB Interface Polarity as Active Low
    OLED_SetColumnAddress_160128RGB(0, 159);
    OLED_SetRowAddress_160128RGB(0, 127);
}

void setup() 
{
  // setup globals
  ulReqcount=0; 
  ulReconncount=0; 

  // start serial
  Serial.begin(115200);
  delay(10);

  // prepare GPIO LED
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output

  // prepare DHT VDD                // Wärem durch Strom  reduzieren
  dht.begin();
  pinMode(DHTVDD, OUTPUT);
  digitalWrite(DHTVDD, HIGH);        // DHT /off

  // prepare GPIO for Display
  pinMode(RS_PIN, OUTPUT);                     // configure RS_PIN as output
  pinMode(RES_PIN, OUTPUT);                   // configure RES_PIN as output
  pinMode(CS_PIN, OUTPUT);                    // configure CS_PIN as output 
  digitalWrite(CS_PIN, HIGH);                 // set CS_PIN
  pinMode(SDI_PIN, OUTPUT);                   // configure SDI_PIN as output
  pinMode(SCL_PIN, OUTPUT);                   // configure SCL_PIN as output
  digitalWrite(SDI_PIN, LOW);                 // reset SDI_PIN
  digitalWrite(SCL_PIN, LOW);                 // reset SCL_PIN

  // inital connect
  espClient.start_WiFi_connections();
  
  // initial Sync Time
  getNTPTime(); 

  // initial FSBrowser Server in Port 81
  initFSBrowserServer();
  
  SPIFFS.begin();
  {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {    
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      DBG_OUTPUT_PORT.printf("FS File: %s, size: %s\n", fileName.c_str(), formatBytes(fileSize).c_str());
    }
    DBG_OUTPUT_PORT.printf("\n");
  }
}

// the loop function runs over and over again forever
void loop() 
{
  unsigned long now; 
  char charNewTemp[7] = "00.0*C";
  char charNewHum[7]  = "00.0%";
  char charDisplay[7]  = "00.0*C";
  float newHum = 0.0;
  float newTemp = 0.0;
  char charTime[5] ={0};
  char charDate[10] ={0};  
  int sensor_value=0;
  char str[1];
  char topDisplay[15];  
  char webUnit[3];
  int fadeInTime =10000;
  int fadeOutTime = 200;
  int delayTime =0;
    
    OLED_Init_160128RGB();                           // initialize display
    OLED_FillScreen_160128RGB(BLACK);                // fill screen with black
    OLED_FadeOut_160128RGB(0);
    while(1)                                          // wait here forever
    {
         
        espClient.handle_connections(); 
        if (!espClient.config_running)
        {
            FSBrowserServer.handleClient();
         
            // check if Clock sync intervall 
            now = millis();
            if ((now - previousMillisNTP >= intervalNTP) || (setNTP_OK == false))
           {
               // save the last time you read the sensor 
              previousMillisNTP = now;   
              // Sync Time
               getNTPTime();
           }
       
           OLED_FillArea_160128RGB(0, 160, 0, 128, BLACK); //clear screen

          // hh:mm
          strcpy(charTime, printDigits(hour()).c_str());
          strcat(charTime, ":");
          strcat(charTime, printDigits(minute()).c_str());
          OLED_StringSmallFont_160128RGB(80 - countPixel(charTime)/2, 38, charTime , BLUE, BLACK);   // 0

          // dd.mm.yyyy
          strcpy(charDate, printDigits(day()).c_str());
          strcat(charDate, ".");
          strcat(charDate, printDigits(month()).c_str());
          strcat(charDate, ".");
          strcat(charDate, printDigits(year()).c_str());
          OLED_StringSmallFont_160128RGB(80 - countPixel(charDate)/2, 18, charDate , BLUE, BLACK);   // 0

          // send temperature, humidity via MQTT
          now = millis();
          if (now - previousMillisMQTT > 10000) 
          {
            previousMillisMQTT = now;

            digitalWrite(DHTVDD, HIGH);                     // DHT on for read
            milli_delay(500);
            newTemp = dht.readTemperature(false);           // Read temperature if trus as Fahrenheit, if false as Celsius
            newHum = dht.readHumidity();                    // Read humidity (percent)
            digitalWrite(DHTVDD, LOW);                      // DHT off for cool down

            if (checkBound(newTemp, temp, diff)) {
              temp = newTemp;
              dtostrf(temp, 4, 1, charNewTemp);
              Serial.print("New temperature: ");
              Serial.println(String(temp));
              espClient.pub(2,1,0, charNewTemp);     // MQTT publish TopicTree struct -[2]Sensor--[1]temperature--[0]T1
              strcat(charNewTemp, "*C");
            }

            if (checkBound(newHum, hum, diff)) {
              hum = newHum;
              dtostrf(hum, 4, 1, charNewHum);
              Serial.print("New humidity: ");
              Serial.println(String(hum));
              espClient.pub(2,2,0, charNewHum);     // MQTT publish TopicTree struct -[2]Sensor--[2]humidity--[0]H1
              strcat(charNewHum, "%");
            }  
          }

// If Diving-Control is true
#ifdef LDC
         // Read analog port A0 
         sensor_value  = analogRead(SENSOR);
         sv = (sensor_value*100/1023);

         Serial.print("Analog A0: ");
         Serial.println(sv);
        
         // Set Driving to OLED
         OLED_Driving_Current_160128RGB(223 * sv/100, 197 * sv/100, 179 * sv/100);      //OLED set driving
#endif
         // Display Temerature
         strcpy(espClient.MyOLEDDisplay[4].Screen, charNewTemp); // copy for display on Website

         OLED_FillArea_160128RGB(0, 160, 100, 128, BLACK);
         OLED_StringSmallFont_160128RGB(80 - countPixel("Temperatur")/2 , 124, "Temperatur"  , BLUE, BLACK);   // 0
         OLED_FillArea_160128RGB(0, 160, 43, 95, BLACK);
         OLED_StringBigFont_160128RGB(0, 90, charNewTemp , YELLOW, BLACK);   // 0

         OLED_FadeIn_160128RGB(fadeInTime);
         digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on 
         milli_delay(atoi(espClient.cfg.webDurationScreen4)*1000);                      // Wait for a second
         OLED_FadeOut_160128RGB(fadeOutTime);

         // Display Humidity
         strcpy(espClient.MyOLEDDisplay[5].Screen, charNewHum); // copy for display on Website
        
         OLED_FillArea_160128RGB(0, 160, 100, 128, BLACK);
         OLED_StringSmallFont_160128RGB(80 - countPixel("Luftfeuchte")/2, 124, "Luftfeuchte" , BLUE, BLACK);   // 0
         OLED_FillArea_160128RGB(0, 160, 43, 95, BLACK);
         OLED_StringBigFont_160128RGB(3, 90, charNewHum, YELLOW, BLACK);   // 0
         OLED_FadeIn_160128RGB(fadeInTime);
         digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
         milli_delay(atoi(espClient.cfg.webDurationScreen5)*1000);                      // Wait for two seconds (to demonstrate the active low LED)
         OLED_FadeOut_160128RGB(fadeOutTime);

         for ( int i=0; i<4;i++) // 10, wenn alle 9 Zusatzdisplay ausgewertet werden sollen
         {
           if (strcmp(espClient.MyOLEDDisplay[i].Screen, "clear") !=0)
           {
             dtostrf(atof(espClient.MyOLEDDisplay[i].Screen), 4, 1, charDisplay);
             // Clear first line
             OLED_FillArea_160128RGB(0, 160, 100, 128, BLACK);
             // Display Toptitle
             switch (i) 
             {
               case 0:
                 strcpy(topDisplay,espClient.cfg.webNameScreen0);
                 strcpy(webUnit,espClient.cfg.webUnitScreen0);
                 delayTime = atoi(espClient.cfg.webDurationScreen0)*1000;
                 break;
               case 1:
                 strcpy(topDisplay,espClient.cfg.webNameScreen1);
                 strcpy(webUnit,espClient.cfg.webUnitScreen1);
                 delayTime = atoi(espClient.cfg.webDurationScreen1)*1000;
                 break;
             }
             strcat(charDisplay, webUnit);           // Einheit (*C, %) anhängen

             OLED_StringSmallFont_160128RGB(80 - countPixel(topDisplay)/2, 127, topDisplay , BLUE, BLACK);   // Toptitle             
             OLED_FillArea_160128RGB(0, 160, 43, 95, BLACK);
             OLED_StringBigFont_160128RGB(0, 90, charDisplay, YELLOW, BLACK);   // Value
             OLED_FadeIn_160128RGB(fadeInTime);
             milli_delay(delayTime);                      
             OLED_FadeOut_160128RGB(fadeOutTime);
          }
       }

       // Show Weather Bitmap
       OLED_FillArea_160128RGB(0, 160, 0, 128, BLACK);
       selectWeatherIcon(0);  // 0=today
       OLED_FadeIn_160128RGB(fadeInTime);
       milli_delay(5000);                      // Wait for a second
       OLED_FadeOut_160128RGB(fadeOutTime);
    } 
    else
    {
       Serial.println("Config mode activ! (loop())");
       OLED_FillArea_160128RGB(0, 160, 0, 128, BLACK);
       OLED_StringSmallFont_160128RGB(80 - countPixel("Config mode")/2, 102, "Config mode" , WHITE, BLACK);   // 0
    } // if config_runnig
    
  } //while(1) 
}
