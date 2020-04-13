/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <Throttle Serial class>
*************************************************************/

#include "DCCpp.h"

#ifdef USE_THROTTLES
ThrottleSerial::ThrottleSerial(const char* inName, SerialInterface* inpInterface) : Throttle(inName)
{
	this->pInterface = inpInterface;
}

bool ThrottleSerial::begin()
{
	return true;
}

bool ThrottleSerial::receiveMessages()
{
	char commandString[MAX_COMMAND_LENGTH + 1];
	char c;

	// Serial comm always active...
	while (this->pInterface->available() > 0) {    // while there is data on the serial line
		c = this->pInterface->read();
		if (c == '<')                    // start of new command
			commandString[0] = 0;
		else 
			if (c == '>')
			{// end of new command
				this->pushMessageInStack(this->id, commandString);
				return true;
			}
			else 
				if (strlen(commandString) < MAX_COMMAND_LENGTH)  // if commandString still has space, append character just read from serial line
					sprintf(commandString, "%s%c", commandString, c); // otherwise, character is ignored (but continue to look for '<' or '>')
	} // while

	return false;
}

bool ThrottleSerial::sendMessage(const char *pMessage)
{
	this->pInterface->println(pMessage);
	return true;
}

void ThrottleSerial::end()
{
}

bool ThrottleSerial::isConnected()
{
	return true;
}

#ifdef DCCPP_DEBUG_MODE
void ThrottleSerial::printThrottle()
{
	Serial.print(this->name);
	Serial.print(" id:");
	Serial.print(this->id);

	Serial.println("");
}
#endif
#endif