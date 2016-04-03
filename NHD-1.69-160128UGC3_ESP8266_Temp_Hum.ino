
//---------------------------------------------------------
/*
 NHD-1.69-160128UGC3_ESP8266_Temp_Hum.ino
 V 0.11
 Program for writing to Newhaven Display's 160x128 Graphic Color OLED with SEPS525 controller.
 
 Pick one up n the Newhaven Display shop!
 ------> http://www.newhavendisplay.com/nhd169160128ugc3-p-5603.html
 
 This code is written for the Arduino Uno R3.
 
 Modifyed to ESP8266-12E NodeMCU - RL
 ------> https://github.com/wingfighter/ESP8266_OLED_NHD-1.69-160128UGC3.git  
 
 Copyright (c) 2015 - Newhaven Display International, LLC.
 Copyright (c) 2016 - wingfighter
 
 Newhaven Display invests time and resources providing this open source code,
 please support Newhaven Display by purchasing products from Newhaven Display!
*/


#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <DHT.h>
#include <Time.h>
#include <TimeLib.h>
#include <PubSubClient.h>

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

#define SENSOR A0         // ADC Pin

const char* wifi_ssid     = "...........";
const char* wifi_password = "..........."; 
unsigned long ulReqcount;
unsigned long ulReconncount;

//NTP
/* Don't hardwire the IP address or we won't get the benefits of the pool.
 *  Lookup the IP address for the host name instead */
IPAddress timeServerIP;                        // time.nist.gov NTP server address
const char* ntpServerName = "time.nist.gov";
unsigned int localPort = 2390;                // local port to listen for UDP packets
int gmt = 1;                                  // Zeitzone
const int NTP_PACKET_SIZE = 48;               // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE];          //buffer to hold incoming and outgoing packets
unsigned long previousMillisNTP = 0;          // will store last NTP was read
const long intervalNTP = 86400000;              // interval at which to sync Clock with NTP (86400000ms = 24h)

// A UDP instance to let us send and receive packets over UDP
WiFiUDP udp;  

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
const long interval = 2000;              // interval at which to read sensor

// MQTT-Client
#define mqtt_server "192.168.0.150"    //FHEM-Server, MQTT-Server (Mosquitto)
#define mqtt_user "mqtt_user"
#define mqtt_password "mqtt_pw"
#define humidity_topic "fhem/Sensor/DHT22/Luftfeuchte"
#define temperature_topic "fhem/Sensor/DHT22/Temperatur"
#define display_topic "fhem/Display"
#define BUFFER_SIZE 100
char Display[10][6] = {{"clear"},
                      {"clear"},
                      {"clear"},
                      {"clear"},
                      {"clear"},
                      {"clear"},
                      {"clear"},
                      {"clear"},
                      {"clear"},
                      {"clear"}};

unsigned long previousMillisMQTT = 0;        // will store last MQTT was send

// init MQTT-Client
WiFiClient MQTTClient;
PubSubClient mqttClient(mqtt_server, 1883, mqttCallback, MQTTClient);

/*********************************/
/******** FONT TABLE 5x8 *********/
/************* START *************/
/*********************************/

