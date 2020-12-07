//-------------------------------------------------------------------
#ifndef __ThrottleWifiJMRI_hpp__
#define __ThrottleWifiJMRI_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

#if defined(USE_TEXTCOMMAND) && defined(USE_THROTTLES) && defined(USE_WIFI)

/** This is a class to handle Serial communications.
*/
class ThrottleWifiJMRI : public ThrottleWifi
{
public:
	/** Creates a new instance of this interface.
	@param inName	Wifi port.
	*/
	ThrottleWifiJMRI(int inPort = 2560);

	bool begin(EthernetProtocol inProtocol = TCP);

#ifdef DCCPP_DEBUG_MODE
	/** Print the status of the Throttle.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	void printThrottle();
#endif
};

#endif
#endif