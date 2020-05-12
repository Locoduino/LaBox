/*************************************************************
project: <LaBox>
author: <Thierry PARIS>
description: <LaBox Wifi Controller sample>
*************************************************************/

#include "LaBox.h"

#if !defined(USE_TEXTCOMMAND) || !defined(USE_WIFI)
#error To be able to compile this sample,the lines #define USE_TEXTCOMMAND and #define USE_WIFI_EXTERNSSID or USE_WIFI_LOCALSSID must be uncommented in DCCpp.h
#endif

// WIFI

#ifdef USE_WIFI_EXTERNSSID
const char* ssid = "VIDEOFUTUR_C56165_2.4G";
const char* password = "EenPghQD";
#endif
#ifdef USE_WIFI_LOCALSSID
const char* ssid = "LaBoxServer";
const char* password = "";
#endif

// the media access control (ethernet hardware) address for the shield:
//uint8_t wifiMac[] = { 0xBE, 0xEF, 0xBE, 0xEF, 0xBE, 0x80 };
//the IP address for the shield:
//uint8_t wifiIp[] = { 192, 168, 1, 100 };
int dccPPPort = 10000;
int wifiPort = 44444;

// SERIAL
 
//SERIAL_INTERFACE(Serial, Normal);

//ThrottleSerial throttleSerial("Serial", new SerialInterfaceNormal());
ThrottleWifi throttleWifi0("DCCpp", dccPPPort);

ThrottleWifi throttleWifi1("Z21 - 1", Z21_UDPPORT);
MessageConverterZ21 converter1;
ThrottleWifi throttleWifi2("Z21 - 2", Z21_UDPPORT);
MessageConverterZ21 converter2;
ThrottleWifi throttleWifi3("Z21 - 3", Z21_UDPPORT);
MessageConverterZ21 converter3;

ThrottleWifi throttleWifi4("WiThrottle - 1", wifiPort);
MessageConverterWiThrottle converterWT1;
ThrottleWifi throttleWifi5("WiThrottle - 2", wifiPort);
MessageConverterWiThrottle converterWT2;
ThrottleWifi throttleWifi6("WiThrottle - 3", wifiPort);
MessageConverterWiThrottle converterWT3;

void setup()
{
  Serial.begin(115200);
  Serial.println("LaBox 0.5");

  ThrottleWifi::connectWifi(ssid, password);
  
  throttleWifi0.begin(TCP);
  
  throttleWifi1.setConverter(&converter1);
  throttleWifi1.begin(UDP);
  throttleWifi2.setConverter(&converter2);
  throttleWifi2.begin(UDP);
  throttleWifi3.setConverter(&converter3);
  throttleWifi3.begin(UDP);
  
  throttleWifi4.setConverter(&converterWT1);
  throttleWifi4.begin(TCP);
  throttleWifi5.setConverter(&converterWT2);
  throttleWifi5.begin(TCP);
  throttleWifi6.setConverter(&converterWT3);
  throttleWifi6.begin(TCP);

  DCCpp::begin();
  /* Configuration for ESP32, can be adapted...
  DIR -> GPIO_32
  PWM -> EN
  MAX471 -> GPIO_36 (A0)
  */
  DCCpp::beginMain(UNDEFINED_PIN, 32, 34, A0);  
}

void loop()
{
  DCCpp::loop();
}
