//-------------------------------------------------------------------
#ifndef __ThrottleEthernet_hpp__
#define __ThrottleEthernet_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

#if !defined(ARDUINO_ARCH_ESP32)
#if defined(USE_TEXTCOMMAND) && defined(USE_THROTTLES) && defined(USE_ETHERNET)

#ifdef USE_ETHERNET_WIZNET_5100
#define COMM_SHIELD_NAME      "ARDUINO-CC ETHERNET SHIELD (WIZNET 5100)"
#include <Ethernet.h>         // built-in Arduino.cc library

#elif defined(USE_ETHERNET_WIZNET_5500)
#define COMM_SHIELD_NAME      "ARDUINO-ORG ETHERNET-2 SHIELD (WIZNET 5500)"
#include <Ethernet2.h>        // https://github.com/arduino-org/Arduino

#elif defined(USE_ETHERNET_WIZNET_5200)
#define COMM_SHIELD_NAME      "SEEED STUDIO ETHERNET SHIELD (WIZNET 5200)"
#include <EthernetV2_0.h>     // https://github.com/Seeed-Studio/Ethernet_Shield_W5200

#elif defined(USE_ETHERNET_ENC28J60)
#define COMM_SHIELD_NAME      "BASIC ETHERNET SHIELD (ENC28J60)"
#include <UIPEthernet.h>     // https://github.com/ntruchsess/arduino_uip/tree/Arduino_1.5.x

#endif

/** This is a class to handle an ethernet connection.
*/
class ThrottleEthernet : public Throttle
{
private:
	IPAddress ethernetIp;
	uint8_t ethernetMac[6];
	int port = 0;

	EthernetProtocol protocol = EthernetProtocol::TCP;
	EthernetServer *pServer;

public:
	/** Creates a new instance of this interface.
	@param inName	throttle new name.
	*/
	ThrottleEthernet(const String& inName, uint8_t* inMac, uint8_t* inIp, int inPort, EthernetProtocol inProtocol);

	bool begin(EthernetProtocol inProtocol = TCP);
	bool loop();
	bool sendMessage(const String& inMessage);
	void end();
	bool isConnected();
	bool sendNewline();
	IPAddress remoteIP();

#ifdef DCCPP_DEBUG_MODE
	/** Print the status of the Throttle.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	void printThrottle();
#endif
};

#endif
#endif
#endif
