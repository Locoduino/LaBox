/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <Throttle Wifi variant for JMRI class>
*************************************************************/

#include "DCCpp.h"

#if defined(USE_TEXTCOMMAND) && defined(USE_THROTTLES) && defined(USE_WIFI)
ThrottleWifiJMRI::ThrottleWifiJMRI(int inPort) : ThrottleWifi("JMRI", inPort)
{
}

bool ThrottleWifiJMRI::begin(EthernetProtocol inProtocol)
{
	return ThrottleWifi::begin(inProtocol);
}

#ifdef DCCPP_DEBUG_MODE
void ThrottleWifiJMRI::printThrottle()
{
	Serial.print(this->id);
	Serial.print(" : ");
	Serial.print(this->name);

	Serial.println("");
}
#endif
#endif