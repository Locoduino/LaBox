/**********************************************************************

SerialCommand.h
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#ifndef TextCommand_h
#define TextCommand_h

#include "DCCpp.h"

#ifdef USE_THROTTLES
#include "LaBox.h"
#endif

/**	\defgroup commandsGroup Text Commands Syntax
*/

#ifdef USE_TEXTCOMMAND

#include "PacketRegister.h"
#include "CurrentMonitor.h"

#define  MAX_COMMAND_LENGTH         30

/** DCC++ BASE STATION COMMUNICATES VIA THE SERIAL PORT USING SINGLE-CHARACTER TEXT COMMANDS
WITH OPTIONAL PARAMETERS, AND BRACKETED BY < AND > SYMBOLS.  SPACES BETWEEN PARAMETERS
ARE REQUIRED.  SPACES ANYWHERE ELSE ARE IGNORED.  A SPACE BETWEEN THE SINGLE-CHARACTER
COMMAND AND THE FIRST PARAMETER IS ALSO NOT REQUIRED.*/
struct TextCommand{
#if defined(ARDUINO_ARCH_ESP32)
  static void receiveCommands();
#endif

  static char commandString[MAX_COMMAND_LENGTH+1];
  static void init(volatile RegisterList *, volatile RegisterList *, CurrentMonitor *);
  static bool parse(char *);
  static void process();

#ifdef USE_THROTTLES
  static Throttle* pCurrentThrottle;
#endif
}; // TextCommand
  
#ifdef USE_THROTTLES
#define DCCPP_INTERFACE	(*TextCommand::pCurrentThrottle)
#else
#define DCCPP_INTERFACE	Serial
#endif

#endif
#endif
