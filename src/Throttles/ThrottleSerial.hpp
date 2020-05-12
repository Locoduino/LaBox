//-------------------------------------------------------------------
#ifndef __ThrottleSerial_hpp__
#define __ThrottleSerial_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

#if defined(USE_TEXTCOMMAND) && defined(USE_THROTTLES)

/** This is a class to handle Serial communications.
*/
class ThrottleSerial : public Throttle
{
private:
	SerialInterface* pInterface;

public:
	/** Creates a new instance of this interface.
	@param inName	throttle new name.
	@param inpInterface	Serial canal for communication.
	*/
	ThrottleSerial(const String& inName, SerialInterface* inpInterface);

	bool begin(EthernetProtocol inProtocol);
	void end();
	bool loop();
	bool sendMessage(const String& inMessage);
	bool isConnected();

#ifdef DCCPP_DEBUG_MODE
	/** Print the status of the Throttle.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	void printThrottle();
#endif
};

#endif
#endif