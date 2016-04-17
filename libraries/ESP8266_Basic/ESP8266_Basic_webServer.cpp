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
#include "ESP8266_Basic_webServer.h"

ESP8266_Basic_webServer::ESP8266_Basic_webServer() : webServer(80){ 

  httpUpdater.setup(&webServer); 
 
  webServer.on("/", std::bind(&ESP8266_Basic_webServer::rootPageHandler, this));
  webServer.on("/sensor", std::bind(&ESP8266_Basic_webServer::sensorPageHandler, this));
  webServer.on("/screens", std::bind(&ESP8266_Basic_webServer::screenConfigHandler, this));
 
  webServer.on("/wlan_config", std::bind(&ESP8266_Basic_webServer::wlanPageHandler, this));
  webServer.on("/gpio", std::bind(&ESP8266_Basic_webServer::gpioPageHandler, this));
  //webServer.on("/cfg", std::bind(&ESP8266_Basic_webServer::cfgPageHandler, this));
  webServer.onNotFound(std::bind(&ESP8266_Basic_webServer::handleNotFound, this));
}

//===============================================================================
//  WEB-Server control
//===============================================================================
void ESP8266_Basic_webServer::start(){
  Serial.println("Start WEB-Server");
  
  pinMode(GPIO2, OUTPUT);
  webServer.begin(); 
  Serial.println("HTTP server started");

}
//===> handle WEB-Server <-----------------------------------------------------
void ESP8266_Basic_webServer::handleClient(){
   webServer.handleClient();
}
//===> CFGstruct pointer <-----------------------------------------------------
void ESP8266_Basic_webServer::set_cfgPointer(CFG *p){
  cfg = p;
   char buffer[30];
   sprintf(buffer, "with %%p:  cfg    = %p\n", cfg);
   Serial.print(buffer);

}

//===> OLEDDisplaystruct pointer <---------------------------------------------
void ESP8266_Basic_webServer::set_OLEDPointer(MyScreen *p){
  oled = p;
   char buffer[30];
   sprintf(buffer, "with %%p:  oled    = %p\n", oled);
   Serial.print(buffer);

}

//===> Callback for CFGchange <------------------------------------------------
void ESP8266_Basic_webServer::set_saveConfig_Callback(CallbackFunction c){
  saveConfig_Callback = c;
}
//===> Update Firmware <-------------------------------------------------------
void ESP8266_Basic_webServer::updateFirmware(){
  Serial.println("Updating Firmware.......");
  t_httpUpdate_return ret = ESPhttpUpdate.update(cfg->updateServer, 80, cfg->filePath);
  switch(ret) {
    case HTTP_UPDATE_FAILED:
        Serial.println("[update] Update failed.");
        break;
    case HTTP_UPDATE_NO_UPDATES:
        Serial.println("[update] Update no Update.");
        break;
    case HTTP_UPDATE_OK:
        Serial.println("[update] Update ok."); // may not called we reboot the ESP
        break;
  }
}

//===============================================================================
//  HTML handling
//===============================================================================

//===> Sensor Page <-------------------------------------------------------
void ESP8266_Basic_webServer::sensorPageHandler(){
 String rm = ""
  
  "<!doctype html> <html>"
  "<head> <meta charset='utf-8'>"
  "<title>ESP8266 Configuration</title>"
  "</head>"

  "<body><body bgcolor='#F0F0F0'><font face='VERDANA,ARIAL,HELVETICA'> <form> <font face='VERDANA,ARIAL,HELVETICA'>"
  "<b><h1>ESP8266 Configuration</h1></b>";

  rm += ""
  "<font size='-2'>&copy; by Pf@nne/16   |   " + String(cfg->version) + "</font>"
  "</body bgcolor> </body></font>"
  "</html>"  
  ;

  webServer.send(200, "text/html", rm);


}

