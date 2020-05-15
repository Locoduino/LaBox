//-------------------------------------------------------------------
#ifndef __Throttles_hpp__
#define __Throttles_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

#if defined(USE_THROTTLES)

/**	This is the static class for assigned Throttle list.
*/
class Throttles
{
private:
	static Throttle* pFirstThrottle;	/**< Address of the first object of this class. NULL means empty list of Locomotives. Do not change it !*/
	static uint16_t throttlesCounter;	/**< When a throttle is created, affect this number as its id, and increments the counter. No id must be reused ! */

public:
	/** Get the first Throttle.
	@return The first throttle of the linked list throttle or NULL if the list is empty.
	*/
	static Throttle* getFirst();
	/** Get a particular Throttle.
	@param inId	The throttle id.
	@return The found throttle or NULL if not found.
	*/
	static Throttle* get(uint16_t inId);
	/** Adds a new throttle.
	@param inpThrottle	throttle to add.
	@return true if the throttle is registerted or NULL if no place available.
	@remark Id is automatically affected from the current value of throttleCounter.
	*/
	static bool add(Throttle *inpThrottle);
	/** Removes a particular throttle.
	@param inId	The id of the throttle to remove.
	*/
	static void remove(uint16_t inId);
	/** Get a particular throttle.
	@param inName	The name of the throttle.
	@return The found throttle or NULL if not found.
	*/
	static Throttle* get(const String& inName);
	/** Get a particular throttle.
	@param inRemoteIP		remote ip address to find.
	@return The found throttle or NULL if not found.
	*/
	static Throttle* get(IPAddress inRemoteIP);
	/** Removes a particular throttle.
	@param inName	The name of the throttle.
	*/
	static void remove(const String& inName);
	/** Get the total number of assigned throttles.
	@return Number of throttles.
	*/
	static int count();

#ifdef DCCPP_DEBUG_MODE
	/** Print the list of assigned throttle.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	static void printThrottles();
#endif
};
#endif
#endif