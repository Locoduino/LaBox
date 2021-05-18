//-------------------------------------------------------------------
#ifndef __ThrottleWifiWebServer_hpp__
#define __ThrottleWifiWebServer_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

#if defined(USE_TEXTCOMMAND) && defined(USE_THROTTLES) && defined(USE_WIFI)

/** This is a class to handle HTTP Web Server communications.
*/
class ThrottleWifiWebServer : public ThrottleWifi
{
public:
	/** Creates a new instance of this interface.
	@param inName	Throttle name.
	@param inPort	Communication port.
	*/
	ThrottleWifiWebServer(const String& inName, int inPort = 80);

	void sendHTML();

	bool begin(EthernetProtocol inProtocol = TCP);
	bool pushMessage(const String& inMessage);

	static String* URLEncode(const String& inCommand, String* pBuffer);
	static String* URLDecode(const String& inCommand, String* pBuffer);

#ifdef DCCPP_DEBUG_MODE
	/** Print the status of the Throttle.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	void printThrottle();
#endif
};

#endif
#endif