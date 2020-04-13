//-------------------------------------------------------------------
#ifndef __ThrottleWifi_hpp__
#define __ThrottleWifi_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

#if defined(USE_THROTTLES) && defined(USE_WIFI)

#include <WiFi.h>

/** This is a class to handle wifi communications.
*/
class ThrottleWifi : public Throttle
{
private:
	uint8_t wifiIp[4];
	uint8_t wifiIpMac[6];
	int port = 0;
	EthernetProtocol protocol = EthernetProtocol::TCP;

	WiFiServer *pServer;

public:
	static void connectWifi(const char* inSsid, const char* inPassword);

	/** Creates a new instance of this wifi class.
	@param inName	throttle new name.
	*/
	ThrottleWifi(const char* inName,uint8_t* inMac, uint8_t* inIp, int inPort, EthernetProtocol inProtocol);

	bool begin();
	bool receiveMessages();
	bool sendMessage(const char *);
	void end();
	bool isConnected();
	bool sendNewline();

#ifdef DCCPP_DEBUG_MODE
	/** Print the status of the Throttle.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	void printThrottle();
#endif
};

#endif
#endif