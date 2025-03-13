# NMEA0183 to NMEA2000 AIS Gateway

![ESP32](https://img.shields.io/badge/ESP32-grey?logo=Espressif)
![KiCad](https://img.shields.io/badge/KiCad-darkblue?logo=KiCad)
[![OBP](https://img.shields.io/badge/Sailing_with-OpenBoatsProjects-blue)](https://open-boat-projects.org/de/)

This repository shows how to build a gateway to read NMEA0183 AIS messages and transform them to NMEA2000 PGNs.

![grafik](https://github.com/gerryvel/NMEA2000-AIS-Gateway/assets/17195231/892ed235-b84e-4af3-bd27-74ac6808b6d8)

If you need the opposite direction, then this might be of interest for you: https://github.com/ronzeiller/NMEA0183-AIS

The purpose is to use an existing NMEA0183 AIS receiver together with devices (e.g. MFD) that support only NMEA2000.
Many new chart plotters only support NMEA2000 and do not have an additional RS232 or RS422 input.

I had to change and add some AIS related [PGN structures](https://github.com/AK-Homberger/NMEA2000-AIS-Gateway/blob/bea3ec6255678d2c806bf09e5bebd815074605ea/MyAISToN2k/NMEA0183AIStoNMEA2000.h#L73) in addition to the NMEA2000 library to make this work also with Raymarine MFDs.

The used ESP32 reads AIVDM messages from UART2 (connected to GPIO 16, baud rate 38400). Forwarding to NMEA2000 is done via a CAN bus transceiver.

The solution is re-using AIS decoder code from [this](https://github.com/aduvenhage/ais-decoder) GitHub repository. It is published under [MIT-License](https://github.com/aduvenhage/ais-decoder/blob/master/LICENSE).

It was necessary to change the code, to make it usable with the Arduino IDE and the ESP32. The reason for the changes was the different C compiler versions (clang#14 to clang#11). It was quite an effort to do the backport to Arduino clang#11 version, because of missing functions.

Currently, following AIS message types are supported: 1-3, 5, 14, 18, 19, 24A, 24B

Detailled information regarding AIS messages can be found [here](https://gpsd.gitlab.io/gpsd/AIVDM.html).

Only AIVDM messages (other ships) are decoded by default. If you also want to decode and forward own ship messages (AIVDO) comment/uncomment the appropriate lines (code change requires Arduino IDE):

```
// Select (comment/uncomment) if you want to decode only other ship (AIVDM) or also own ship (AIVDO) messages
// if (!NMEA0183Msg.IsMessageCode("VDM") && !NMEA0183Msg.IsMessageCode("VDO")) return;   // Not a AIVDM/AIVDO message, return
if (!NMEA0183Msg.IsMessageCode("VDM")) return;   // Not a AIVDM message, return
```
## Software Install
To install the program on the ESP32 you do have two options:

1. Install the binary file with the **NodeMCU PyFlasher** tool
2. Compile and upload the program with the **Arduino IDE**

## NodeMCU PyFlasher

The easiest way to upload the binary file is to use the [NodeMCU PyFlasher tool](https://github.com/marcelstoer/nodemcu-pyflasher) tool.

Download the appropriate version for your operating system. Then download the binary file **[MyAISToN2k.ino.esp32.bin](https://github.com/AK-Homberger/NMEA2000-AIS-Gateway/blob/main/MyAISToN2k.ino.esp32.bin)** to the PC.

Then start the NodeMCU PyFlaher program and select the downloaded [binary](https://github.com/AK-Homberger/NMEA2000-AIS-Gateway/blob/main/MyAISToN2k.ino.esp32.bin) and set the following options including the correct COM port:

![Esptool](https://github.com/AK-Homberger/NMEA2000-AIS-Gateway/blob/main/NodeMCUPyFlasher.png)

Then press "Flash NodeMCU". On some ESP32 it might be nessessary to press the "BOOT" button to start uploading.

## Arduino IDE

To install the [software](https://github.com/AK-Homberger/NMEA2000-AIS-Gateway/tree/main/MyAISToN2k) with the Arduino IDE you have to install the IDE and the ESP32 board support.

To use the gateway the following libraries have to be installed (as ZIP file):
- [NMEA2000](https://github.com/ttlappalainen/NMEA2000)
- [NMEA2000_esp32](https://github.com/ttlappalainen/NMEA2000_esp32)
- [NMEA0183](https://github.com/ttlappalainen/NMEA0183)

Click on the name of each repository, then click on "Code" and select "Download ZIP". Then in the Arduino IDE: Sketch, Include library, select Add .ZIP library and select the location of the downloaded libraries for each of the three libraries and install them one by one.

Then download the [repository](https://github.com/AK-Homberger/NMEA2000-AIS-Gateway/archive/refs/heads/main.zip) as ZIP file and upack it.
The Arduino code is in the "MyAISToN2k" folder. Open "MyAISToN2k.ino" in the Arduino IDE.

How to install and use the Arduino IDE is explained in the [NMEA2000 workshop](https://github.com/AK-Homberger/NMEA2000-Workshop).

## Hardware

![grafik](https://github.com/gerryvel/NMEA2000-AIS-Gateway/assets/17195231/8e712a91-7659-4c04-9c1b-aaa15964ecfc)
![grafik](https://github.com/gerryvel/NMEA2000-AIS-Gateway/assets/17195231/90d778f7-7d0f-4435-b715-f36132301e8c)
![image](https://github.com/user-attachments/assets/0f9a2b5e-f591-4115-a3f6-3465384aacb1)

The board can be ordered at Aisler.net: https://aisler.net/p/VYXKWDGC

## Parts:
You only need these parts for the gateway:

- Adafruit Huzzah! ESP32 (for programming need USB-Adapter)[Link](https://www.exp-tech.de/plattformen/internet-of-things-iot/9350/adafruit-huzzah32-esp32-breakout-board)
- SN65HVD230 [Link](https://www.reichelt.de/high-speed-can-transceiver-1-mbit-s-3-3-v-so-8-sn-65hvd230d-p58427.html?&trstct=pos_0&nbc=1)
- Traco-Power TSR 1-2450 for 12V / 5V [Link](https://www.reichelt.de/dc-dc-wandler-tsr-1-1-w-5-v-1000-ma-sil-to-220-tsr-1-2450-p116850.html?search=tsr+1-24)
- B360 [Link](https://www.reichelt.de/schottkydiode-60-v-3-a-do-214ab-smc-b-360-f-p95202.html?&trstct=pos_10&nbc=1)
- 1N4148 [Link](https://www.reichelt.de/schalt-diode-100-v-150-ma-do-35-1n-4148-dio-p219454.html?&nbc=1)
- BC847 [Link](https://www.reichelt.de/bipolartransistor-npn-45v-0-1a-0-25w-sot-23-bc-847a-smd-p18561.html?&nbc=1)


## Updates:
- 07.10.24 Version 0.9 LED cosmetic > LED init = RD/GN/BU on > N2k start GN 1s on, N2k fail RD 1s on, Loop GN flash
- 01.09.23 Version 0.81 Hardware Update, add ESD Components in 12V Input and CanOutput 
- 25.08.23 Version 0.8: Corrected esp_efuse_mac_get_default(chipid); and  (tN2kAISDTE) _dte, (tN2kAISTransceiverInformation) _ais_version);
- 13.01.22 Version 0.7: Select wether AIVDO messages (own ship) are to be decoded also in addition to AIVDM.
- 13.01.22 Version 0.6: Corrected time PGNs for ETA calculation and changed AIS PGN handling for Raymarine MFDs.
- 01.03.21 Version 0.5: Send message type 19 as PGN129040 (instead of 18 and 24a/24b PGNs).
- 20.02.21 Version 0.4: Added message type 14 support.
- 19.02.21 Version 0.3: Added message type 19 support.
- 19.02.21 Version 0.2: Added ETA calculation for message 5.
- 18.02.21 Version 0.1: Initial version.
