//-------------------------------------------------------------------
#ifndef __ThrottleWifiAPClient_hpp__
#define __ThrottleWifiAPClient_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

#if defined(USE_THROTTLES) && defined(USE_WIFI)

#include <WiFi.h>
#include <WiFiMulti.h>

#define WIFIAP_MAXCLIENTS		4

/** This is a class to handle wifi communications through an access point (AP) defined by the ESP32.
*/
class ThrottleWifiAPClient : public Throttle
{
private:
	static WiFiMulti *pWifiMulti;
	static uint8_t wifiServerIp[4];
	static WiFiServer* pServer;
	static int port;
	static ThrottleWifiAPClient* pFirstClient;
	static int numberOfClients;

	uint8_t wifiIp[4];

	WiFiClient Client;

public:
	static void connectWifi(const char* inSsid, const char* inPassword, int inPort = 23, int inClientsNumber = 2);

	/** Creates a new instance of this wifi class.
	@param inName	throttle new name.
	*/
	ThrottleWifiAPClient(const char* inName);

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