unsigned char Ascii_1[97][5] = {     // Refer to "Times New Roman" Font Database...
    //   Basic Characters
    {0x00,0x00,0x00,0x00,0x00},     //   (  0)    - 0x0000 Empty set
    {0x00,0x00,0x4F,0x00,0x00},     //   (  1)  ! - 0x0021 Exclamation Mark
    {0x00,0x07,0x00,0x07,0x00},     //   (  2)  " - 0x0022 Quotation Mark
    {0x14,0x7F,0x14,0x7F,0x14},     //   (  3)  # - 0x0023 Number Sign
    {0x24,0x2A,0x7F,0x2A,0x12},     //   (  4)  $ - 0x0024 Dollar Sign
    {0x23,0x13,0x08,0x64,0x62},     //   (  5)  % - 0x0025 Percent Sign
    {0x36,0x49,0x55,0x22,0x50},     //   (  6)  & - 0x0026 Ampersand
    {0x00,0x05,0x03,0x00,0x00},     //   (  7)  ' - 0x0027 Apostrophe
    {0x00,0x1C,0x22,0x41,0x00},     //   (  8)  ( - 0x0028 Left Parenthesis
    {0x00,0x41,0x22,0x1C,0x00},     //   (  9)  ) - 0x0029 Right Parenthesis
    {0x14,0x08,0x3E,0x08,0x14},     //   ( 10)  * - 0x002A Asterisk
    {0x08,0x08,0x3E,0x08,0x08},     //   ( 11)  + - 0x002B Plus Sign
    {0x00,0x50,0x30,0x00,0x00},     //   ( 12)  , - 0x002C Comma
    {0x08,0x08,0x08,0x08,0x08},     //   ( 13)  - - 0x002D Hyphen-Minus
    {0x00,0x60,0x60,0x00,0x00},     //   ( 14)  . - 0x002E Full Stop
    {0x20,0x10,0x08,0x04,0x02},     //   ( 15)  / - 0x002F Solidus
    {0x3E,0x51,0x49,0x45,0x3E},     //   ( 16)  0 - 0x0030 Digit Zero
    {0x00,0x42,0x7F,0x40,0x00},     //   ( 17)  1 - 0x0031 Digit One
    {0x42,0x61,0x51,0x49,0x46},     //   ( 18)  2 - 0x0032 Digit Two
    {0x21,0x41,0x45,0x4B,0x31},     //   ( 19)  3 - 0x0033 Digit Three
    {0x18,0x14,0x12,0x7F,0x10},     //   ( 20)  4 - 0x0034 Digit Four
    {0x27,0x45,0x45,0x45,0x39},     //   ( 21)  5 - 0x0035 Digit Five
    {0x3C,0x4A,0x49,0x49,0x30},     //   ( 22)  6 - 0x0036 Digit Six
    {0x01,0x71,0x09,0x05,0x03},     //   ( 23)  7 - 0x0037 Digit Seven
    {0x36,0x49,0x49,0x49,0x36},     //   ( 24)  8 - 0x0038 Digit Eight
    {0x06,0x49,0x49,0x29,0x1E},     //   ( 25)  9 - 0x0039 Dight Nine
    {0x00,0x36,0x36,0x00,0x00},     //   ( 26)  : - 0x003A Colon
    {0x00,0x56,0x36,0x00,0x00},     //   ( 27)  ; - 0x003B Semicolon
    {0x08,0x14,0x22,0x41,0x00},     //   ( 28)  < - 0x003C Less-Than Sign
    {0x14,0x14,0x14,0x14,0x14},     //   ( 29)  = - 0x003D Equals Sign
    {0x00,0x41,0x22,0x14,0x08},     //   ( 30)  > - 0x003E Greater-Than Sign
    {0x02,0x01,0x51,0x09,0x06},     //   ( 31)  ? - 0x003F Question Mark
    {0x32,0x49,0x79,0x41,0x3E},     //   ( 32)  @ - 0x0040 Commercial At
    {0x7E,0x11,0x11,0x11,0x7E},     //   ( 33)  A - 0x0041 Latin Capital Letter A
    {0x7F,0x49,0x49,0x49,0x36},     //   ( 34)  B - 0x0042 Latin Capital Letter B
    {0x3E,0x41,0x41,0x41,0x22},     //   ( 35)  C - 0x0043 Latin Capital Letter C
    {0x7F,0x41,0x41,0x22,0x1C},     //   ( 36)  D - 0x0044 Latin Capital Letter D
    {0x7F,0x49,0x49,0x49,0x41},     //   ( 37)  E - 0x0045 Latin Capital Letter E
    {0x7F,0x09,0x09,0x09,0x01},     //   ( 38)  F - 0x0046 Latin Capital Letter F
    {0x3E,0x41,0x49,0x49,0x7A},     //   ( 39)  G - 0x0047 Latin Capital Letter G
    {0x7F,0x08,0x08,0x08,0x7F},     //   ( 40)  H - 0x0048 Latin Capital Letter H
    {0x00,0x41,0x7F,0x41,0x00},     //   ( 41)  I - 0x0049 Latin Capital Letter I
    {0x20,0x40,0x41,0x3F,0x01},     //   ( 42)  J - 0x004A Latin Capital Letter J
    {0x7F,0x08,0x14,0x22,0x41},     //   ( 43)  K - 0x004B Latin Capital Letter K
    {0x7F,0x40,0x40,0x40,0x40},     //   ( 44)  L - 0x004C Latin Capital Letter L
    {0x7F,0x02,0x0C,0x02,0x7F},     //   ( 45)  M - 0x004D Latin Capital Letter M
    {0x7F,0x04,0x08,0x10,0x7F},     //   ( 46)  N - 0x004E Latin Capital Letter N
    {0x3E,0x41,0x41,0x41,0x3E},     //   ( 47)  O - 0x004F Latin Capital Letter O
    {0x7F,0x09,0x09,0x09,0x06},     //   ( 48)  P - 0x0050 Latin Capital Letter P
    {0x3E,0x41,0x51,0x21,0x5E},     //   ( 49)  Q - 0x0051 Latin Capital Letter Q
    {0x7F,0x09,0x19,0x29,0x46},     //   ( 50)  R - 0x0052 Latin Capital Letter R
    {0x46,0x49,0x49,0x49,0x31},     //   ( 51)  S - 0x0053 Latin Capital Letter S
    {0x01,0x01,0x7F,0x01,0x01},     //   ( 52)  T - 0x0054 Latin Capital Letter T
    {0x3F,0x40,0x40,0x40,0x3F},     //   ( 53)  U - 0x0055 Latin Capital Letter U
    {0x1F,0x20,0x40,0x20,0x1F},     //   ( 54)  V - 0x0056 Latin Capital Letter V
    {0x3F,0x40,0x38,0x40,0x3F},     //   ( 55)  W - 0x0057 Latin Capital Letter W
    {0x63,0x14,0x08,0x14,0x63},     //   ( 56)  X - 0x0058 Latin Capital Letter X
    {0x07,0x08,0x70,0x08,0x07},     //   ( 57)  Y - 0x0059 Latin Capital Letter Y
    {0x61,0x51,0x49,0x45,0x43},     //   ( 58)  Z - 0x005A Latin Capital Letter Z
    {0x00,0x7F,0x41,0x41,0x00},     //   ( 59)  [ - 0x005B Left Square Bracket
    {0x02,0x04,0x08,0x10,0x20},     //   ( 60)  \ - 0x005C Reverse Solidus
    {0x00,0x41,0x41,0x7F,0x00},     //   ( 61)  ] - 0x005D Right Square Bracket
    {0x04,0x02,0x01,0x02,0x04},     //   ( 62)  ^ - 0x005E Circumflex Accent
    {0x40,0x40,0x40,0x40,0x40},     //   ( 63)  _ - 0x005F Low Line
    {0x01,0x02,0x04,0x00,0x00},     //   ( 64)  ` - 0x0060 Grave Accent
    {0x20,0x54,0x54,0x54,0x78},     //   ( 65)  a - 0x0061 Latin Small Letter A
    {0x7F,0x48,0x44,0x44,0x38},     //   ( 66)  b - 0x0062 Latin Small Letter B
    {0x38,0x44,0x44,0x44,0x20},     //   ( 67)  c - 0x0063 Latin Small Letter C
    {0x38,0x44,0x44,0x48,0x7F},     //   ( 68)  d - 0x0064 Latin Small Letter D
    {0x38,0x54,0x54,0x54,0x18},     //   ( 69)  e - 0x0065 Latin Small Letter E
    {0x08,0x7E,0x09,0x01,0x02},     //   ( 70)  f - 0x0066 Latin Small Letter F
    {0x06,0x49,0x49,0x49,0x3F},     //   ( 71)  g - 0x0067 Latin Small Letter G
    {0x7F,0x08,0x04,0x04,0x78},     //   ( 72)  h - 0x0068 Latin Small Letter H
    {0x00,0x44,0x7D,0x40,0x00},     //   ( 73)  i - 0x0069 Latin Small Letter I
    {0x20,0x40,0x44,0x3D,0x00},     //   ( 74)  j - 0x006A Latin Small Letter J
    {0x7F,0x10,0x28,0x44,0x00},     //   ( 75)  k - 0x006B Latin Small Letter K
    {0x00,0x41,0x7F,0x40,0x00},     //   ( 76)  l - 0x006C Latin Small Letter L
    {0x7C,0x04,0x18,0x04,0x7C},     //   ( 77)  m - 0x006D Latin Small Letter M
    {0x7C,0x08,0x04,0x04,0x78},     //   ( 78)  n - 0x006E Latin Small Letter N
    {0x38,0x44,0x44,0x44,0x38},     //   ( 79)  o - 0x006F Latin Small Letter O
    {0x7C,0x14,0x14,0x14,0x08},     //   ( 80)  p - 0x0070 Latin Small Letter P
    {0x08,0x14,0x14,0x18,0x7C},     //   ( 81)  q - 0x0071 Latin Small Letter Q
    {0x7C,0x08,0x04,0x04,0x08},     //   ( 82)  r - 0x0072 Latin Small Letter R
    {0x48,0x54,0x54,0x54,0x20},     //   ( 83)  s - 0x0073 Latin Small Letter S
    {0x04,0x3F,0x44,0x40,0x20},     //   ( 84)  t - 0x0074 Latin Small Letter T
    {0x3C,0x40,0x40,0x20,0x7C},     //   ( 85)  u - 0x0075 Latin Small Letter U
    {0x1C,0x20,0x40,0x20,0x1C},     //   ( 86)  v - 0x0076 Latin Small Letter V
    {0x3C,0x40,0x30,0x40,0x3C},     //   ( 87)  w - 0x0077 Latin Small Letter W
    {0x44,0x28,0x10,0x28,0x44},     //   ( 88)  x - 0x0078 Latin Small Letter X
    {0x0C,0x50,0x50,0x50,0x3C},     //   ( 89)  y - 0x0079 Latin Small Letter Y
    {0x44,0x64,0x54,0x4C,0x44},     //   ( 90)  z - 0x007A Latin Small Letter Z
    {0x00,0x08,0x36,0x41,0x00},     //   ( 91)  { - 0x007B Left Curly Bracket
    {0x00,0x00,0x7F,0x00,0x00},     //   ( 92)  | - 0x007C Vertical Line
    {0x00,0x41,0x36,0x08,0x00},     //   ( 93)  } - 0x007D Right Curly Bracket
    {0x02,0x01,0x02,0x04,0x02},     //   ( 94)  ~ - 0x007E Tilde
    {0x08,0x0C,0x0E,0x0C,0x08},     //   ( 95)  upward facing triangle/arrow
    {0x08,0x18,0x38,0x18,0x08},     //   ( 96)  downward facing triangle/arrow
};

