/*
  This code is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  This code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
// Reads AIVDM messages from NMEA0183 (ESP32 UART 2 on GPIO 16) and forwards them to the N2k bus
// Version 0.7, 13.01.2022, AK-Homberger
// Version 0.8, 11.08.2023, Gerry Sebb

// Is using modified (clang#14 to clang#11) version of this AIS decoder: https://github.com/aduvenhage/ais-decoder
// AIS decoder is under MIT license: https://github.com/aduvenhage/ais-decoder/blob/master/LICENSE

// Currently following AIS message types are supported: 1-3, 5, 14, 18, 19, 24A, 24B

#include <Arduino.h>
#include <NMEA2000_CAN.h>  // This will automatically choose right CAN library and create suitable NMEA2000 object
#include <NMEA2000.h>
#include <N2kMessages.h>
#include <NMEA0183.h>
#include <Preferences.h>
#include "LED.h"
#include "NMEA0183AIStoNMEA2000.h"  // Contains class, global variables and code !!!

// Variable
#define MAX_NMEA0183_MESSAGE_SIZE 150
#define ESP32_CAN_TX_PIN GPIO_NUM_4  // Set CAN TX port to 4 
#define ESP32_CAN_RX_PIN GPIO_NUM_5  // Set CAN RX port to 5
#define Version "V 0.9 vom 07.10.2024"

int NodeAddress;                    // To store last Node Address
Preferences preferences;            // Nonvolatile storage on ESP32 - To store LastDeviceAddress

// NMEA message and stream for AIS receiving
tNMEA0183Msg NMEA0183Msg;
tNMEA0183 NMEA0183;

const unsigned long TransmitMessages[] PROGMEM = { 129038L, // Class A position report
                                                   129794L, // Class A static and voyage related data
                                                   129802L, // Safety related broadcast message
                                                   129039L, // Class B standard position report
                                                   129040L, // Class B extended position report
                                                   129809L, // Class B static data part A
                                                   129810L, // Class B static data part B
                                                   0
                                                 };

MyAisDecoder decoder;               // Create decoder object
AIS::DefaultSentenceParser parser;  // Create parser object

//*****************************************************************************
void setup() {

  uint8_t chipid[6];
  uint32_t id = 0;
  int i = 0;

  Serial.begin(115200);

  Serial.printf("NMEA2000-AIS-Gateway setup %s start\n", Version);

  LEDInit();

  delay(1000);

  // Serial2.begin(38400, SERIAL_8N1);   // Configure Serial2 (GPIO 16)
  NMEA0183.Begin(&Serial2, 3, 38400); // Start NMEA0183 stream handling

  esp_efuse_mac_get_default(chipid);
  for (i = 0; i < 6; i++) id += (chipid[i] << (7 * i));

  // Setup NMEA2000 system
  NMEA2000.SetProductInformation("AIS01", // Manufacturer's Model serial code
                                 10,  // Manufacturer's product code
                                 "NMEA0183 AIS to N2k",  // Manufacturer's Model ID
                                 "1.0.0.2 (2024-10-07)", // Manufacturer's Software version code
                                 "1.1.0.0 (2023-08-18)"  // Manufacturer's Model version
                                );
  // Det device information
  NMEA2000.SetDeviceInformation(id,  // Unique number. Use e.g. Serial number.
                                135, // Device function=NMEA 0183 Gateway. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20%26%20function%20codes%20v%202.00.pdf
                                25,  // Device class=Inter/Intranetwork Device. See codes on http://www.nmea.org/Assets/20120726%20nmea%202000%20class%20%26%20function%20codes%20v%202.00.pdf
                                2046 // Just choosen free from code list on http://www.nmea.org/Assets/20121020%20nmea%202000%20registration%20list.pdf
                               );

  preferences.begin("nvs", false);                          // Open nonvolatile storage (nvs)
  NodeAddress = preferences.getInt("LastNodeAddress", 32);  // Read stored last NodeAddress, default 32
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


//*****************************************************************************
void HandleSytemTime(const tN2kMsg & N2kMsg) {
  unsigned char SID;
  tN2kTimeSource TimeSource;
  double SecondsSinceMidnight = 0;

  ParseN2kSystemTime(N2kMsg, SID, DaysSince1970, SecondsSinceMidnight, TimeSource);
}


//*****************************************************************************
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


//*****************************************************************************
void MyHandleNMEA2000Msg(const tN2kMsg &N2kMsg) {

  if (N2kMsg.PGN == 129029UL) HandleGNSS(N2kMsg);      // Just to get time from GNSS
  if (N2kMsg.PGN == 126992UL) HandleSytemTime(N2kMsg); // or this way
}


//*****************************************************************************
void CheckSourceAddressChange() {
  int SourceAddress = NMEA2000.GetN2kSource();
  if (SourceAddress != NodeAddress) { // Save potentially changed Source Address to NVS memory
    NodeAddress = SourceAddress;      // Set new Node Address (to save only once)
    preferences.begin("nvs", false);
    preferences.putInt("LastNodeAddress", SourceAddress);
    preferences.end();
    Serial.printf("Address Change: New Address=%d\n", SourceAddress);
  }
}


//*****************************************************************************
void ParseAIVDM_Message() {
  int i = 0;
  char buf[MAX_NMEA0183_MESSAGE_SIZE];

  if (!NMEA0183.GetMessage(NMEA0183Msg)) return;  // New message? If not return

  // Select (comment/uncomment) if you want to decode only other ship (AIVDM) or also own ship (AIVDO) messages
  
  // if (!NMEA0183Msg.IsMessageCode("VDM") && !NMEA0183Msg.IsMessageCode("VDO")) return;
  if (!NMEA0183Msg.IsMessageCode("VDM")) return;   // Not a AIVDM message, return

  if (!NMEA0183Msg.GetMessage(buf, MAX_NMEA0183_MESSAGE_SIZE)) return;  // GetMessage copy to buffer failed

  strcat(buf, "\n");  // Decoder expects that.

  do {
    i = decoder.decodeMsg(buf, strlen(buf), i, parser);   // Decode AIVDM/AIVDO message
  } while (i != 0);                                       // To be called until return value is 0
}


//*****************************************************************************
void loop() {

// LED
  LEDflash(LED(Green)); // blink for loop run

// NMEA
  NMEA2000.ParseMessages();
  ParseAIVDM_Message();      // Parse AIS
  CheckSourceAddressChange();
}
