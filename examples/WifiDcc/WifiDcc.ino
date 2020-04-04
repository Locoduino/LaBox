/*************************************************************
project: <LaBox>
author: <Thierry PARIS>
description: <LaBox Wifi Controller sample>
*************************************************************/

#include "LaBox.h"

#if !defined(USE_TEXTCOMMAND) || !defined(USE_WIFI)
#error To be able to compile this sample,the lines #define USE_TEXTCOMMAND and #define USE_WIFI must be uncommented in DCCpp.h
#endif

const char* ssid     = "VIDEOFUTUR_C56165_2.4G";
const char* password = "EenPghQD";

WiFiServer DCCPP_INTERFACE(2560);

void setup()
{
  Serial.begin(115200);
  Serial.println("LaBox 0.1");

  DCCpp::beginWifi(ssid, password, EthernetProtocol::TCP);
  
  DCCpp::begin();
  /* Configuration for ESP32, can be adapted...
  DIR -> GPIO_32
  PWM -> EN
  MAX471 -> GPIO_36 (A0)
  */
  DCCpp::beginMain(UNDEFINED_PIN, 32, UNDEFINED_PIN, A0);  
}

void loop()
{
  DCCpp::loop();
}