//#############################################################################
void ESP8266_Basic_webServer::rootPageHandler()
{
  // Check if there are any GET parameters
  if (webServer.hasArg("webUser")) strcpy(cfg->webUser, webServer.arg("webUser").c_str());
  if (webServer.hasArg("webPassword")) strcpy(cfg->webPassword, webServer.arg("webPassword").c_str());
  if (webServer.hasArg("apName")) strcpy(cfg->apName, webServer.arg("apName").c_str());
  if (webServer.hasArg("apPassword")) strcpy(cfg->apPassword, webServer.arg("apPassword").c_str());
  if (webServer.hasArg("wifiSSID")) strcpy(cfg->wifiSSID, webServer.arg("wifiSSID").c_str());
  if (webServer.hasArg("wifiPSK")) strcpy(cfg->wifiPSK, webServer.arg("wifiPSK").c_str());
  if (webServer.hasArg("wifiIP")) strcpy(cfg->wifiIP, webServer.arg("wifiIP").c_str());
  if (webServer.hasArg("mqttServer")) strcpy(cfg->mqttServer, webServer.arg("mqttServer").c_str());
  if (webServer.hasArg("mqttPort")) strcpy(cfg->mqttPort, webServer.arg("mqttPort").c_str());
  if (webServer.hasArg("mqttDeviceName")) strcpy(cfg->mqttDeviceName, webServer.arg("mqttDeviceName").c_str());
  if (webServer.hasArg("updateServer")) strcpy(cfg->updateServer, webServer.arg("updateServer").c_str());
  if (webServer.hasArg("filePath")) strcpy(cfg->filePath, webServer.arg("filePath").c_str());
  

  String rm = ""
  
  "<!doctype html> <html>"
  "<head> <meta charset='utf-8'>"
  "<title>ESP8266 Configuration</title>"
  "</head>"

  "<body><body bgcolor='#F0F0F0'><font face='VERDANA,ARIAL,HELVETICA'> <form> <font face='VERDANA,ARIAL,HELVETICA'>"
  "<b><h1>ESP8266 Configuration</h1></b>";

  if (WiFi.status() == WL_CONNECTED){
    rm += "ESP8266 connected to: "; rm += WiFi.SSID(); rm += "<br>";
    rm += "DHCP IP: "; rm += String(IPtoString(WiFi.localIP())); rm += "<p></p>";
  }else{
    rm += "ESP8266 ist not connected!"; rm += "<p></p>";
  }	
  
  String str = String(cfg->mqttStatus);
  if (str == "connected"){
    rm += "ESP8266 connected to MQTT-Broker: "; rm += cfg->mqttServer; rm += "<p></p>";
  }

  rm += ""
  "<table width='30%' border='1' cellpadding='0' cellspacing='2'>"
  " <tr>"
  "  <td><b><font size='+1'>WEB Server</font></b></td>"
  "  <td></td>"
  " </tr>"
  " <tr>"
  "  <td>Username</td>"
  "  <td><input type='text' id='webUser' name='webUser' value='" + String(cfg->webUser) + "' size='30' maxlength='40' placeholder='Username'></td>"
  " </tr>"
  " <tr>"
  " <tr>"
  "  <td>Password</td>"
  "  <td><input type='text' id='webPassword' name='webPassword' value='" + String(cfg->webPassword) + "' size='30' maxlength='40' placeholder='Password'></td>"
  " </tr>"
  " <tr>"

  " <tr>"
  "  <td><b><font size='+1'>Accesspoint</font></b></td>"
  "  <td></td>"
  " </tr>"
  " <tr>"
  "  <td>SSID</td>"
  "  <td><input type='text' id='apName' name='apName' value='" + String(cfg->apName) + "' size='30' maxlength='40' placeholder='SSID'></td>"
  " </tr>"
  " <tr>"
  " <tr>"
  "  <td>Password</td>"
  "  <td><input type='text' id='apPassword' name='apPassword' value='" + String(cfg->apPassword) + "' size='30' maxlength='40' placeholder='Password'></td>"
  " </tr>"
  " <tr>"

  " <tr>"
  "  <td><b><font size='+1'>WiFi</font></b></td>"
  "  <td></td>"
  " </tr>"
  "  <td>SSID</td>"
  "  <td><input type='text' id='wifiSSID' name='wifiSSID' value='" + String(cfg->wifiSSID) + "' size='30' maxlength='40' placeholder='SSID'></td>"
  " </tr>"
  " <tr>"
  " <tr>"
  "  <td>Password</td>"
  "  <td><input type='text' id='wifiPSK' name='wifiPSK' value='" + String(cfg->wifiPSK) + "' size='30' maxlength='40' placeholder='Password'></td>"
  " </tr>"
  " <tr>"

  " <tr>"
  "  <td><b><font size='+1'>MQTT</font></b></td>"
  "  <td></td>"
  " </tr>"
  " <tr>"
  "  <td>Broker IP</td>"
  "  <td><input type='text' id='mqttServer' name='mqttServer' value='" + String(cfg->mqttServer) + "' size='30' maxlength='40' placeholder='IP Address'></td>"
  " </tr>"
  " <tr>"
  " <tr>"
  "  <td>Port</td>"
  "  <td><input type='text' id='mqttPort' name='mqttPort' value='" + String(cfg->mqttPort) + "' size='30' maxlength='40' placeholder='Port'></td>"
  " </tr>"
  " <tr>"
  "  <td>Devicename</td>"
  "  <td><input type='text' id='mqttDeviceName' name='mqttDeviceName' value='" + String(cfg->mqttDeviceName) + "' size='30' maxlength='40' placeholder='Devicename'></td>"
  " </tr>"

  " <tr>"
  "  <td><b><font size='+1'>UpdateServer</font></b></td>"
  "  <td></td>"
  " </tr>"
  " <tr>"
  "  <td>Server IP</td>"
  "  <td><input type='text' id='updateServer' name='updateServer' value='" + String(cfg->updateServer) + "' size='30' maxlength='40' placeholder='IP Address'></td>"
  " </tr>"
  " <tr>"
  " <tr>"
  "  <td>FilePath</td>"
  "  <td><input type='text' id='filePath' name='filePath' value='" + String(cfg->filePath) + "' size='30' maxlength='40' placeholder='Path'></td>"
  " </tr>"

  " <tr>"
  "  <td><p></p> </td>"
  "  <td>  </td>"
  " </tr>"
  " <tr>"
  "  <td></td>"
  "  <td><input type='button' onclick=\"location.href='./screens'\"  value='Screens konfigurieren' style='height:30px; width:200px' >  </font></form> </td>"
  " </tr>"


  " <tr>"
  "  <td></td>"
  "  <td></td>"
  " </tr>"
  " <tr>"
  "  <td></td>"
  "  <td><input type='submit' value='Configuration sichern' style='height:30px; width:200px' > </font></form> </td>"
  " </tr>"

 " <tr>"
 "  <td></td>"
 "  <td></td>"
 " </tr>"
 " <tr>"
 "  <td></td>"
 "  <td><input type='submit' value='Update Firmware' id='update' name='update' value='' style='height:30px; width:200px' ></td>"
 " </tr>"

  " <tr>"
  "  <td></td>"
  "  <td></td>"
  " </tr>"
  " <tr>"
  "  <td></td>"
  "  <td><input type='submit' value='RESET' id='reset' name='reset' value='' style='height:30px; width:200px' >  </font></form> </td>"
  " </tr>"

  " <tr>"
  "  <td></td>"
  "  <td></td>"
  " </tr>"
  " <tr>"
  "  <td></td>"
  "  <td><input type='button' onclick=\"location.href='./update'\"  value='Flash Firmware' style='height:30px; width:200px' >  </font></form> </td>"
  " </tr>"

  "</table>"

  "<font size='-2'>&copy; by Pf@nne/16   |   " + String(cfg->version) + "</font>"
  "</body bgcolor> </body></font>"
  "</html>"  
  ;

  webServer.send(200, "text/html", rm);
 
  if (saveConfig_Callback != nullptr)
    saveConfig_Callback();
  else
     Serial.println("null");
	 
  if (webServer.hasArg("reset")) ESP.restart();
  if (webServer.hasArg("update")) updateFirmware();
}

