//-------------------------------------------------------------------
#ifndef __Locomotive_hpp__
#define __Locomotive_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"
#include "WString.h"
#include "Functions.hpp"

#ifdef USE_LOCOMOTIVES
#include "ArduinoJson.h"
#include "Registers.hpp"

#define LOCOMOTIVE_NAME_SIZE	32

/** This is a class to handle a locomotive.
An instance of this class handle the status of one locomotive.
*/
class Locomotive
{
private:
	String name;			// User name of this locomotive
	uint16_t address;	// 10239 is the DCC norm maximum.	9983 for Digitrax, 9999 for NCE and Lenz.
	int8_t speedRegisterNumber;	// Speed DCCpp register number
	uint8_t speedMax;	// Maximum speed for this loco : 14, 28 or 128.

	uint8_t currentSpeed;
	bool forwardDirection;	// true if the locomotive go forward.
	
public:
	Locomotive* pNextLocomotive;					/**< Address of the next object of this class. NULL means end of the list of Locomotives. Do not change it !*/
	Functions functions;							/**< Current sate of all functions of this locomotive. */
	long tag;															/**< User data associated with this locomotive.*/

	/** Creates a new instance by default.
	*/
	Locomotive();

	/** Creates a new instance.
	@param inName	Locomotive new name.
	@param inSpeedRegister	Locomotive DCC++ register.
	@param inAddress	Locomotive DCC address.
	@param inSpeedMax	Locomotive max speed steps, default is 128.
	*/
	Locomotive(const String& inName, uint8_t speedRegister, uint16_t inAddress = 3, uint8_t inSpeedMax = 128);

	/** Sets all data of the locomotive to default values.
	*/
	void initialize();

	//********************************************************************************
	// Functions to update the Locomotive instance data. DONT SEND ANY ORDER TO DCCpp !

	/** Set the locomotive DCC address
	@param inAddress	Locomotive new DCC address.
	*/
	void setAddress(uint16_t inAddress) { this->address = inAddress; }
	/** Get the locomotive DCC address.
	@return	Locomotive DCC address.
	*/
	uint16_t getAddress() const { return this->address; }
	/** Set the locomotive name
	@param inName	Locomotive new name.
	*/
	void setName(const String& inName) { this->name = inName; }
	/** Get the locomotive name.
	@return	Locomotive name.
	*/
	const String& getName() const { return this->name; }
	/** Get the locomotive speed register number.
	@return	Locomotive register number. 0 if no register allocated for this loco.
	*/
	uint8_t getSpeedRegister() const { return this->speedRegisterNumber; }

	/** Set the locomotive max speed : 14, 28 or 128.
	@param inSpeedMax	Locomotive max speed steps, default is 128.
	@remark	any other value than 14, 28 or 128 will result in a 128 speed steps value.
	*/
	void setSpeedMax(uint8_t inSpeedMax) { if (inSpeedMax == 14 || inSpeedMax == 28 || inSpeedMax == 128) this->speedMax = inSpeedMax; else this->speedMax = 128; }
	/** Get the locomotive max speed.
	@return	Locomotive max speed steps : 14, 28 or 128.
	*/
	uint8_t getSpeedMax() const { return this->speedMax; }

	/** Set the locomotive current speed.
	@param inSpeed	Locomotive new speed, from 0 to 127 : not DCC speed (see remark...)
	@remark	For 128 steps max, 0 is stop, 1 is emergency stop... 
	So user speed is coded from 0 (stopped) to 126, and DCC speed value is coded from 2 to 127, with 0 or 1 means stopped machine.

	DCC 128 speed    : 0 1 2 3 4 ... 125 126 127
	                   |\	 | | |			|		|		|
	Loco currentSpeed: 0 0 1 2 3 ... 124 125 126
	*/
	void setSpeed(uint8_t speed) 
	{
		this->currentSpeed = speed;
	}

	/** Get the locomotive current speed.
	@return	Locomotive current speed.
	*/
	uint8_t getSpeed() const {	return this->currentSpeed; }

	/** Set the locomotive direction.
	@param inForward	True if the locomotive must go forward.
	*/
	void setDirection(bool inForward) {		this->forwardDirection = inForward;	}
	/** Check if the locomotive direction is forward.
	@return True if the locomotive direction is forward.
	*/
	bool isDirectionForward() const {		return this->forwardDirection;	}

	/** Set the given function to the given state.
	*/
	void setFunction(uint8_t inFunction, bool inActivate);

	//********************************************************************************
	// Functions to update the Locomotive data AND send DCCpp corresponding orders !

	/** Set the locomotive current speed.
	@param inSpeed	Locomotive new speed.
	@remark	For 128 steps max, 0 is stop, 1 is emergency stop...
	*/
	void setDCCSpeed(uint8_t speed);

	/** Send a DCC command to stop this locomotive.
	*/
	void stop();

	/* Send a DCC command to stop the locmotive IMMEDIATELY
	*/
	void emergencyStop();

	/** Set the locomotive direction.
	@param inForward	True if the locomotive must go forward.
	*/
	void setDCCDirection(bool inForward);

	/** Set the given function to the given state.
	*/
	void setDCCFunction(uint8_t inFunction, bool inActivate);

	/* Save this locomotives data in JSON format.
	*/
	bool Save(JsonObject inObject);

#ifdef DCCPP_DEBUG_MODE
	/** Print the status of the locomotive.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	void printLocomotive();
#endif
};

#endif
#endif