/*===============================*/
/*======= FONT TABLE 5x8 ========*/
/*============= END =============*/
/*===============================*/


/*********************************/
/***** CHARACTERS BITMAP *********/
/************* START *************/
/*********************************/

// 27 Pixel breit/ 34 Pixel hoch
unsigned char Verdana[][148] = {
  {
    0x00, 0x7f, 0xc0, 0x00, //0
    0x03, 0xff, 0xf8, 0x00, 
    0x07, 0xff, 0xfc, 0x00, 
    0x0f, 0xff, 0xfe, 0x00, 
    0x1f, 0xff, 0xff, 0x00, 
    0x3f, 0xff, 0xff, 0x80, 
    0x3f, 0xe0, 0xff, 0x80, 
    0x7f, 0xc0, 0x7f, 0xc0, 
    0x7f, 0x80, 0x3f, 0xc0, 
    0x7f, 0x80, 0x3f, 0xc0, 
    0x7f, 0x80, 0x3f, 0xc0, 
    0xff, 0x00, 0x1f, 0xe0, 
    0xff, 0x00, 0x1f, 0xe0, 
    0xff, 0x00, 0x1f, 0xe0, 
    0xff, 0x00, 0x1f, 0xe0, 
    0xff, 0x00, 0x1f, 0xe0, 
    0xff, 0x00, 0x1f, 0xe0, 
    0xff, 0x00, 0x1f, 0xe0, 
    0xff, 0x00, 0x1f, 0xe0, 
    0xff, 0x00, 0x1f, 0xe0, 
    0xff, 0x00, 0x1f, 0xe0, 
    0xff, 0x00, 0x1f, 0xe0, 
    0xff, 0x00, 0x1f, 0xe0, 
    0x7f, 0x80, 0x3f, 0xc0, 
    0x7f, 0x80, 0x3f, 0xc0, 
    0x7f, 0x80, 0x3f, 0xc0, 
    0x7f, 0xc0, 0x7f, 0xc0, 
    0x3f, 0xe0, 0xff, 0x80, 
    0x3f, 0xff, 0xff, 0x80, 
    0x1f, 0xff, 0xff, 0x00, 
    0x0f, 0xff, 0xfe, 0x00, 
    0x07, 0xff, 0xfc, 0x00, 
    0x03, 0xff, 0xf8, 0x00, 
    0x00, 0x7f, 0xc0, 0x00
  },
  {
    0x00, 0x0f, 0xc0, 0x00, //1
    0x00, 0x1f, 0xc0, 0x00, 
    0x00, 0x1f, 0xc0, 0x00, 
    0x00, 0x3f, 0xc0, 0x00, 
    0x00, 0xff, 0xc0, 0x00, 
    0x1f, 0xff, 0xc0, 0x00, 
    0x1f, 0xff, 0xc0, 0x00, 
    0x1f, 0xff, 0xc0, 0x00, 
    0x1f, 0xff, 0xc0, 0x00, 
    0x1f, 0xff, 0xc0, 0x00, 
    0x00, 0x3f, 0xc0, 0x00, 
    0x00, 0x3f, 0xc0, 0x00, 
    0x00, 0x3f, 0xc0, 0x00, 
    0x00, 0x3f, 0xc0, 0x00, 
    0x00, 0x3f, 0xc0, 0x00, 
    0x00, 0x3f, 0xc0, 0x00, 
    0x00, 0x3f, 0xc0, 0x00, 
    0x00, 0x3f, 0xc0, 0x00, 
    0x00, 0x3f, 0xc0, 0x00, 
    0x00, 0x3f, 0xc0, 0x00, 
    0x00, 0x3f, 0xc0, 0x00, 
    0x00, 0x3f, 0xc0, 0x00, 
    0x00, 0x3f, 0xc0, 0x00, 
    0x00, 0x3f, 0xc0, 0x00, 
    0x00, 0x3f, 0xc0, 0x00, 
    0x00, 0x3f, 0xc0, 0x00, 
    0x00, 0x3f, 0xc0, 0x00, 
    0x00, 0x3f, 0xc0, 0x00, 
    0x1f, 0xff, 0xff, 0x80, 
    0x1f, 0xff, 0xff, 0x80, 
    0x1f, 0xff, 0xff, 0x80, 
    0x1f, 0xff, 0xff, 0x80, 
    0x1f, 0xff, 0xff, 0x80, 
    0x1f, 0xff, 0xff, 0x80
  },
  { 
    0x00, 0xff, 0xc0, 0x00, //2 
    0x0f, 0xff, 0xf0, 0x00, 
    0x1f, 0xff, 0xfc, 0x00, 
    0x3f, 0xff, 0xfe, 0x00, 
    0x3f, 0xff, 0xff, 0x00, 
    0x3f, 0xff, 0xff, 0x00, 
    0x3f, 0x83, 0xff, 0x80, 
    0x3c, 0x00, 0xff, 0x80, 
    0x30, 0x00, 0x7f, 0x80, 
    0x20, 0x00, 0x7f, 0x80, 
    0x00, 0x00, 0x7f, 0x80, 
    0x00, 0x00, 0x7f, 0x80, 
    0x00, 0x00, 0x7f, 0x80, 
    0x00, 0x00, 0xff, 0x00, 
    0x00, 0x00, 0xff, 0x00, 
    0x00, 0x01, 0xff, 0x00, 
    0x00, 0x03, 0xfe, 0x00, 
    0x00, 0x07, 0xfc, 0x00, 
    0x00, 0x0f, 0xfc, 0x00, 
    0x00, 0x1f, 0xf8, 0x00, 
    0x00, 0x3f, 0xf0, 0x00, 
    0x00, 0x7f, 0xe0, 0x00, 
    0x00, 0xff, 0xc0, 0x00, 
    0x01, 0xff, 0x80, 0x00, 
    0x03, 0xff, 0x00, 0x00, 
    0x07, 0xfe, 0x00, 0x00, 
    0x0f, 0xfc, 0x00, 0x00, 
    0x1f, 0xf8, 0x00, 0x00, 
    0x3f, 0xff, 0xff, 0xc0, 
    0x3f, 0xff, 0xff, 0xc0, 
    0x3f, 0xff, 0xff, 0xc0, 
    0x3f, 0xff, 0xff, 0xc0, 
    0x3f, 0xff, 0xff, 0xc0, 
    0x3f, 0xff, 0xff, 0xc0
  },
  {
    0x01, 0xff, 0xc0, 0x00, //3
    0x1f, 0xff, 0xf8, 0x00, 
    0x3f, 0xff, 0xfc, 0x00, 
    0x3f, 0xff, 0xfe, 0x00, 
    0x3f, 0xff, 0xff, 0x00, 
    0x3f, 0xff, 0xff, 0x80, 
    0x3f, 0x01, 0xff, 0x80, 
    0x38, 0x00, 0xff, 0x80, 
    0x30, 0x00, 0x7f, 0x80, 
    0x00, 0x00, 0x7f, 0x80, 
    0x00, 0x00, 0x7f, 0x80, 
    0x00, 0x00, 0xff, 0x00, 
    0x00, 0x01, 0xff, 0x00, 
    0x00, 0xff, 0xfe, 0x00, 
    0x00, 0xff, 0xfc, 0x00, 
    0x00, 0xff, 0xe0, 0x00, 
    0x00, 0xff, 0xfc, 0x00, 
    0x00, 0xff, 0xfe, 0x00, 
    0x00, 0xff, 0xff, 0x00, 
    0x00, 0x00, 0xff, 0x80, 
    0x00, 0x00, 0x7f, 0xc0, 
    0x00, 0x00, 0x3f, 0xc0, 
    0x00, 0x00, 0x3f, 0xc0, 
    0x00, 0x00, 0x3f, 0xc0, 
    0x00, 0x00, 0x3f, 0xc0, 
    0x60, 0x00, 0x7f, 0xc0, 
    0x78, 0x00, 0x7f, 0xc0, 
    0x7f, 0x01, 0xff, 0x80, 
    0x7f, 0xff, 0xff, 0x80, 
    0x7f, 0xff, 0xff, 0x00, 
    0x7f, 0xff, 0xfe, 0x00, 
    0x7f, 0xff, 0xfc, 0x00, 
    0x3f, 0xff, 0xf0, 0x00, 
    0x03, 0xff, 0x80, 0x00
  },
  {
    0x00, 0x01, 0xfe, 0x00, //4 
    0x00, 0x03, 0xfe, 0x00, 
    0x00, 0x07, 0xfe, 0x00, 
    0x00, 0x07, 0xfe, 0x00, 
    0x00, 0x0f, 0xfe, 0x00, 
    0x00, 0x1f, 0xfe, 0x00, 
    0x00, 0x3f, 0xfe, 0x00, 
    0x00, 0x7f, 0xfe, 0x00, 
    0x00, 0x7f, 0xfe, 0x00, 
    0x00, 0xfd, 0xfe, 0x00, 
    0x01, 0xf9, 0xfe, 0x00, 
    0x03, 0xf9, 0xfe, 0x00, 
    0x03, 0xf1, 0xfe, 0x00, 
    0x07, 0xe1, 0xfe, 0x00, 
    0x0f, 0xc1, 0xfe, 0x00, 
    0x1f, 0xc1, 0xfe, 0x00, 
    0x1f, 0x81, 0xfe, 0x00, 
    0x3f, 0x01, 0xfe, 0x00, 
    0x7e, 0x01, 0xfe, 0x00, 
    0xfe, 0x01, 0xfe, 0x00, 
    0xff, 0xff, 0xff, 0xe0, 
    0xff, 0xff, 0xff, 0xe0, 
    0xff, 0xff, 0xff, 0xe0, 
    0xff, 0xff, 0xff, 0xe0, 
    0xff, 0xff, 0xff, 0xe0, 
    0xff, 0xff, 0xff, 0xe0, 
    0x00, 0x01, 0xfe, 0x00, 
    0x00, 0x01, 0xfe, 0x00, 
    0x00, 0x01, 0xfe, 0x00, 
    0x00, 0x01, 0xfe, 0x00, 
    0x00, 0x01, 0xfe, 0x00, 
    0x00, 0x01, 0xfe, 0x00, 
    0x00, 0x01, 0xfe, 0x00, 
    0x00, 0x01, 0xfe, 0x00
  },
  {
    0x1f, 0xff, 0xff, 0x80, //5 
    0x1f, 0xff, 0xff, 0x80, 
    0x1f, 0xff, 0xff, 0x80, 
    0x1f, 0xff, 0xff, 0x80, 
    0x1f, 0xff, 0xff, 0x80, 
    0x1f, 0xff, 0xff, 0x80, 
    0x1f, 0xe0, 0x00, 0x00, 
    0x1f, 0xe0, 0x00, 0x00, 
    0x1f, 0xe0, 0x00, 0x00, 
    0x1f, 0xe0, 0x00, 0x00, 
    0x1f, 0xe0, 0x00, 0x00, 
    0x1f, 0xe0, 0x00, 0x00, 
    0x1f, 0xff, 0xe0, 0x00, 
    0x1f, 0xff, 0xf8, 0x00, 
    0x1f, 0xff, 0xfe, 0x00, 
    0x1f, 0xff, 0xff, 0x00, 
    0x1f, 0xff, 0xff, 0x80, 
    0x1f, 0xff, 0xff, 0x80, 
    0x1c, 0x01, 0xff, 0xc0, 
    0x00, 0x00, 0x7f, 0xc0, 
    0x00, 0x00, 0x7f, 0xc0, 
    0x00, 0x00, 0x3f, 0xc0, 
    0x00, 0x00, 0x3f, 0xc0, 
    0x00, 0x00, 0x3f, 0xc0, 
    0x00, 0x00, 0x3f, 0xc0, 
    0x30, 0x00, 0x7f, 0xc0, 
    0x38, 0x00, 0x7f, 0x80, 
    0x3f, 0x01, 0xff, 0x80, 
    0x3f, 0xff, 0xff, 0x00, 
    0x3f, 0xff, 0xff, 0x00, 
    0x3f, 0xff, 0xfe, 0x00, 
    0x3f, 0xff, 0xf8, 0x00, 
    0x1f, 0xff, 0xf0, 0x00, 
    0x01, 0xff, 0x80, 0x00
  },
  {
    0x00, 0x07, 0xff, 0x00, //6 
    0x00, 0x3f, 0xff, 0x00, 
    0x00, 0xff, 0xff, 0x00, 
    0x01, 0xff, 0xff, 0x00, 
    0x03, 0xff, 0xff, 0x00, 
    0x07, 0xff, 0xff, 0x00, 
    0x0f, 0xfe, 0x03, 0x00, 
    0x1f, 0xf8, 0x00, 0x00, 
    0x1f, 0xe0, 0x00, 0x00, 
    0x3f, 0xe0, 0x00, 0x00, 
    0x3f, 0xc0, 0x00, 0x00, 
    0x3f, 0xc0, 0x00, 0x00, 
    0x3f, 0x87, 0xf0, 0x00, 
    0x7f, 0xbf, 0xfe, 0x00, 
    0x7f, 0xff, 0xff, 0x00, 
    0x7f, 0xff, 0xff, 0x80, 
    0x7f, 0xff, 0xff, 0xc0, 
    0x7f, 0xff, 0xff, 0xc0, 
    0x7f, 0xc0, 0xff, 0xc0, 
    0x7f, 0x80, 0x3f, 0xe0, 
    0x7f, 0x80, 0x1f, 0xe0, 
    0x7f, 0x80, 0x1f, 0xe0, 
    0x7f, 0x80, 0x1f, 0xe0, 
    0x7f, 0x80, 0x1f, 0xe0, 
    0x3f, 0xc0, 0x1f, 0xe0, 
    0x3f, 0xc0, 0x3f, 0xe0, 
    0x3f, 0xe0, 0x3f, 0xc0, 
    0x1f, 0xf0, 0xff, 0xc0, 
    0x1f, 0xff, 0xff, 0x80, 
    0x0f, 0xff, 0xff, 0x80, 
    0x07, 0xff, 0xff, 0x00, 
    0x03, 0xff, 0xfe, 0x00, 
    0x01, 0xff, 0xf8, 0x00, 
    0x00, 0x3f, 0xe0, 0x00
  },
  {
    0x7f, 0xff, 0xff, 0xc0, //7 
    0x7f, 0xff, 0xff, 0xc0, 
    0x7f, 0xff, 0xff, 0xc0, 
    0x7f, 0xff, 0xff, 0xc0, 
    0x7f, 0xff, 0xff, 0xc0, 
    0x7f, 0xff, 0xff, 0xc0, 
    0x00, 0x00, 0x3f, 0xc0, 
    0x00, 0x00, 0x7f, 0xc0, 
    0x00, 0x00, 0xff, 0x80, 
    0x00, 0x00, 0xff, 0x80, 
    0x00, 0x01, 0xff, 0x00, 
    0x00, 0x01, 0xff, 0x00, 
    0x00, 0x03, 0xfe, 0x00, 
    0x00, 0x07, 0xfc, 0x00, 
    0x00, 0x07, 0xfc, 0x00, 
    0x00, 0x0f, 0xf8, 0x00, 
    0x00, 0x0f, 0xf8, 0x00, 
    0x00, 0x1f, 0xf0, 0x00, 
    0x00, 0x1f, 0xf0, 0x00, 
    0x00, 0x3f, 0xe0, 0x00, 
    0x00, 0x7f, 0xc0, 0x00, 
    0x00, 0x7f, 0xc0, 0x00, 
    0x00, 0xff, 0x80, 0x00, 
    0x00, 0xff, 0x80, 0x00, 
    0x01, 0xff, 0x00, 0x00, 
    0x03, 0xff, 0x00, 0x00, 
    0x03, 0xfe, 0x00, 0x00, 
    0x07, 0xfc, 0x00, 0x00, 
    0x07, 0xfc, 0x00, 0x00, 
    0x0f, 0xf8, 0x00, 0x00, 
    0x1f, 0xf8, 0x00, 0x00, 
    0x1f, 0xf0, 0x00, 0x00, 
    0x3f, 0xf0, 0x00, 0x00, 
    0x7f, 0xe0, 0x00, 0x00
  },
  {
    0x00, 0x7f, 0xc0, 0x00, //8 
    0x03, 0xff, 0xf8, 0x00, 
    0x0f, 0xff, 0xfe, 0x00, 
    0x1f, 0xff, 0xff, 0x00, 
    0x3f, 0xff, 0xff, 0x80, 
    0x3f, 0xff, 0xff, 0x80, 
    0x7f, 0xe0, 0xff, 0xc0, 
    0x7f, 0xc0, 0x7f, 0xc0, 
    0x7f, 0x80, 0x3f, 0xc0, 
    0x7f, 0x80, 0x3f, 0xc0, 
    0x7f, 0x80, 0x3f, 0xc0, 
    0x7f, 0xe0, 0x3f, 0x80, 
    0x3f, 0xf8, 0x7f, 0x80, 
    0x3f, 0xff, 0xff, 0x00, 
    0x1f, 0xff, 0xfc, 0x00, 
    0x0f, 0xff, 0xf0, 0x00, 
    0x03, 0xff, 0xfc, 0x00, 
    0x07, 0xff, 0xff, 0x00, 
    0x1f, 0xff, 0xff, 0x80, 
    0x3f, 0xc7, 0xff, 0xc0, 
    0x7f, 0x80, 0xff, 0xc0, 
    0x7f, 0x00, 0x3f, 0xe0, 
    0xff, 0x00, 0x1f, 0xe0, 
    0xff, 0x00, 0x1f, 0xe0, 
    0xff, 0x00, 0x1f, 0xe0, 
    0xff, 0x80, 0x1f, 0xe0, 
    0xff, 0x80, 0x3f, 0xe0, 
    0x7f, 0xe0, 0x7f, 0xc0, 
    0x7f, 0xff, 0xff, 0xc0, 
    0x3f, 0xff, 0xff, 0x80, 
    0x3f, 0xff, 0xff, 0x00, 
    0x0f, 0xff, 0xfe, 0x00, 
    0x07, 0xff, 0xf8, 0x00, 
    0x00, 0xff, 0xc0, 0x00
  },
  {
    0x00, 0x3f, 0xc0, 0x00, //9 
    0x01, 0xff, 0xf8, 0x00, 
    0x07, 0xff, 0xfc, 0x00, 
    0x0f, 0xff, 0xfe, 0x00, 
    0x1f, 0xff, 0xff, 0x00, 
    0x1f, 0xff, 0xff, 0x80, 
    0x3f, 0xf0, 0xff, 0x80, 
    0x3f, 0xc0, 0x7f, 0xc0, 
    0x7f, 0xc0, 0x3f, 0xc0, 
    0x7f, 0x80, 0x3f, 0xc0, 
    0x7f, 0x80, 0x1f, 0xe0, 
    0x7f, 0x80, 0x1f, 0xe0, 
    0x7f, 0x80, 0x1f, 0xe0, 
    0x7f, 0x80, 0x1f, 0xe0, 
    0x7f, 0xc0, 0x1f, 0xe0, 
    0x3f, 0xf0, 0x3f, 0xe0, 
    0x3f, 0xff, 0xff, 0xe0, 
    0x3f, 0xff, 0xff, 0xe0, 
    0x1f, 0xff, 0xff, 0xe0, 
    0x0f, 0xff, 0xff, 0xe0, 
    0x07, 0xff, 0xdf, 0xe0, 
    0x00, 0xfe, 0x1f, 0xc0, 
    0x00, 0x00, 0x3f, 0xc0, 
    0x00, 0x00, 0x3f, 0xc0, 
    0x00, 0x00, 0x7f, 0x80, 
    0x00, 0x00, 0xff, 0x80, 
    0x00, 0x01, 0xff, 0x00, 
    0x0c, 0x07, 0xff, 0x00, 
    0x0f, 0xff, 0xfe, 0x00, 
    0x0f, 0xff, 0xfc, 0x00, 
    0x0f, 0xff, 0xf8, 0x00, 
    0x0f, 0xff, 0xf0, 0x00, 
    0x0f, 0xff, 0xc0, 0x00, 
    0x0f, 0xfe, 0x00, 0x00
  },
  {
    0x00, 0x0f, 0xfc, 0x00, //C 
    0x00, 0x7f, 0xff, 0xc0, 
    0x01, 0xff, 0xff, 0xe0, 
    0x03, 0xff, 0xff, 0xe0, 
    0x07, 0xff, 0xff, 0xe0, 
    0x0f, 0xff, 0xff, 0xe0, 
    0x1f, 0xfc, 0x07, 0xe0, 
    0x3f, 0xf0, 0x01, 0xe0, 
    0x3f, 0xe0, 0x00, 0x60, 
    0x7f, 0xc0, 0x00, 0x20, 
    0x7f, 0x80, 0x00, 0x00, 
    0x7f, 0x80, 0x00, 0x00, 
    0xff, 0x80, 0x00, 0x00, 
    0xff, 0x00, 0x00, 0x00, 
    0xff, 0x00, 0x00, 0x00, 
    0xff, 0x00, 0x00, 0x00, 
    0xff, 0x00, 0x00, 0x00, 
    0xff, 0x00, 0x00, 0x00, 
    0xff, 0x00, 0x00, 0x00, 
    0xff, 0x00, 0x00, 0x00, 
    0xff, 0x00, 0x00, 0x00, 
    0xff, 0x80, 0x00, 0x00, 
    0x7f, 0x80, 0x00, 0x00, 
    0x7f, 0x80, 0x00, 0x00, 
    0x7f, 0xc0, 0x00, 0x20, 
    0x3f, 0xe0, 0x00, 0x60, 
    0x3f, 0xf0, 0x01, 0xe0, 
    0x1f, 0xfc, 0x07, 0xe0, 
    0x0f, 0xff, 0xff, 0xe0, 
    0x0f, 0xff, 0xff, 0xe0, 
    0x07, 0xff, 0xff, 0xe0, 
    0x01, 0xff, 0xff, 0xe0, 
    0x00, 0x7f, 0xff, 0x80, 
    0x00, 0x0f, 0xfc, 0x00
  },
  {
    0x00, 0x1f, 0x80, 0x00, //° 
    0x00, 0x7f, 0xe0, 0x00, 
    0x00, 0xff, 0xf0, 0x00, 
    0x01, 0xff, 0xf8, 0x00, 
    0x03, 0xff, 0xfc, 0x00, 
    0x03, 0xf0, 0xfc, 0x00, 
    0x07, 0xe0, 0x7e, 0x00, 
    0x07, 0xc0, 0x3e, 0x00, 
    0x07, 0xc0, 0x3e, 0x00, 
    0x07, 0xc0, 0x3e, 0x00, 
    0x07, 0xc0, 0x3e, 0x00, 
    0x07, 0xe0, 0x7e, 0x00, 
    0x03, 0xf0, 0xfc, 0x00, 
    0x03, 0xff, 0xfc, 0x00, 
    0x01, 0xff, 0xf8, 0x00, 
    0x00, 0xff, 0xf0, 0x00, 
    0x00, 0x7f, 0xe0, 0x00, 
    0x00, 0x1f, 0x80, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x00, 0x00
  },
  {
    0x00, 0x1f, 0xc0, 0x00, //left % 
    0x00, 0x7f, 0xf0, 0x00, 
    0x00, 0xff, 0xfc, 0x00, 
    0x01, 0xff, 0xfc, 0x00, 
    0x03, 0xff, 0xfe, 0x00, 
    0x03, 0xf0, 0x7e, 0x00, 
    0x07, 0xf0, 0x7f, 0x00, 
    0x07, 0xe0, 0x3f, 0x00, 
    0x07, 0xe0, 0x3f, 0x00, 
    0x07, 0xe0, 0x3f, 0x00, 
    0x07, 0xe0, 0x3f, 0x00, 
    0x07, 0xe0, 0x3f, 0x00, 
    0x07, 0xe0, 0x3f, 0x00, 
    0x07, 0xe0, 0x3f, 0x20, 
    0x07, 0xf0, 0x7f, 0x20, 
    0x03, 0xf0, 0x7e, 0x60, 
    0x03, 0xff, 0xfe, 0x60, 
    0x01, 0xff, 0xfc, 0xe0, 
    0x00, 0xff, 0xfd, 0xe0, 
    0x00, 0x7f, 0xf1, 0xe0, 
    0x00, 0x1f, 0xc3, 0xe0, 
    0x00, 0x00, 0x03, 0xe0, 
    0x00, 0x00, 0x07, 0xe0, 
    0x00, 0x00, 0x07, 0xc0, 
    0x00, 0x00, 0x0f, 0xc0, 
    0x00, 0x00, 0x0f, 0x80, 
    0x00, 0x00, 0x1f, 0x80, 
    0x00, 0x00, 0x1f, 0x00, 
    0x00, 0x00, 0x3f, 0x00, 
    0x00, 0x00, 0x3e, 0x00, 
    0x00, 0x00, 0x7e, 0x00, 
    0x00, 0x00, 0x7c, 0x00, 
    0x00, 0x00, 0xfc, 0x00, 
    0x00, 0x01, 0xf8, 0x00
  },
  {
    0x03, 0xf0, 0x00, 0x00, // right % 
    0x07, 0xe0, 0x00, 0x00, 
    0x07, 0xc0, 0x00, 0x00, 
    0x0f, 0xc0, 0x00, 0x00, 
    0x0f, 0x80, 0x00, 0x00, 
    0x1f, 0x80, 0x00, 0x00, 
    0x1f, 0x00, 0x00, 0x00, 
    0x3f, 0x00, 0x00, 0x00, 
    0x3e, 0x00, 0x00, 0x00, 
    0x7e, 0x00, 0x00, 0x00, 
    0x7c, 0x00, 0x00, 0x00, 
    0xfc, 0x00, 0x00, 0x00, 
    0xf8, 0x00, 0x00, 0x00, 
    0xf8, 0x7f, 0x00, 0x00, 
    0xf1, 0xff, 0xc0, 0x00, 
    0xf3, 0xff, 0xf0, 0x00, 
    0xe7, 0xff, 0xf0, 0x00, 
    0xcf, 0xff, 0xf8, 0x00, 
    0xcf, 0xc1, 0xf8, 0x00, 
    0x9f, 0xc1, 0xfc, 0x00, 
    0x9f, 0x80, 0xfc, 0x00, 
    0x1f, 0x80, 0xfc, 0x00, 
    0x1f, 0x80, 0xfc, 0x00, 
    0x1f, 0x80, 0xfc, 0x00, 
    0x1f, 0x80, 0xfc, 0x00, 
    0x1f, 0x80, 0xfc, 0x00, 
    0x1f, 0x80, 0xfc, 0x00, 
    0x1f, 0xc1, 0xfc, 0x00, 
    0x0f, 0xc1, 0xf8, 0x00, 
    0x0f, 0xff, 0xf8, 0x00, 
    0x07, 0xff, 0xf0, 0x00, 
    0x03, 0xff, 0xf0, 0x00, 
    0x01, 0xff, 0xc0, 0x00, 
    0x00, 0x7f, 0x00, 0x00
  },
  {
    0x00, 0x00, //. 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x00, 0x00, 
    0x0f, 0xf0, 
    0x0f, 0xf0, 
    0x0f, 0xf0, 
    0x0f, 0xf0, 
    0x0f, 0xf0, 
    0x0f, 0xf0, 
    0x0f, 0xf0, 
    0x0f, 0xf0
  }
};

