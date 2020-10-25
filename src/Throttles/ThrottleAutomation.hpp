//-------------------------------------------------------------------
#ifndef __ThrottleAutomation_hpp__
#define __ThrottleAutomation_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

#if defined(USE_TEXTCOMMAND) && defined(USE_THROTTLES)

enum AutomationState
{
	stopped,	// All stopped. If start is wanted, start from the first item.
	started,	// At work...
	paused,		// Temporary stopped. Will restart from the current item.
};

struct ThrottleAutomationItem
{
	int delay;			// delay for 'this' command
	char command[20];
	char comment[20];
	ThrottleAutomationItem* next;

	ThrottleAutomationItem(int inDelay, const char* inCommand, const char* inComment);
};

/** This is a class to handle an automatic cycle of DCC commands.
*/
class ThrottleAutomation : public Throttle
{
private:
	ThrottleAutomationItem* first = NULL;
	ThrottleAutomationItem* currentItem = NULL;
	unsigned long timeValue = 0;
	AutomationState currentState = stopped;

public:
	/** Creates a new instance of this interface.
	@param inName	throttle new name.
	*/
	ThrottleAutomation(const String& inName);

	ThrottleAutomationItem* GetLastItem();
	void AddItem(int inDelay, const char* inCommand, const char* inComment);
	bool begin(EthernetProtocol inProtocol = TCP);
	bool loop();
	void end();
	bool sendMessage(const String& inMessage);
	bool isConnected();

	void Start();
	void Stop();
	void Pause();
	void ReStart();

#ifdef DCCPP_DEBUG_MODE
	/** Print the status of the Throttle.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	void printThrottle();
	/** Print all the items of the Throttle.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	void printThrottleItems();
#endif
};

#endif
#endif
