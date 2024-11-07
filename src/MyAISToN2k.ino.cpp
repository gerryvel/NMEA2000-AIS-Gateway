# 1 "C:\\Users\\GERRYA~1\\AppData\\Local\\Temp\\tmp8149skrs"
#include <Arduino.h>
# 1 "C:/Users/gerryadmin/Documents/PlatformIO/Projects/NMEA2000-AIS-Gateway/src/MyAISToN2k.ino"
# 23 "C:/Users/gerryadmin/Documents/PlatformIO/Projects/NMEA2000-AIS-Gateway/src/MyAISToN2k.ino"
#include <Arduino.h>
#include <NMEA2000_CAN.h>
#include <NMEA2000.h>
#include <N2kMessages.h>
#include <NMEA0183.h>
#include <Preferences.h>
#include "LED.h"
#include "NMEA0183AIStoNMEA2000.h"


#define MAX_NMEA0183_MESSAGE_SIZE 150
#define ESP32_CAN_TX_PIN GPIO_NUM_4
#define ESP32_CAN_RX_PIN GPIO_NUM_5
#define Version "V 0.9 vom 07.10.2024"
uint8_t chipid[6];
uint32_t id = 0;
int i = 0;
int NodeAddress;
Preferences preferences;


tNMEA0183Msg NMEA0183Msg;
tNMEA0183 NMEA0183;

const unsigned long TransmitMessages[] PROGMEM = { 129038L,
                                                   129794L,
                                                   129802L,
                                                   129039L,
                                                   129040L,
                                                   129809L,
                                                   129810L,
                                                   0
                                                 };

MyAisDecoder decoder;
AIS::DefaultSentenceParser parser;
void setup();
void HandleSytemTime(const tN2kMsg & N2kMsg);
void HandleGNSS(const tN2kMsg & N2kMsg);
void MyHandleNMEA2000Msg(const tN2kMsg &N2kMsg);
void CheckSourceAddressChange();
void ParseAIVDM_Message();
void loop();
#line 61 "C:/Users/gerryadmin/Documents/PlatformIO/Projects/NMEA2000-AIS-Gateway/src/MyAISToN2k.ino"
void setup() {

  Serial.begin(115200);

  Serial.printf("NMEA2000-AIS-Gateway setup %s start\n", Version);

  LEDInit();
  delay(1000);



  NMEA0183.Begin(&Serial2, 3, 38400);

  esp_efuse_mac_get_default(chipid);
  for (i = 0; i < 6; i++) id += (chipid[i] << (7 * i));


  NMEA2000.SetProductInformation("AIS01",
                                 10,
                                 "NMEA0183 AIS to N2k",
                                 "1.0.0.2 (2023-08-18)",
                                 "1.1.0.0 (2023-08-18)"
                                );

  NMEA2000.SetDeviceInformation(id,
                                135,
                                25,
                                2046
                               );

  preferences.begin("nvs", false);
  NodeAddress = preferences.getInt("LastNodeAddress", 32);
  preferences.end();

  Serial.printf("NodeAddress=%d\n", NodeAddress);

  NMEA2000.SetMode(tNMEA2000::N2km_ListenAndNode, NodeAddress);
  NMEA2000.EnableForward(false);
  NMEA2000.SetMsgHandler(MyHandleNMEA2000Msg);

  if (NMEA2000.Open()){
    Serial.println("NMEA2000 Initialized");
    LEDon(LED(Green));
  }else{
    Serial.println("NMEA2000 Initialized failed");
    LEDon(LED(Red));
  }
 delay (1000);

 LEDoff_RGB();

}



void HandleSytemTime(const tN2kMsg & N2kMsg) {
  unsigned char SID;
  tN2kTimeSource TimeSource;
  double SecondsSinceMidnight = 0;

  ParseN2kSystemTime(N2kMsg, SID, DaysSince1970, SecondsSinceMidnight, TimeSource);
}



void HandleGNSS(const tN2kMsg & N2kMsg) {

  unsigned char SID;
  double Latitude;
  double Longitude;
  double Altitude;
  tN2kGNSStype GNSStype;
  tN2kGNSSmethod GNSSmethod;
  unsigned char nSatellites;
  double HDOP;
  double PDOP;
  double GeoidalSeparation;
  unsigned char nReferenceStations;
  tN2kGNSStype ReferenceStationType;
  uint16_t ReferenceSationID;
  double AgeOfCorrection;
  double SecondsSinceMidnight = 0;

  ParseN2kGNSS(N2kMsg, SID, DaysSince1970, SecondsSinceMidnight, Latitude, Longitude, Altitude,
               GNSStype, GNSSmethod, nSatellites, HDOP, PDOP, GeoidalSeparation,
               nReferenceStations, ReferenceStationType, ReferenceSationID, AgeOfCorrection);
}



void MyHandleNMEA2000Msg(const tN2kMsg &N2kMsg) {

  if (N2kMsg.PGN == 129029UL) HandleGNSS(N2kMsg);
  if (N2kMsg.PGN == 126992UL) HandleSytemTime(N2kMsg);
}



void CheckSourceAddressChange() {
  int SourceAddress = NMEA2000.GetN2kSource();
  if (SourceAddress != NodeAddress) {
    NodeAddress = SourceAddress;
    preferences.begin("nvs", false);
    preferences.putInt("LastNodeAddress", SourceAddress);
    preferences.end();
    Serial.printf("Address Change: New Address=%d\n", SourceAddress);
  }
}



void ParseAIVDM_Message() {
  int i = 0;
  char buf[MAX_NMEA0183_MESSAGE_SIZE];

  if (!NMEA0183.GetMessage(NMEA0183Msg)) return;




  if (!NMEA0183Msg.IsMessageCode("VDM")) return;

  if (!NMEA0183Msg.GetMessage(buf, MAX_NMEA0183_MESSAGE_SIZE)) return;

  strcat(buf, "\n");

  do {
    i = decoder.decodeMsg(buf, strlen(buf), i, parser);
  } while (i != 0);
}



void loop() {


  LEDflash(LED(Green));


  NMEA2000.ParseMessages();
  ParseAIVDM_Message();
  CheckSourceAddressChange();
}