//#############################################################################

void ESP8266_Basic_webServer::wlanPageHandler()
{
  // Check if there are any GET parameters
  if (webServer.hasArg("ssid"))
  {    
    if (webServer.hasArg("password"))
    {
      WiFi.begin(webServer.arg("ssid").c_str(), webServer.arg("password").c_str());
    }
    else
    {
      WiFi.begin(webServer.arg("ssid").c_str());
    }
    
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(500);
      Serial.print(".");
    }
      
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

    delay(1000);
  }
  
  String response_message = "";
  response_message += "<html>";
  response_message += "<head><title>ESP8266 Webserver</title></head>";
  response_message += "<body style=\"background-color:PaleGoldenRod\"><h1><center>WLAN Settings</center></h1>";
  
  if (WiFi.status() == WL_CONNECTED)
  {
    response_message += "Status: Connected<br>";
  }
  else
  {
    response_message += "Status: Disconnected<br>";
  }
  
  response_message += "<p>To connect to a WiFi network, please select a network...</p>";

  // Get number of visible access points
  int ap_count = WiFi.scanNetworks();
  
  if (ap_count == 0)
  {
    response_message += "No access points found.<br>";
  }
  else
  {
    response_message += "<form method=\"get\">";

    // Show access points
    for (uint8_t ap_idx = 0; ap_idx < ap_count; ap_idx++)
    {
      response_message += "<input type=\"radio\" name=\"ssid\" value=\"" + String(WiFi.SSID(ap_idx)) + "\">";
      response_message += String(WiFi.SSID(ap_idx)) + " (RSSI: " + WiFi.RSSI(ap_idx) +")";
      (WiFi.encryptionType(ap_idx) == ENC_TYPE_NONE) ? response_message += " " : response_message += "*";
      response_message += "<br><br>";
    }
    
    response_message += "WiFi password (if required):<br>";
    response_message += "<input type=\"text\" name=\"password\"><br>";
    response_message += "<input type=\"submit\" value=\"Connect\">";
    response_message += "</form>";
  }

  response_message += "</body></html>";
  
  webServer.send(200, "text/html", response_message);
}


