//-------------------------------------------------------------------
#ifndef __MessageConverterWiThrottle_hpp__
#define __MessageConverterWiThrottle_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"
#include "WString.h"

#if defined(USE_THROTTLES) && defined(USE_WIFI)

#define HEARTBEATTIMEOUT 10
#define WICOMMANDSIZE	30
#define WIMAXLOCOSNUMBER	6
#define WI_TIMEOUT		60000		// if no activity during 1 minute, disconnect the throttle...

// Define turnout object structures
typedef struct
{
	int address;
	int id;
	int tStatus;
} tData;

/** This is a class to handle decoder functions.
An instance of this class handle the status of the functions of one decoder.
A function can be active or not.
*/
class MessageConverterWiThrottle : public MessageConverter
{
private:
	void locoAction(Throttle* inpThrottle, int inLocoNumber, const String& inLocoDescriptor, const String& inActionKey, const String& inActionVal);
	void accessoryToggle(int aAddr, String aStatus);

public:
	static String command;
#ifdef USE_TURNOUT
	static tData turnouts[512];
#endif

	bool heartbeatEnable;
	Locomotive* pLocos[WIMAXLOCOSNUMBER];
	unsigned long heartbeat[WIMAXLOCOSNUMBER];

	/** Creates a new instance for only one register.
	@param inName	throttle new name.
	*/
	MessageConverterWiThrottle();

	void clientStart(Throttle* inpThrottle);
	void clientStop(Throttle* inpThrottle);
	bool convert(Throttle* inpThrottle, const String& inCommand);
	void stayAlive(Throttle* inpThrottle);
	EthernetProtocol getProtocol() { return TCP; }

	void locoAdd(Throttle* inpThrottle, int inLocoNumber, String th, String actionKey);
	void locoRelease(Throttle* inpThrottle, int inLocoNumber, String th, String actionKey);
	void checkHeartbeat(Throttle* inpThrottle);

#ifdef DCCPP_DEBUG_MODE
	/** Print the status of the Converter.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	void printConverter();
#endif
};

#endif
#endif