//-------------------------------------------------------------------
#ifndef __Throttle_hpp__
#define __Throttle_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

#if defined(USE_THROTTLES)
#define THROTTLE_NAME_SIZE	32
#define THROTTLE_PRINTMEMO_SIZE	512

/** This is a class to handle decoder functions.
An instance of this class handle the status of the functions of one decoder.
A function can be active or not.
*/
class Throttle
{
private:
	char printMemo[THROTTLE_PRINTMEMO_SIZE];						// buffer for print() functions

protected:
	char name[THROTTLE_NAME_SIZE];
	uint16_t id;

public:
	Throttle* pNextThrottle;					/**< Address of the next object of this class. NULL means end of the list of Throttles. Do not change it !*/

	/** Creates a new instance for only one register.
	@param inName	throttle new name.
	*/
	Throttle(const char* inName);

	/** Sets the throttle id
	@param inId		Throttle new id.
	*/
	void setId(uint16_t inId) { this->id = inId; }

	/** Gets the throttle id.
	@return	Throttle id.
	*/
	uint16_t getId() { return this->id; }

	/** Sets the Throttle name
	@param inName	Throttle new name.
	*/
	void setName(const char * inName) { strncpy(this->name, inName, THROTTLE_NAME_SIZE); }

	/** Gets the Throttle name.
	@return	Throttle name.
	*/
	const char *getName() const { return this->name; }

	static void pushMessageInStack(uint16_t inThrottleId, const char* pMessage);
	static const char* getMessageFromStackMessage(const char* pMessage);
	static Throttle* getThrottleFromStackMessage(const char* pMessage);

	virtual bool begin() = 0;
	virtual bool receiveMessages() = 0;
	virtual bool sendMessage(const char *) = 0;
	virtual void end() = 0;
	virtual bool isConnected() = 0;
	virtual bool sendNewline();

	void Print(const char line[]);
	void Println(const char line[]);
	void Print(int value);
	void Print(int value, int i);
	void Println(int value);
	void Println(int value, int i);

	static void print(const char line[]);
	static void println(const char line[]);
	static void print(int value);
	static void print(int value, int i);
	static void println(int value);
	static void println(int value, int i);

#ifdef VISUALSTUDIO
	static void test();
#endif

#ifdef DCCPP_DEBUG_MODE
	/** Print the status of the Throttle.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	virtual void printThrottle();
#endif
};

#include "SerialInterface.hpp"
#include "ThrottleSerial.hpp"
#include "ThrottleEthernet.hpp"
#include "ThrottleWifi.hpp"
#include "ThrottleWifiAPClient.hpp"

#endif
#endif