void ESP8266_Basic_webServer::gpioPageHandler()
{
  // Check if there are any GET parameters
  if (webServer.hasArg("gpio2"))
  { 
    if (webServer.arg("gpio2") == "1")
    {
      digitalWrite(GPIO2, HIGH);
    }
    else
    {
      digitalWrite(GPIO2, LOW);
    }
  }

  String response_message = "<html><head><title>ESP8266 Webserver</title></head>";
  response_message += "<body style=\"background-color:PaleGoldenRod\"><h1><center>Control GPIO pins</center></h1>";
  response_message += "<form method=\"get\">";

  response_message += "GPIO2:<br>";

  if (digitalRead(GPIO2) == LOW)
  {
    response_message += "<input type=\"radio\" name=\"gpio2\" value=\"1\" onclick=\"submit();\">On<br>";
    response_message += "<input type=\"radio\" name=\"gpio2\" value=\"0\" onclick=\"submit();\" checked>Off<br>";
  }
  else
  {
    response_message += "<input type=\"radio\" name=\"gpio2\" value=\"1\" onclick=\"submit();\" checked>On<br>";
    response_message += "<input type=\"radio\" name=\"gpio2\" value=\"0\" onclick=\"submit();\">Off<br>";
  }

  response_message += "</form></body></html>";

  
  webServer.send(200, "text/html", response_message);
}


