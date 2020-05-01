/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <Throttle Serial class>
*************************************************************/

#include "DCCpp.h"

#if defined(USE_TEXTCOMMAND) && defined(USE_THROTTLES)
ThrottleSerial::ThrottleSerial(const String& inName, SerialInterface* inpInterface) : Throttle(inName)
{
	this->pInterface = inpInterface;
}

bool ThrottleSerial::begin()
{
	return true;
}

bool ThrottleSerial::loop()
{
	bool added = false;

	// Serial comm always active...
	while (this->pInterface->available() > 0) 
	{    // while there is data on the serial line
		added = Throttle::getCharacter(this->pInterface->read(), this);
	} // while

	return added;
}

bool ThrottleSerial::sendMessage(const String& inMessage)
{
	this->pInterface->println(inMessage);
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
	Serial.print(this->id);
	Serial.print(" : ");
	Serial.print(this->name);
	if (this->pConverter != NULL)
	{
		Serial.print(" (");
		this->pConverter->printConverter();
		Serial.print(") ");
	}

	Serial.println("");
}
#endif
#endif