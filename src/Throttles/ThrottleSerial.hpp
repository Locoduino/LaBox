//-------------------------------------------------------------------
#ifndef __ThrottleSerial_hpp__
#define __ThrottleSerial_hpp__
//-------------------------------------------------------------------

#ifdef USE_THROTTLES
#include "DCCpp.h"

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
	ThrottleSerial(const char* inName, SerialInterface* inpInterface);

	bool begin();
	bool receiveMessages();
	bool sendMessage(const char *);
	void end();
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