/*===============================*/
/*====== CHARACTERS BITMAP ======*/
/*============= END =============*/
/*===============================*/


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
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
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
  //delay(1000);
  // max. 10s auf Antwort vom Zeitserver warten
  long now = millis();
  while ((!cb) && ((millis()-now) < 10000))
    cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no packet yet");
  }
  else {
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
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

/*===============================*/
/*============ WiFi =============*/
/*=========== BEGIN =============*/
/*===============================*/

void WiFiStart()
{
  ulReconncount++;
  
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);
  
  WiFi.begin(wifi_ssid, wifi_password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Print the IP address
  Serial.println(WiFi.localIP());

  // UDP-Connection for NTP-Sync
  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());
    
}
  
/*===============================*/
/*============ WiFi =============*/
/*============= END =============*/
/*===============================*/


/*===============================*/
/*============ MQTT =============*/
/*=========== BEGIN =============*/
/*===============================*/



// ===========================================================
// Callback Funktion von MQTT. Die Funktion wird aufgerufen
// wenn ein Wert empfangen wurde.
// ===========================================================
void mqttCallback(char* topic, byte* payload, unsigned int length) 
{
  int i = 0;
  char *ptr;
  char message_buff[BUFFER_SIZE];
 
   Serial.println("Message arrived: topic: " + String(topic));
   Serial.println("Length: " + String(length,DEC));
 
   // Kopieren der Nachricht und erstellen eines Bytes mit abschließender \0
   for(i=0; i<length; i++) 
   {
     message_buff[i] = payload[i];
   }
   message_buff[i] = '\0';
 
  // Konvertierung der nachricht in ein String
  String msgString = String(message_buff);
  Serial.println("Payload: " + msgString);
  Serial.print("Payload message_buff[]: ");
  Serial.println(message_buff);

  // Test auf Topic display_topic
  if (strncmp(topic, display_topic, strlen(display_topic))==0) 
  {
    ptr = strrchr(topic, '/');
    ptr++;
    strcpy(Display[(int)*ptr-'0'], "     ");

    // Anhang an display_topic (z.B. 1) gibt Display an, auf dem der Werte angezeigt werden soll
    // und darf nicht außerhaln 0...9 liegen
    if ( (*ptr >= '0') && (*ptr <= '9'))
      strcpy(Display[(int)*ptr-'0'], message_buff);
  }
} 


