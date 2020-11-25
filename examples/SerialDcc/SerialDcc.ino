/*************************************************************
project: <LaBox>
author: <Thierry PARIS>
description: <LaBox Serial Controller sample>
*************************************************************/

#include "LaBox.h"

#if !defined(USE_TEXTCOMMAND) || !defined(USE_WIFI)
#error To be able to compile this sample,the lines #define USE_TEXTCOMMAND and #define USE_WIFI_EXTERNSSID or USE_WIFI_LOCALSSID must be uncommented in DCCpp.h
#endif

//--------------------------- HMI client -------------------------------------
hmi boxHMI(&Wire);
//----------------------------------------------------------------------------

// SERIAL
 
SERIAL_INTERFACE(Serial, Normal);
ThrottleSerial throttleSerial("Serial", SERIAL_INTERFACE_INSTANCE(Normal));

void setup()
{
  Serial.begin(115200);

  Serial.print("LaBox ");
  Serial.println(LABOX_LIBRARY_VERSION);

  //----------- Start HMI -------------
  boxHMI.begin();
  
  throttleSerial.begin();

  DCCpp::begin();

  // configuration pour L6203 La Box
  DCCpp::beginMain(UNDEFINED_PIN, 33, 32, 36);  
}

void loop()
{
  boxHMI.update();
  DCCpp::loop();
}
