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
#define Version "ESP_Basic by Pf@nne V0.200/winfighter V0.35"
 
  typedef char* topicField; 

//publish struct
  const int pub_e1 = 4;			//define TreeDepht here!!!
  const int pub_e2 = 5;
  const int pub_e3 = 30;
  const int pub_e4 = 0;
  
  typedef struct Tpub_topicE1{
    topicField item[pub_e1];
	int count;
  };
  typedef struct Tpub_topicE2{
    topicField item[pub_e1][pub_e2];
	int count;
  };
  typedef struct Tpub_topicE3{
    topicField item[pub_e1][pub_e2][pub_e3];
	int count;
  };
 
  typedef struct Tpub_Topic{
    Tpub_topicE1 E1;
    Tpub_topicE2 E2;
    Tpub_topicE3 E3;	
  };
  
//subscribe struct
  const int sub_e1 = 5;			//define TreeDepht here!!!
  const int sub_e2 = 5;
  const int sub_e3 = 9;
  const int sub_e4 = 0;
  
  typedef struct Tsub_topicE1{
    topicField item[sub_e1];
	int count;
  };
  typedef struct Tsub_topicE2{
    topicField item[sub_e1][sub_e2];
	int count;
  };
  typedef struct Tsub_topicE3{
    topicField item[sub_e1][sub_e2][sub_e3];
	int count;
  };
 
  typedef struct Tsub_Topic{
    Tsub_topicE1 E1;
    Tsub_topicE2 E2;
    Tsub_topicE3 E3;	
  };
  
 
class Topics{
public:
  Topics();
  Tpub_Topic pub; 
  Tsub_Topic sub; 
private:
}; 


// Config struct
  typedef struct CFG{
    char version[45] = Version;
	char configSize[5]; 
    char webUser[40];
    char webPassword[40];
    char apName[40];
    char apPassword[40];
    char wifiSSID[40];
    char wifiPSK[40];
    char wifiIP[20];
    char mqttServer[20];
    char mqttPort[6];
    char mqttDeviceName[21];
    char mqttSecSub[31];
	char mqttStatus[20];
	char updateServer[20];
	char filePath[40];
    char webNameScreen0[17];
    char webUnitScreen0[3];
    char webDurationScreen0[3];
    char webNameScreen1[17];
    char webUnitScreen1[3];
    char webDurationScreen1[3];
    char webNameScreen2[17];
    char webUnitScreen2[3];
    char webDurationScreen2[3];
    char webNameScreen3[17];
    char webUnitScreen3[3];
    char webDurationScreen3[3];
    char webNameScreen4[17];
    char webUnitScreen4[3];
    char webDurationScreen4[3];
    char webNameScreen5[17];
    char webUnitScreen5[3];
    char webDurationScreen5[3];
    char webNameScreen6[17];
    char webUnitScreen6[3];
    char webDurationScreen6[3];
    char webNameScreen7[17];
    char webUnitScreen7[3];
    char webDurationScreen7[3];
    char webNameScreen8[17];
    char webUnitScreen8[3];
    char webDurationScreen8[3];
    char webNameScreen9[17];
    char webUnitScreen9[3];
    char webDurationScreen9[3];

  };
  

  typedef struct TdissectResult{
    bool found = false;
    String topic = "";
	String value = "";
	String itemPath = "";
	int treeDepth = 0;
    int E1 = NULL;
    int E2 = NULL;
    int E3 = NULL;
    int E4 = NULL;   
  };
  
// MyFile struct
  typedef struct MyFile{
    char Field_01[15];
    char Field_02[15];
  };
  
 
 //MyDisplay sruct
 typedef struct MyScreen{
	 char Screen[6] = "clear";
	 char topRow[17] = "--Display n--";
	 char mesurementUnit[3] = "*C"; 
	 char duration[3] = "2";
 };
 
 //WeatherIcon sruct
 typedef struct WeatherIcon{
	 char weatherCode[3] = "99";
	 char forecastDate[17] = "01 November 1970";
	 char condition[31] = "überwiegend wolkig";
 };

  