void mqttReconnect() {
  // Loop until we're reconnected
  char mqttDisplayTopic[strlen(display_topic)+2];
  while (!mqttClient.connected()) 
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (WiFi.status() == WL_CONNECTED) 
    {
      if (!mqttClient.connected()) 
      {
        Serial.println("Connecting to MQTT server");
        if (mqttClient.connect("ESP8266Client")) 
        {
          Serial.println("Connected to MQTT server");
          strcpy(mqttDisplayTopic,display_topic);
          mqttClient.subscribe(strcat(mqttDisplayTopic,"/#"));
          Serial.print("Set Subscribe to : ");
          Serial.println(mqttDisplayTopic);
        } else 
        {
          Serial.println("Could not connect to MQTT server");   
        }
   
      }
    }
  }  
}

/*===============================*/
/*============ MQTT =============*/
/*=========== BEGIN =============*/
/*===============================*/

/*********************************/
/****** LOW LEVEL FUNCTIONS ******/
/************* START *************/
/*********************************/

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

void OLED_FillScreen_160128RGB(unsigned long color)    // fill screen with a given color
{
    unsigned int i;
    OLED_SetPosition_160128RGB(0,0);
    OLED_WriteMemoryStart_160128RGB();
    for(i=0;i<20480;i++)
    {
        OLED_Pixel_160128RGB(color);
    }
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

/*===============================*/
/*===== LOW LEVEL FUNCTIONS =====*/
/*============= END =============*/
/*===============================*/


/*********************************/
/***** HIGH LEVEL FUNCTIONS ******/
/************ START **************/
/*********************************/
 
void OLED_Text_160128RGB(unsigned char x_pos, unsigned char y_pos, unsigned char letter, unsigned long textColor, unsigned long backgroundColor)  // function to show letter
{
    int zeile, j;         // zeile: Anzahle Zeilen=Zeichenhöhe, j=Bytes/Zeile
    int byteZeile = 4;    // Bytes pro Zeile
    int zeileZeichen=37;  // Zeilen pro Zeichen
    
    int count;
    unsigned char index = 0;
    unsigned char mask = 0x80;
    
    for(zeile=0;zeile<zeileZeichen;zeile++)              // Zeichenhöhe
    {
        OLED_SetPosition_160128RGB(x_pos,y_pos);
        OLED_WriteMemoryStart_160128RGB();
        if (letter == 14) byteZeile=2; else byteZeile=4;
        for (j=0;j<byteZeile;j++)                     // Bytes/Zeile
        {
            for (count=0;count<8;count++)     // Pixel/Byte
            {
                if((Verdana[letter][index] & mask) == mask)
                    OLED_Pixel_160128RGB(textColor);
                else
                   OLED_Pixel_160128RGB(backgroundColor);
                mask = mask >> 1;
            }
            index++;
            mask = 0x80;
        }
        y_pos--;
    }
}

void OLED_Text2x_160128RGB(unsigned char x_pos, unsigned char y_pos, unsigned char letter, unsigned long textColor, unsigned long backgroundColor)  // function to show letter (2x size)
{
    int i;
    int count;
    unsigned char mask = 0x80;
    
    for(i=1;i<=16;i++)     // each character is 16 pixels tall
    {
        OLED_SetPosition_160128RGB(x_pos,y_pos);
        OLED_WriteMemoryStart_160128RGB();
        for (count=0;count<10;count++)    // each character is 10 pixels wide
        {
            if((Ascii_1[letter][(count/2)] & mask) == mask)
                OLED_Pixel_160128RGB(textColor);
            else
                OLED_Pixel_160128RGB(backgroundColor);
        }
        y_pos++;
        if((i%2)==0)
        {
            mask = mask >> 1;
        }
    }
}

void OLED_String2x_160128RGB(unsigned char x_pos, unsigned char y_pos, const char array_of_string[], unsigned long textColor, unsigned long backgroundColor)  // function to show String (2x size)
{
    for (int i=0;i < strlen(array_of_string); i++)
       OLED_Text2x_160128RGB(x_pos + i * 12, y_pos, static_cast<int>(array_of_string[i])-32, textColor, backgroundColor);   
}


void OLED_StringVerdana_160128RGB(unsigned char x_pos, unsigned char y_pos, const char array_of_string[], unsigned long textColor, unsigned long backgroundColor)  // function to show Number in Verdana
{
    char array_pos = 0; 
    for (int i=0;i < sizeof(array_of_string); i++)
    {                      // Buchstabenabstand, Zeile, wenn Dezimalpunkt - dann andere Stelle in Font-Array abfragen, sonst Zahlenwert-Position  
       switch (array_of_string[i]) 
       {
         case ' ':
            //if ' ' then print '0'
            array_pos = 0;
            break;
         case '.':
            //Array-Pos for '.'
            array_pos = array_of_string[i]-32;
            break;
        default: 
            //Array-Pos for '0' to '9' 
            array_pos = array_of_string[i]-48;
            break;
     }
     if (array_of_string[i]=='.') x_pos -= 6;
     OLED_Text_160128RGB(x_pos + i * 32, y_pos, array_pos,  textColor, backgroundColor);
     if (array_of_string[i]=='.') x_pos -= 18;
    }      
}


/*===============================*/
/*==== HIGH LEVEL FUNCTIONS =====*/
/*============= END =============*/
/*===============================*/


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
  delay(1);

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
  WiFi.mode(WIFI_STA);
  WiFiStart();

  // initial Sync Time
  getNTPTime(); 
  
  // init mqtt connect
  mqttReconnect(); 
  
}