void ESP8266_Basic_webServer::screenConfigHandler()
{

  // Check if there are any GET parameters
  if (webServer.hasArg("webNameScreen0")) strcpy(cfg->webNameScreen0, webServer.arg("webNameScreen0").c_str());
  if (webServer.hasArg("webUnitScreen0")) strcpy(cfg->webUnitScreen0, webServer.arg("webUnitScreen0").c_str());
  if (webServer.hasArg("webNameScreen1")) strcpy(cfg->webNameScreen1, webServer.arg("webNameScreen1").c_str());
  if (webServer.hasArg("webUnitScreen1")) strcpy(cfg->webUnitScreen1, webServer.arg("webUnitScreen1").c_str());
  if (webServer.hasArg("webNameScreen2")) strcpy(cfg->webNameScreen2, webServer.arg("webNameScreen2").c_str());
  if (webServer.hasArg("webUnitScreen2")) strcpy(cfg->webUnitScreen2, webServer.arg("webUnitScreen2").c_str());
  if (webServer.hasArg("webNameScreen3")) strcpy(cfg->webNameScreen3, webServer.arg("webNameScreen3").c_str());
  if (webServer.hasArg("webUnitScreen3")) strcpy(cfg->webUnitScreen3, webServer.arg("webUnitScreen3").c_str());
  if (webServer.hasArg("webNameScreen4")) strcpy(cfg->webNameScreen4, webServer.arg("webNameScreen4").c_str());
  if (webServer.hasArg("webUnitScreen4")) strcpy(cfg->webUnitScreen4, webServer.arg("webUnitScreen4").c_str());
  if (webServer.hasArg("webNameScreen5")) strcpy(cfg->webNameScreen5, webServer.arg("webNameScreen5").c_str());
  if (webServer.hasArg("webUnitScreen5")) strcpy(cfg->webUnitScreen5, webServer.arg("webUnitScreen5").c_str());
  if (webServer.hasArg("webNameScreen6")) strcpy(cfg->webNameScreen6, webServer.arg("webNameScreen6").c_str());
  if (webServer.hasArg("webUnitScreen6")) strcpy(cfg->webUnitScreen6, webServer.arg("webUnitScreen6").c_str());
  if (webServer.hasArg("webNameScreen7")) strcpy(cfg->webNameScreen7, webServer.arg("webNameScreen7").c_str());
  if (webServer.hasArg("webUnitScreen7")) strcpy(cfg->webUnitScreen7, webServer.arg("webUnitScreen7").c_str());
  if (webServer.hasArg("webNameScreen8")) strcpy(cfg->webNameScreen8, webServer.arg("webNameScreen8").c_str());
  if (webServer.hasArg("webUnitScreen8")) strcpy(cfg->webUnitScreen8, webServer.arg("webUnitScreen8").c_str());
  if (webServer.hasArg("webNameScreen9")) strcpy(cfg->webNameScreen9, webServer.arg("webNameScreen9").c_str());
  if (webServer.hasArg("webUnitScreen9")) strcpy(cfg->webUnitScreen9, webServer.arg("webUnitScreen9").c_str());

   char buffer[30];

  Serial.print("oled[0]->Screen: ");
  Serial.println(oled[0].Screen);

   sprintf(buffer, "with %%p:  oled[0]->Screen    = %p\n",  &oled[0].Screen);
   Serial.print(buffer);

  Serial.print("oled[1]->Screen: ");
  Serial.println(oled[1].Screen);

   sprintf(buffer, "with %%p:  oled[1]->Screen    = %p\n",  &oled[1].Screen);
   Serial.print(buffer);

  Serial.print("oled[2]->Screen: ");
  Serial.println(oled[2].Screen);

   sprintf(buffer, "with %%p:  oled[2]->Screen    = %p\n",  &oled[2].Screen);
   Serial.print(buffer);

  Serial.print("oled[3]->Screen: ");
  Serial.println(oled[3].Screen);

   sprintf(buffer, "with %%p:  oled[3]->Screen    = %p\n",  &oled[3].Screen);
   Serial.print(buffer);
/*

  Serial.print("oled[4]->Screen: ");
  Serial.println(oled[4]->Screen);
  Serial.print("oled[5]->Screen: ");
  Serial.println(oled[5]->Screen);
  Serial.print("oled[6]->Screen: ");
  Serial.println(oled[6]->Screen);
  Serial.print("oled[7]->Screen: ");
  Serial.println(oled[7]->Screen);
  Serial.print("oled[8]->Screen: ");
  Serial.println(oled[8]->Screen);
  Serial.print("oled[9]->Screen: ");
  Serial.println(oled[91]->Screen);

*/



  String rm = ""

"<!doctype html> <html>"
  "<head> <meta charset='utf-8'>"
  "<title>ESP8266 Configuration</title>"
  "</head>"

  "<body><body bgcolor='#F0F0F0'><font face='VERDANA,ARIAL,HELVETICA'> <form> <font face='VERDANA,ARIAL,HELVETICA'>"
  "<b><h1>ESP8266 Configuration</h1></b>";

  if (WiFi.status() == WL_CONNECTED){
    rm += "ESP8266 connected to: "; rm += WiFi.SSID(); rm += "<br>";
    rm += "DHCP IP: "; rm += String(IPtoString(WiFi.localIP())); rm += "<p></p>";
  }else{
    rm += "ESP8266 ist not connected!"; rm += "<p></p>";
  }	
  
  String str = String(cfg->mqttStatus);
  if (str == "connected"){
    rm += "ESP8266 connected to MQTT-Broker: "; rm += cfg->mqttServer; rm += "<p></p>";
  }
//  height: 732px;
  rm += ""
  
  " <table style='width: 440px;' border='1' cellpadding='0' cellspacing='2'>"
  " <tbody> "
  "		<tr> "
  "			<td><b><font size='+1'>Screens</font></b></td>"
  "			<td style='width: 192px;'>Name</td>"
  "			<td>aktueller Wert</td>"
  "			<td>Einheit</td>"
  "		</tr>"

  "		<tr>"
  "			<td>Temp. in</td>"
  "			<td style='width: 192px;'>Temperatur</td>"
  "			<td>" + String(oled[4].Screen) + "</td>"
  "			<td>*C</td>"
  "		</tr>"

  "		<tr>"
  "			<td>Humi. in</td>"
  "			<td style='width: 192px;'>Luftfeuchte</td>"
  "			<td>" + String(oled[5].Screen) + "</td>"
  "			<td>%</td>"
  "		</tr>"

  "		<tr>"
  "			<td>Screen 0</td>"
  "			<td style='width: 192px;'><input id='webNameScreen0' name='webNameScreen0' value='" + String(cfg->webNameScreen0) + "' size='13' maxlength='13' placeholder='Name Screen 0' type='text'></td>"
  "			<td>" + String(oled[0].Screen) + "</td>"
  "			<td><input id='webUnitScreen0' name='webUnitScreen0' value='" + String(cfg->webUnitScreen0) + "' size='2' maxlength='2' placeholder='*C' type='text'></td>"
  "		</tr>"

  "		<tr>"
  "			<td>Screen 1</td>"
  "			<td style='width: 192px;'><input id='webNameScreen1' name='webNameScreen1' value='" + String(cfg->webNameScreen1) + "' size='13' maxlength='13' placeholder='Name Screen 1' type='text'></td>"
  "			<td>" + String(oled[1].Screen) + "</td>"
  "			<td><input id='webUnitScreen1' name='webUnitScreen1' value='" + String(cfg->webUnitScreen1) + "' size='2' maxlength='2' placeholder='*C' type='text'></td>"
  "		</tr>"

  "		<tr>"
  "			<td>Screen 2</td>"
  "			<td style='width: 192px;'><input id='webNameScreen2' name='webNameScreen2' value='" + String(cfg->webNameScreen2) + "' size='13' maxlength='13' placeholder='Name Screen 2' type='text'></td>"
  "			<td>" + String(oled[2].Screen) + "</td>"
  "			<td><input id='webUnitScreen2' name='webUnitScreen2' value='" + String(cfg->webUnitScreen2) + "' size='2' maxlength='2' placeholder='*C' type='text'></td>"
  "		</tr>"

  "		<tr>"
  "			<td>Screen 3</td>"
  "			<td style='width: 192px;'><input id='webNameScreen3' name='webNameScreen3' value='" + String(cfg->webNameScreen3) + "' size='13' maxlength='13' placeholder='Name Screen 3' type='text'></td>"
  "			<td>" + String(oled[3].Screen) + "</td>"
  "			<td><input id='webUnitScreen3' name='webUnitScreen3' value='" + String(cfg->webUnitScreen3) + "' size='2' maxlength='2' placeholder='*C' type='text'></td>"
  "		</tr>"

  "		<tr>"
  "			<td></td>"
  "			<td style='width: 192px;'><input value='Configuration sichern' style='height: 30px; width: 200px;' type='submit'> </td>"
  "			<td></td>"  
  "			<td></td>"
  "		</tr>"

  "		<tr>"
  "			<td></td>"
  "			<td style='width: 192px;'><input onclick=\"location.href='./'\" value='Homepage' style='height: 30px; width: 200px;' type='button'> </td>"
  "			<td></td>"  
  "			<td></td>"
  "		</tr>"
  "</tbody>"
  "</table>"
  "</form>"
  "<font size='-2'>&copy; by Pf@nne/16   |   " + String(cfg->version) + "</font>"
  "</body>"
  "</html>"
  ;
 
  if (saveConfig_Callback != nullptr)
    saveConfig_Callback();
  else
     Serial.println("null");

  webServer.send(200, "text/html", rm);
}


void ESP8266_Basic_webServer::handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += webServer.uri();
  message += "\nMethod: ";
  message += (webServer.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += webServer.args();
  message += "\n";
  
  for (uint8_t i = 0; i < webServer.args(); i++)
  {
    message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
  }
  
  webServer.send(404, "text/plain", message);
}

//===============================================================================
//  helpers
//===============================================================================

//===> IPToString  <-----------------------------------------------------------
String ESP8266_Basic_webServer::IPtoString(IPAddress ip) {
  String res = "";
  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }
  res += String(((ip >> 8 * 3)) & 0xFF);
  return res;
}
