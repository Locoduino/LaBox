//-------------------------------------------------------------------
#ifndef __MessageConverter_hpp__
#define __MessageConverter_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

#if defined(USE_THROTTLES)

class Throttle;

/** This is a class to handle message conversion.
*		if the messages of a throttle must be translated, add a converter to this throttle !
*		Each Throttle must have its own Converter if necessary.
*/
class MessageConverter
{
public:
	/** Creates a new instance of this converter.
	*/
	MessageConverter();

	/** Start this converter.	Called only once a start.
	@param inpThrottle Converter owner.
	*/
	virtual void clientStart(Throttle* inpThrottle) {}
	/** Stop this converter. Called when a Throttle is closed, its converter is closed too.
	@param inpThrottle Converter owner.
	*/
	virtual void clientStop(Throttle* inpThrottle) {}
	/** If the converter works with text commands, parse the given string and call direct LaBox functions if needed.
	@param inpThrottle Converter owner.
	@param inCommand	string to parse.
	@return false if nothing has been done...
	*/
	virtual bool convert(Throttle* inpThrottle, const String& inCommand) { return false; }
	/** If the converter works with binary commands, translate the next binary buffer item and call direct LaBox functions if needed.
	@param inpThrottle Converter owner.
	@return false if nothing has been done...
	*/
	virtual bool processBuffer(Throttle* inpThrottle) { return false; }
	/** Do what it is necessary to keep the owner Throttle alive.
	@param inpThrottle Converter owner.
	*/
	virtual void stayAlive(Throttle* inpThrottle) {}
	/** Get the used protocol for this converter.
	*/
	virtual EthernetProtocol getProtocol() = 0;

#ifdef DCCPP_DEBUG_MODE
	/** Print the status of the Converter.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	virtual void printConverter() {}
#endif
};

#endif
#endif