//-------------------------------------------------------------------
#ifndef __ThrottleWifi_hpp__
#define __ThrottleWifi_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

#if defined(USE_TEXTCOMMAND) && defined(USE_THROTTLES) && defined(USE_WIFI)

#include <WiFi.h>
#include <WiFiUdp.h>
#include <WiFiAP.h>

#define UDP_BUFFERSIZE	256

/** This is a class to handle wifi communications.
*/
class ThrottleWifi : public Throttle
{
private:
	static IPAddress wifiIp;
	static WiFiUDP _ClientUDP;


	int port = 0;
	enum EthernetProtocol protocol = EthernetProtocol::TCP;

	IPAddress remoteIp;
	
	WiFiServer *pServer;
	WiFiClient client;
	WiFiUDP ClientUDP;

public:
	static void connectWifi(const char* inSsid, const char* inPassword);
	static ThrottleWifi* GetThrottle(IPAddress inRemoteIp, int inPort, EthernetProtocol inProtocol);

	/** Creates a new instance of this wifi class.
	@param inName	throttle new name.
	*/
	ThrottleWifi(const String& inName, int inPort);

	bool begin(EthernetProtocol inProtocol = TCP);
	bool loop();
	bool sendMessage(const String& inMessage);
	void end();
	bool isConnected();
	bool sendNewline();
	CircularBuffer* getCircularBuffer() const;
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