// the loop function runs over and over again forever
void loop() 
{
  long now; 
  char charNewTemp[5] = "00.0";
  char charNewHum[5]  = "00.0";
  char charDisplay[5]  = "00.0";
  float newHum = 0.0;
  float newTemp = 0.0;
  char charTime[5] ={0};
  char charDate[10] ={0};  
  int sensor_value=0;
  
    
    strcpy(Display[1], "clear");
    OLED_Init_160128RGB();                           // initialize display
    OLED_FillScreen_160128RGB(BLACK);                // fill screen with black

    while(1)                                          // wait here forever
    {
        // check if WLAN is connected
        if (WiFi.status() != WL_CONNECTED)
        {
          WiFiStart();
        }
  
        // check if Clock sync intervall 
        now= millis();
        if(now - previousMillisNTP >= intervalNTP) 
        {
           // save the last time you read the sensor 
           previousMillisNTP = now;   
           // Sync Time
           getNTPTime();
        }
       

        // hh:mm
        strcpy(charTime, printDigits(hour()).c_str());
        strcat(charTime, ":");
        strcat(charTime, printDigits(minute()).c_str());
        OLED_String2x_160128RGB(50, 20, charTime , BLUE, BLACK);   // 0

        // dd.mm.yyyy
        strcpy(charDate, printDigits(day()).c_str());
        strcat(charDate, ".");
        strcat(charDate, printDigits(month()).c_str());
        strcat(charDate, ".");
        strcat(charDate, printDigits(year()).c_str());
        OLED_String2x_160128RGB(20, 0, charDate , BLUE, BLACK);   // 0

       
        // Check if MQTT Server connected
        if (!mqttClient.connected()) 
        {
           Serial.println("MQTT: Call Reconnect... ");
           mqttReconnect();      
           Serial.println("MQTT: Connected");
        }

       
        // send temperature, humidity via MQTT
        now = millis();
        if (now - previousMillisMQTT > 10000) 
        {
          previousMillisMQTT = now;

          digitalWrite(DHTVDD, HIGH);                     // DHT on for read
          delay(500);
          newTemp = dht.readTemperature(false);           // Read temperature if trus as Fahrenheit, if false as Celsius
          newHum = dht.readHumidity();                    // Read humidity (percent)
          digitalWrite(DHTVDD, LOW);                      // DHT off for cool down

          if (checkBound(newTemp, temp, diff)) {
            temp = newTemp;
            Serial.print("New temperature: ");
            Serial.println(String(temp));
            mqttClient.publish(temperature_topic, String(temp).c_str(), true);
          }

          if (checkBound(newHum, hum, diff)) {
            hum = newHum;
            Serial.print("New humidity: ");
            Serial.println(String(hum));
            mqttClient.publish(humidity_topic, String(hum).c_str(), true);
          }  
        }
        mqttClient.loop();

        // Read analog port A0 
        sensor_value  = analogRead(SENSOR);
        int x = (sensor_value*100/1023);
        // Set Driving to OLED
        OLED_Driving_Current_160128RGB(223 * x/100, 197 * x/100, 179 * x/100);      //OLED initialization

        // Display Temerature
        dtostrf(temp, 4, 1, charNewTemp);
        OLED_String2x_160128RGB(20, 100, "Temperatur " , BLUE, BLACK);   // 0
        OLED_StringVerdana_160128RGB(3, 80, charNewTemp, YELLOW, BLACK);   // 0
        OLED_Text_160128RGB(102, 80, 11, YELLOW, BLACK);   // ° - Abstand 28 (Zeichenbreite) + 8
        OLED_Text_160128RGB(126, 80, 10, YELLOW, BLACK);   // C - Abstand 28 (Zeichenbreite) + 8

        digitalWrite(LED_BUILTIN, LOW);   // Turn the LED on (Note that LOW is the voltage level
                                          // but actually the LED is on; this is because 
                                          // it is acive low on the ESP-01)
        delay(2000);                      // Wait for a second

        // Display Humidity
        dtostrf(hum, 4, 1, charNewHum);
        OLED_String2x_160128RGB(20, 100, "Luftfeuchte" , BLUE, BLACK);   // 0
        OLED_StringVerdana_160128RGB(3, 80, charNewHum, YELLOW, BLACK);   // 0
        OLED_Text_160128RGB(102, 80, 12, YELLOW, BLACK);   // % left - Abstand 28 (Zeichenbreite) + 8
        OLED_Text_160128RGB(129, 80, 13, YELLOW, BLACK);   // % right - Abstand 28 (Zeichenbreite) + 8
        digitalWrite(LED_BUILTIN, HIGH);  // Turn the LED off by making the voltage HIGH
        delay(2000);                      // Wait for two seconds (to demonstrate the active low LED)

        for ( int i=0; i<10;i++)
        {
//          Serial.print("OLED:Dislpay ");
//          Serial.print(i);
//          Serial.print(": ");
//          Serial.println(Display[i]);
          if (strcmp(Display[i], "clear") !=0)
          {
            dtostrf(atof(Display[i]), 4, 1, charDisplay);
            char str[1];
            char topDisplay[10];
            sprintf(str, "%d", i);
            strcpy(topDisplay," Display ");
            strcat(topDisplay,str);
            strcat(topDisplay," ");
            OLED_String2x_160128RGB(20, 100, topDisplay , BLUE, BLACK);   // 0
            OLED_StringVerdana_160128RGB(3, 80, charDisplay, YELLOW, BLACK);   // 0
            OLED_Text_160128RGB(102, 80, 11, YELLOW, BLACK);   // ° - Abstand 28 (Zeichenbreite) + 8
            OLED_Text_160128RGB(126, 80, 10, YELLOW, BLACK);   // C - Abstand 28 (Zeichenbreite) + 8
            delay(2000);                      // Wait for a second
          }
        }
    } 
}
