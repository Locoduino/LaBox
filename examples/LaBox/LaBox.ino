/*************************************************************
project: <LaBox>
author: <Thierry PARIS>
description: <LaBox Wifi Controller sample>
*************************************************************/

#include "LaBox.h"

#if !defined(USE_TEXTCOMMAND) || !defined(USE_WIFI)
#error To be able to compile this sample,the lines #define USE_TEXTCOMMAND and #define USE_WIFI must be uncommented in DCCpp.h
#endif

// WIFI

const char* ssid     = "ssid";
const char* password = "password";

// the media access control (ethernet hardware) address for the shield:
uint8_t wifiMac[] = { 0xBE, 0xEF, 0xBE, 0xEF, 0xBE, 0x80 };
//the IP address for the shield:
uint8_t wifiIp[] = { 192, 168, 1, 100 };
//WiFiServer DCCPP_INTERFACE(2560);

// SERIAL

SERIAL_INTERFACE(Serial, Normal);

ThrottleSerial throttleSerial("Serial", new SerialInterfaceNormal());
ThrottleWifi throttleWifi("Wifi", wifiMac, wifiIp, 2560, TCP);

void setup()
{
  Serial.begin(115200);
  Serial.println("LaBox 0.2");

  // Add 4 Wifi clients
  ThrottleWifiAPClient::connectWifi(ssid, password, 23, 4);

  Throttles::printThrottles();
  
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
