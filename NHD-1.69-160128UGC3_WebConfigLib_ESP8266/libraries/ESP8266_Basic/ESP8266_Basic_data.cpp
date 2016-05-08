/******************************************************************************

  ProjectName: ESP8266_Basic                      ***** *****
  SubTitle   : ESP8266 Template                  *     *     ************
                                                *   **   **   *           *
  Copyright by Pf@nne                          *   *   *   *   *   ****    *
                                               *   *       *   *   *   *   *
  Last modification by:                        *   *       *   *   ****    *
  - Pf@nne (pf@nne-mail.de)                     *   *     *****           *
                                                 *   *        *   *******
  Date    : 29.03.2016                            *****      *   *
  Version : alpha 0.200                                     *   *
  Revison :                                                *****

********************************************************************************/
#include <Arduino.h>
#include "ESP8266_Basic_data.h"

Topics::Topics(){ 

/* MQTT subcribe TopicTree struct   !!define TreeDepht in .h!!!
  [x] mqttDeviceName 
   ¦
   ¦-[0] WiFiConfig
   ¦  ¦-[00] WEBserver
   ¦  ¦  ¦-[000] Username
   ¦  ¦  +-[001] Password
   ¦  ¦-[01] Accesspoint
   ¦  ¦  ¦-[010] SSID
   ¦  ¦  +-[011] Password
   ¦  ¦-[02] WiFi
   ¦  ¦  ¦-[020] SSID
   ¦  ¦  ¦-[021] Password
   ¦  ¦  +-[022] IP
   ¦  ¦-[03] MQTT
   ¦  ¦  ¦-[030] Server
   ¦  ¦  ¦-[031] Port
   ¦  ¦  +-[032] Status
   ¦  +-[04] UpdateServer
   ¦     ¦-[040] Server
   ¦     +-[041] FilePath
   ¦     
   ¦-[1] Control
   ¦  ¦-[10] ESP8266
   ¦  ¦  ¦-[100] Reboot
   ¦  ¦  ¦-[101] ShowConfig   
   ¦  ¦  +-[102] Update Firmware   
   ¦     
   ¦-[2] Display
   ¦  ¦-[20] NHD 1.69
   ¦  ¦  ¦-[200] Screen 0
   ¦  ¦  ¦-[201] Screen 1   
   ¦  ¦  ¦-[202] Screen 2   
   ¦  ¦  ¦-[203] Screen 3   
   ¦  ¦  ¦-[204] Screen 4   
   ¦  ¦  ¦-[205] Screen 5   
   ¦  ¦  ¦-[206] Screen 6   
   ¦  ¦  ¦-[207] Screen 7   
   ¦  ¦  ¦-[208] Screen 8   
   ¦  ¦  +-[209] Screen 9   
   ¦     
   ¦-[3] Warther
   ¦  ¦-[30] Forecast
   ¦  ¦  ¦-[300] Day 0
   ¦  ¦  ¦-[301] Day 1   
   ¦  ¦  ¦-[302] Day 2   
   ¦  ¦  ¦-[303] Day 3   
   ¦  ¦  ¦-[304] Day 4   
   ¦  ¦  +-[305] Day 5   
   ¦  ¦-[31] Condition
   ¦  ¦  ¦-[310] Day 0
   ¦  ¦  ¦-[311] Day 1   
   ¦  ¦  ¦-[312] Day 2   
   ¦  ¦  ¦-[313] Day 3   
   ¦  ¦  ¦-[314] Day 4   
   ¦  ¦  +-[315] Day 5   
   ¦  ¦-[32] Date
   ¦  ¦  ¦-[320] Day 0
   ¦  ¦  ¦-[321] Day 1   
   ¦  ¦  ¦-[322] Day 2   
   ¦  ¦  ¦-[323] Day 3   
   ¦  ¦  ¦-[324] Day 4   
   ¦  ¦  +-[325] Day 5   
   ¦     
*/

  sub.E1.count = sub_e1;
  sub.E2.count = sub_e2;
  sub.E3.count = sub_e3;
  
  sub.E1.item[0] = "WiFiConfig";
  sub.E2.item[0][0] = "WEBserver";
  sub.E3.item[0][0][0] = "Username";
  sub.E3.item[0][0][1] = "Password";
  sub.E2.item[0][1] = "Accesspoint";
  sub.E3.item[0][1][0] = "SSID";
  sub.E3.item[0][1][1] = "Password";
  sub.E2.item[0][2] = "WiFi";
  sub.E3.item[0][2][0] = "SSID";
  sub.E3.item[0][2][1] = "Password";
  sub.E3.item[0][2][2] = "IP";
  sub.E2.item[0][3] = "MQTT";
  sub.E3.item[0][3][0] = "Server";
  sub.E3.item[0][3][1] = "Port";
  sub.E3.item[0][3][2] = "Status";
  sub.E2.item[0][4] = "UpdateServer";
  sub.E3.item[0][4][0] = "Server";
  sub.E3.item[0][4][1] = "FilePath";
  
  sub.E1.item[1] = "Control";
  sub.E2.item[1][0] = "ESP8266";
  sub.E3.item[1][0][0] = "Reboot";
  sub.E3.item[1][0][1] = "ShowConfig";
  sub.E3.item[1][0][2] = "updateFirmware";
  sub.E3.item[1][0][3] = "MeasureTime";

  sub.E1.item[2] = "Display";
  sub.E2.item[2][0] = "NHD_1.69";
  sub.E3.item[2][0][0] = "Screen_0";
  sub.E3.item[2][0][1] = "Screen_1";
  sub.E3.item[2][0][2] = "Screen_2";
  sub.E3.item[2][0][3] = "Screen_3";
  sub.E3.item[2][0][4] = "Screen_4";
  sub.E3.item[2][0][5] = "Screen_5";
  sub.E3.item[2][0][6] = "Screen_6";
  sub.E3.item[2][0][7] = "Screen_7";
  sub.E3.item[2][0][8] = "Screen_8";
  sub.E3.item[2][0][9] = "Screen_9";

  sub.E1.item[3] = "Weather";
  sub.E2.item[3][0] = "Forecast";
  sub.E3.item[3][0][0] = "Day_0";
  sub.E3.item[3][0][1] = "Day_1";
  sub.E3.item[3][0][2] = "Day_2";
  sub.E3.item[3][0][3] = "Day_3";
  sub.E3.item[3][0][4] = "Day_4";
  sub.E3.item[3][0][5] = "Day_5";
  sub.E2.item[3][1] = "Condition";
  sub.E3.item[3][1][0] = "Day_0";
  sub.E3.item[3][1][1] = "Day_1";
  sub.E3.item[3][1][2] = "Day_2";
  sub.E3.item[3][1][3] = "Day_3";
  sub.E3.item[3][1][4] = "Day_4";
  sub.E3.item[3][1][5] = "Day_5";
  sub.E2.item[3][2] = "Date";
  sub.E3.item[3][2][0] = "Day_0";
  sub.E3.item[3][2][1] = "Day_1";
  sub.E3.item[3][2][2] = "Day_2";
  sub.E3.item[3][2][3] = "Day_3";
  sub.E3.item[3][2][4] = "Day_4";
  sub.E3.item[3][2][5] = "Day_5";

  sub.E1.item[4] = "File";
  sub.E2.item[4][0] = "Read";
  sub.E3.item[4][0][0] = "Field_01";
  sub.E3.item[4][0][1] = "Field_02";
  sub.E2.item[4][1] = "Write";
  sub.E3.item[4][1][0] = "Field_01";
  sub.E3.item[4][1][1] = "Field_02";

/* MQTT publish TopicTree struct   !!define TreeDepht in .h!!!
  [x] mqttDeviceName 
   ¦
   ¦-[0] WiFiConfig
   ¦  ¦-[00] WEBserver
   ¦  ¦  ¦-[000] Username
   ¦  ¦  +-[001] Password
   ¦  ¦-[01] Accesspoint
   ¦  ¦  ¦-[010] SSID
   ¦  ¦  +-[011] Password
   ¦  ¦-[02] WiFi
   ¦  ¦  ¦-[020] SSID
   ¦  ¦  ¦-[021] Password
   ¦  ¦  +-[022] IP
   ¦  ¦-[03] MQTT
   ¦  ¦  ¦-[030] Server
   ¦  ¦  ¦-[031] Port
   ¦  ¦  +-[032] Status
   ¦  +-[04] UpdateServer
   ¦     ¦-[040] Server
   ¦     +-[041] FilePath
   ¦     
   ¦-[1] Control
   ¦  +-[10] Status
   ¦     ¦-[100] WiFi
   ¦     ¦-[101] MQTT
   ¦     +-[101] update Firmware   
   ¦
   ¦-[2] Sensor
   ¦  ¦-[10] temperatur
   ¦  ¦  ¦-[210] T1
   ¦  ¦  +-[211] T2   
   ¦  ¦-[20] humidity
   ¦  ¦  ¦-[220] H1
   ¦  ¦  +-[221] H2   
*/

  pub.E1.count = pub_e1;
  pub.E2.count = pub_e2;
  pub.E3.count = pub_e3;
  
  pub.E1.item[0] = "WiFiConfig";
  pub.E2.item[0][0] = "WEBserver";
  pub.E3.item[0][0][0] = "Username";
  pub.E3.item[0][0][1] = "Password";
  pub.E2.item[0][1] = "Accesspoint";
  pub.E3.item[0][1][0] = "SSID";
  pub.E3.item[0][1][1] = "Password";
  pub.E2.item[0][2] = "WiFi";
  pub.E3.item[0][2][0] = "SSID";
  pub.E3.item[0][2][1] = "Password";
  pub.E3.item[0][2][2] = "IP";
  pub.E2.item[0][3] = "MQTT";
  pub.E3.item[0][3][0] = "Server";
  pub.E3.item[0][3][1] = "Port";
  pub.E3.item[0][3][2] = "Status";
  pub.E2.item[0][4] = "UpdateServer";
  pub.E3.item[0][4][0] = "Server";
  pub.E3.item[0][4][1] = "FilePath";
  
  pub.E1.item[1] = "Control";
  pub.E2.item[1][0] = "Status";
  pub.E3.item[1][0][0] = "WiFi";
  pub.E3.item[1][0][1] = "MQTT";
  pub.E3.item[1][0][2] = "updateFirmware";

  pub.E1.item[2] = "Sensor";
  pub.E2.item[2][1] = "temperature";
  pub.E3.item[2][1][0] = "T1";
  pub.E3.item[2][1][1] = "T2";
  pub.E3.item[2][1][2] = "T3";
  pub.E3.item[2][1][3] = "T4";
  pub.E3.item[2][1][4] = "T5";
  pub.E3.item[2][1][5] = "T6";
  pub.E3.item[2][1][6] = "T7";
  pub.E3.item[2][1][7] = "T8";
  pub.E3.item[2][1][8] = "T9";
  pub.E3.item[2][1][9] = "T10";
  pub.E3.item[2][1][10] = "T11";
  pub.E3.item[2][1][11] = "T12";
  pub.E3.item[2][1][12] = "T13";
  pub.E3.item[2][1][13] = "T14";
  pub.E3.item[2][1][14] = "T15";
  pub.E3.item[2][1][15] = "T16";
  pub.E3.item[2][1][16] = "T17";
  pub.E3.item[2][1][17] = "T18";
  pub.E3.item[2][1][18] = "T19";
  pub.E3.item[2][1][19] = "T20";
  pub.E3.item[2][1][20] = "T21";
  pub.E3.item[2][1][21] = "T22";
  pub.E3.item[2][1][22] = "T23";
  pub.E3.item[2][1][23] = "T24";
  pub.E3.item[2][1][24] = "T25";
  pub.E3.item[2][1][25] = "T26";
  pub.E3.item[2][1][26] = "T27";
  pub.E3.item[2][1][27] = "T28";
  pub.E3.item[2][1][28] = "T29";
  pub.E3.item[2][1][29] = "T30";
  pub.E2.item[2][2] = "humidity";
  pub.E3.item[2][2][0] = "H1";
  pub.E3.item[2][2][1] = "H2";
  pub.E3.item[2][2][2] = "H3";
  pub.E3.item[2][2][3] = "H4";
  pub.E3.item[2][2][4] = "H5";
  pub.E3.item[2][2][5] = "H6";
  pub.E3.item[2][2][6] = "H7";
  pub.E3.item[2][2][7] = "H8";
  pub.E3.item[2][2][8] = "H9";
  pub.E3.item[2][2][9] = "H10";
  pub.E3.item[2][2][10] = "H11";
  pub.E3.item[2][2][11] = "H12";
  pub.E3.item[2][2][12] = "H13";
  pub.E3.item[2][2][13] = "H14";
  pub.E3.item[2][2][14] = "H15";
  pub.E3.item[2][2][15] = "H16";
  pub.E3.item[2][2][16] = "H17";
  pub.E3.item[2][2][17] = "H18";
  pub.E3.item[2][2][18] = "H19";
  pub.E3.item[2][2][19] = "H20";
  pub.E3.item[2][2][20] = "H21";
  pub.E3.item[2][2][21] = "H22";
  pub.E3.item[2][2][22] = "H23";
  pub.E3.item[2][2][23] = "H24";
  pub.E3.item[2][2][24] = "H25";
  pub.E3.item[2][2][25] = "H26";
  pub.E3.item[2][2][26] = "H27";
  pub.E3.item[2][2][27] = "H28";
  pub.E3.item[2][2][28] = "H29";
  pub.E3.item[2][2][29] = "H30";


}
