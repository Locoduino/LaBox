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
uint8_t wifiMac[] = { 0xBE, 0xEF, 0xBE, 0xEF, 0xBE, 0x80 };
//the IP address for the shield:
uint8_t wifiIp[] = { 192, 168, 1, 100 };
//WiFiServer DCCPP_INTERFACE(2560);

// SERIAL
 
SERIAL_INTERFACE(Serial, Normal);

//ThrottleSerial throttleSerial("Serial", new SerialInterfaceNormal());
//ThrottleWifi throttleWifi("Wifi", wifiMac, wifiIp, 2560, TCP);

void setup()
{
  Serial.begin(115200);
  Serial.println("LaBox 0.5");

  //ThrottleWifi::connectWifi(ssid, password);
  
  // Add 4 Wifi clients
  ThrottleWifiAPClient::connectWifi(ssid, password, 2560, 4);
  ThrottleWifiAPClient* pCurr = ThrottleWifiAPClient::pFirstClient;

  for (int i = 0; i < ThrottleWifiAPClient::numberOfClients; i++)
  {
    MessageConverterWiThrottle* pConverter = new MessageConverterWiThrottle();
    pCurr->setConverter(pConverter);
    pCurr = (ThrottleWifiAPClient*)pCurr->pNextThrottle;
  }

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
