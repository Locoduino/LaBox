//-------------------------------------------------------------------
#ifndef __ThrottleWifiAPClient_hpp__
#define __ThrottleWifiAPClient_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

#if defined(USE_TEXTCOMMAND) && defined(USE_THROTTLES) && defined(USE_WIFI)

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>
#include <WiFiAP.h>

#define WIFIAP_MAXCLIENTS		4

/** This is a class to handle wifi communications through an access point (AP) defined by the ESP32.
*/
class ThrottleWifiAPClient : public Throttle
{
private:
	static uint8_t wifiServerIp[4];
	static int port;

	uint8_t wifiIp[4];
	bool connected;

protected:
	static WiFiMulti* pWifiMulti;
	static WiFiServer* pServer;
	static EthernetProtocol	protocol;

	WiFiClient Client;
	WiFiUDP ClientUDP;

public:
	static ThrottleWifiAPClient* pFirstClient;
	static int numberOfClients;
	static void connectWifi(const char* inSsid, const char* inPassword, int inPort = 1001, int inClientsNumber = 2, EthernetProtocol inProtocol = TCP);

	/** Creates a new instance of this wifi class.
	@param inName	throttle new name.
	*/
	ThrottleWifiAPClient(const String& inName);

	bool begin();
	bool loop();
	bool sendMessage(const String& inMessage);
	void end();
	bool isConnected();
	bool sendNewline();

#ifdef DCCPP_DEBUG_MODE
#ifdef DCCPP_PRINT_DCCPP
	/** Print the status of the Throttle.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	void printThrottle();
#endif
#endif
};

#endif
#endif