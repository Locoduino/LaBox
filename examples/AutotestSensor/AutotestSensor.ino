/*************************************************************
project: <LaBox Controller>
author: <Thierry PARIS>
description: <Dcc Automatic Controller sample>
*************************************************************/

/**
* The goal of this sample is to check if the DCC is working.
*
* The main loop function loops through the StateMachineItems linked list and
* send a command for a given delay. When the delay is expired, it pass to the
* next item. If no command is given, the delay is still taken.
* For instance to let the locomotive stop smoothly...
*
* This sample is made for a DCC address 3, a speed of 50 from 128 during rolling,
* and delays according to my test loco. Your machine could have different reactions...
*/

#include "LaBox.h"
#include "string.h"

#if !defined(USE_TEXTCOMMAND) || !defined(USE_THROTTLES) || !defined(USE_SENSOR)
#error To be able to compile this sample, the lines USE_TEXTCOMMAND and USE SENSOR must be uncommented in DCCpp.h
#endif

//--------------------------- HMI client -------------------------------------
#ifdef USE_HMI
hmi boxHMI(&Wire);
#endif
//----------------------------------------------------------------------------

Sensor sens;
ThrottleAutomation automation("Automation");

void setup()
{
  Serial.begin(115200);

  Serial.print("LaBox / Automation sample ");
  Serial.println(LABOX_LIBRARY_VERSION);

  //----------- Start HMI -------------
#ifdef USE_HMI
  boxHMI.begin();
#endif

  sens.begin(25, 12, true);
  automation.begin();

  automation.AddItem(0, "t 1 3 0 1", "ON");       // On register 1, set direction to forward on cab 3
  automation.AddItem(0, "f 3 144", "Light on");     // Light FL (F0) on
  automation.AddItem(3000, "t 1 3 50 1", "Forward");  // On register 1, go forward at speed 30 on cab 3 for 3s
  automation.AddItem(1000, "t 1 3 0 1", "Stop");    // Stop cab 3 after 1 second
  automation.AddItem(AUTOMATIONSENSORID(25, HIGH), "", "Wait");    // Wait complete stop until pin 12 reach HIGH state
  automation.AddItem(1000, "t 1 3 0 0", "To bwd");    // On register 1, set direction to backward on cab 3
  automation.AddItem(3000, "t 1 3 50 0", "Backward"); // On register 1, go backward at speed 30 on cab 3
  automation.AddItem(1000, "t 1 3 0 0", "Stop");    // Stop cab 3 after 1 second
  automation.AddItem(2000, "", "Wait");         // Wait complete stop
  automation.AddItem(1000, "t 1 3 0 1", "To fwd");    // On register 1, set direction to forward on cab 3
  automation.AddItem(500, "f 3 128", "Light off");    // Light off : blink three times
  automation.AddItem(500, "f 3 144", "Light on");   // Light FL (F0) on
  automation.AddItem(500, "f 3 128", "Light off");    // Light off
  automation.AddItem(500, "f 3 144", "Light on");   // Light on
  automation.AddItem(500, "f 3 128", "Light off");    // Light off
  automation.AddItem(500, "f 3 144", "Light on");   // Light on
  automation.AddItem(500, "f 3 128", "Light off");    // Light off

  DCCpp::begin();
  // configuration pour L6203 La Box
  DCCpp::beginMain(UNDEFINED_PIN, 33, 32, 36);  

  DCCpp::powerOn();

  // for test only...
  automation.printThrottleItems();
  
  automation.Start();
}

void loop()
{
#ifdef USE_HMI
  boxHMI.update();
#endif

  DCCpp::loop();
}
