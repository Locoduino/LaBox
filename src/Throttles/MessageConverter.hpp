//-------------------------------------------------------------------
#ifndef __MessageConverter_hpp__
#define __MessageConverter_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

#if defined(USE_THROTTLES)

class Throttle;

/** This is a class to handle decoder functions.
An instance of this class handle the status of the functions of one decoder.
A function can be active or not.
*/
class MessageConverter
{
public:
	/** Creates a new instance for only one register.
	@param inName	throttle new name.
	*/
	MessageConverter();

	virtual void clientStart(Throttle* inpThrottle) {}
	virtual void clientStop(Throttle* inpThrottle) {}
	virtual bool convert(Throttle* inpThrottle, const String& inCommand) = 0;
	virtual void stayAlive(Throttle* inpThrottle) {}

#ifdef DCCPP_DEBUG_MODE
	/** Print the status of the Converter.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	virtual void printConverter() = 0;
#endif
};

#endif
#endif