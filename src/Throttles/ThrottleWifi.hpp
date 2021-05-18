//-------------------------------------------------------------------
#ifndef __ThrottleWifi_hpp__
#define __ThrottleWifi_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

#if defined(USE_TEXTCOMMAND) && defined(USE_THROTTLES) && defined(USE_WIFI)

#define TYPEWIFI	'W'

#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiAP.h>

#define UDP_BUFFERSIZE	256

/** This is a class to handle wifi communications.
*/
class ThrottleWifi : public Throttle
{
protected:
	static IPAddress wifiIp;
	static WiFiUDP _ClientUDP;
	static bool socketsStarted;

	int port = 0;
	enum EthernetProtocol protocol = EthernetProtocol::TCP;

	IPAddress remoteIp;
	
	WiFiServer* pServer;
	WiFiClient client;
	WiFiUDP ClientUDP;

public:
	static void connectWifi(const char* inSsid, const char* inPassword, const IPAddress &inIp = INADDR_NONE, const IPAddress &inGateway = INADDR_NONE, const IPAddress &insubnet = INADDR_NONE, const IPAddress &inDns = INADDR_NONE);
	static ThrottleWifi* GetThrottle(IPAddress inRemoteIp, int inPort, EthernetProtocol inProtocol);

	/** Creates a new instance of this wifi class.
	@param inName	throttle name.
	@param inPort	Communication port.
	*/
	ThrottleWifi(const String& inName, int inPort);

	bool begin(EthernetProtocol inProtocol = TCP);
	bool begin(MessageConverter *apConverter);
	bool loop();
	bool sendMessage(const String& inMessage);
	void end();
	bool isConnected();
	bool SendNewline() const;
	void write(byte* inpData, int inLengthData);
	bool readUdpPacket(ThrottleWifi* inpThrottle);
	IPAddress remoteIP();
	void setRemoteIP(IPAddress inAddress) { this->remoteIp = inAddress; }

#ifdef DCCPP_DEBUG_MODE
	/** Print the status of the Throttle.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	void printThrottle();
#endif
};

#endif
#endif