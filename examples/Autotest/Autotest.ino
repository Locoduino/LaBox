/*************************************************************
project: <Dc/Dcc Controller>
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

#ifndef USE_TEXTCOMMAND
#error To be able to compile this sample,the line #define USE_TEXTCOMMAND must be uncommented in DCCpp.h
#endif

//--------------------------- HMI client -------------------------------------
hmi boxHMI(&Wire);
//----------------------------------------------------------------------------

ThrottleAutomation automation("Automation");

void setup()
{
  Serial.begin(115200);

  Serial.print("LaBox / Automation sample ");
  Serial.println(LABOX_LIBRARY_VERSION);

  //----------- Start HMI -------------
  boxHMI.begin();

  automation.begin();

  automation.AddItem(0, "t 1 3 0 1", "ON");       // On register 1, set direction to forward on cab 3
  automation.AddItem(0, "f 3 144", "Light on");     // Light FL (F0) on
  automation.AddItem(3000, "t 1 3 50 1", "Forward");  // On register 1, go forward at speed 30 on cab 3 for 3s
  automation.AddItem(1000, "t 1 3 0 1", "Stop");    // Stop cab 3 after 1 second
  automation.AddItem(2000, "", "Wait");         // Wait complete stop 2s
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
  // Configuration for my LMD18200. See the page 'Configuration lines' in the documentation for other samples.
  DCCpp::beginMain(UNDEFINED_PIN, 32, UNDEFINED_PIN, 34);

  DCCpp::powerOn();

  automation.printThrottleItems();
  
  automation.Start();
}

void loop()
{
  boxHMI.update();

  DCCpp::loop();
}
