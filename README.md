# NMEA0183 to NMEA2000 AIS Gateway

This repository shows how to build a gateway to read NMEA0183 AIS messages and transform them to NMEA2000 PGNs.

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
# Software Install
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

# Hardware

![grafik](https://github.com/gerryvel/NMEA2000-AIS-Gateway/assets/17195231/8baad071-a646-4ba3-8f42-7a44998f798f)
![grafik](https://github.com/gerryvel/NMEA2000-AIS-Gateway/assets/17195231/652a8870-f99c-44ea-b9c8-74fbf894f9f1)

The board can be ordered at Aisler.net: https://aisler.net/p/VYXKWDGC

## Parts:
You only need these parts for the gateway:


# Updates:
- 13.01.22 Version 0.7: Select wether AIVDO messages (own ship) are to be decoded also in addition to AIVDM.
- 13.01.22 Version 0.6: Corrected time PGNs for ETA calculation and changed AIS PGN handling for Raymarine MFDs.
- 01.03.21 Version 0.5: Send message type 19 as PGN129040 (instead of 18 and 24a/24b PGNs).
- 20.02.21 Version 0.4: Added message type 14 support.
- 19.02.21 Version 0.3: Added message type 19 support.
- 19.02.21 Version 0.2: Added ETA calculation for message 5.
- 18.02.21 Version 0.1: Initial version.
