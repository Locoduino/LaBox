//-------------------------------------------------------------------
#ifndef __Locomotives_hpp__
#define __Locomotives_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

#include "Locomotive.hpp"

typedef void (*notifySpeedDirType)(uint16_t inLocoAddress, uint8_t inSpeed, bool inIsForward);
typedef void (*notifyFunctionType)(uint16_t inLocoAddress, uint8_t inFunction, bool inIsActivated);
typedef void (*notifyEmergencyStopType)(uint16_t inLocoAddress, bool inIsActivated);

/**	This is the static class for assigned locomotive list.
*/
class Locomotives
{
private:
	static Locomotive* pFirstLocomotive;	/**< Address of the first object of this class. NULL means empty list of Locomotives. Do not change it !*/

public:

	static notifySpeedDirType notifySpeedDir;
	static notifyFunctionType notifyFunction;
	static notifyEmergencyStopType notifyEmergencyStop;

	/** Get a particular Locomotive.
	@param inAddress	The DCC id (1-65535) of the locomotive.
	@return The found locomotive or NULL if not found.
	*/
	static Locomotive* get(uint16_t inAddress);
	/** Get a particular Locomotive.
	@param inAddress	The register number [MAIN_REGISTER_FIRSTCONTINUAL, MAX_MAIN_REGISTERS] of the locomotive.
	@return The found locomotive or NULL if not found.
	*/
	static Locomotive* getByRegister(int8_t inRegister);
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
	/** Get the pointer of the first locomotive.
	@return Start of the linked list of locomotives.
	*/
	static Locomotive* getFirstLocomotive() { return pFirstLocomotive; }
	/** Get the pointer of the next locomotive.
	@return The next locomotive in the chained list. If this locomotive does not anymore exists in the list, restart from the first one.
	*/
	static Locomotive* getNextLocomotive(Locomotive *inCurrent);

	/* Send a DCC command to stop the locmotive IMMEDIATELY
	*/
	static void emergencyStop();

	/* Save all locomotives data in JSON format.
	*/
	static bool SaveAll();

#ifdef DCCPP_DEBUG_MODE
	/** Print the list of assigned locomotives.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	static void printLocomotives();

	/** Build a fake list of locomotives, only used for tests.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	static void debugLocomotivesSet();
#endif
};
#endif