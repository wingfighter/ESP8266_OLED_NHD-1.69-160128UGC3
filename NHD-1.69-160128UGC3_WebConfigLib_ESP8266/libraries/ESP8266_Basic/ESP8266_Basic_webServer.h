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
#pragma once

  #include <Arduino.h>
  #include <ESP8266WiFi.h>			//https://github.com/esp8266/Arduino	

  #include <ESP8266WebServer.h>
  #include <WiFiClient.h> 
  #include <DNSServer.h>
  #include <ESP8266HTTPUpdateServer.h>
  #include <ESP8266httpUpdate.h>
  #include <FS.h>

  #define DBG_OUTPUT_PORT Serial
  #define GPIO2 2


  #include "ESP8266_Basic_data.h"
  #include <functional>
  typedef std::function<void(void)> CallbackFunction;
//  using namespace std::placeholders;


class ESP8266_Basic_webServer{
public:
  ESP8266_Basic_webServer();
  void start();
  void handleClient();
  
  void set_cfgPointer(CFG *p);  
  void set_OLEDPointer(MyScreen *p);
//  void set_ConfigRunningPointer(bool *p);
  void set_saveConfig_Callback(CallbackFunction c);
  void updateFirmware();
  
private:
  ESP8266WebServer webServer;
  ESP8266HTTPUpdateServer httpUpdater;
  CallbackFunction saveConfig_Callback;
  CFG *cfg;
  MyScreen *oled;
  File fsUploadFile;
//  bool fConfigRunning;
  //Page controls----------------
  void rootPageHandler();
  void sensorPageHandler();
  void screenConfigHandler();
  void wlanPageHandler();
  void gpioPageHandler();
  void cfgPageHandler();
  void handleNotFound();
/*  bool handleFileRead(String path);
  void handleFileUpload();
  void handleFileDelete();
  void handleFileCreate();
  void handleFileList();
*/

  //helpers----------------------------
  String IPtoString(IPAddress ip);
//  String formatBytes(size_t bytes);
//  String getContentType(String filename);

};

