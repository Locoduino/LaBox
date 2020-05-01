//-------------------------------------------------------------------
#ifndef __Locomotives_hpp__
#define __Locomotives_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

#include "FunctionsState.hpp"
#include "Locomotive.hpp"
#include "Registers.hpp"

/**	This is the static class for assigned locomotive list.
*/
class Locomotives
{
private:
	static Locomotive* pFirstLocomotive;	/**< Address of the first object of this class. NULL means empty list of Locomotives. Do not change it !*/

public:
	/** Get a particular Locomotive.
	@param inAddress	The DCC id (1-65535) of the locomotive.
	@return The found locomotive or NULL if not found.
	*/
	static Locomotive* get(uint16_t inAddress);
	/** Adds a new locomotive.
	@param inName	Locomotive new name.
	@param inAddress	Locomotive new DCC address.
	@param inSpeedMax	Locomotive max speed steps, default is 128.
	@return pointer on the new locomotive or NULL if no place available.
	*/
	static Locomotive* add(const String& inName, uint16_t inAddress, uint8_t inSpeedMax = 128);
	/** Removes a particular locomotive.
	@param inAddress	The DCC id (1-65535) of the locomotive.
	*/
	static void remove(uint16_t inAddress);
	/** Get a particular Locomotive.
	@param inName	The name of the locomotive.
	@return The found locomotive or NULL if not found.
	*/
	static Locomotive* get(const String& inName);
	/** Removes a particular locomotive.
	@param inName	The name of the locomotive.
	*/
	static void remove(const String& inName);
	/** Get the maximum number of locomotives.
	@return Number of locomotives.
	*/
	static int countMax() {	return (MAX_MAIN_REGISTERS - 1) % 2; }
	/** Get the total number of assigned locomotives.
	@return Number of locomotives.
	*/
	static int count();

#ifdef DCCPP_DEBUG_MODE
	/** Print the list of assigned locomotives.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	static void printLocomotives();
#endif
};
#endif