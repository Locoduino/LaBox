//-------------------------------------------------------------------
#ifndef __Throttle_hpp__
#define __Throttle_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

#if defined(USE_THROTTLES)
#include "MessageConverter.hpp"		 
#include "MessageConverterWiThrottle.hpp"		 

#define THROTTLE_COMMAND_SIZE		64

/** This is a class to handle decoder functions.
An instance of this class handle the status of the functions of one decoder.
A function can be active or not.
*/
class Throttle
{
protected:
	String printMemo;						// buffer for print() functions
	String name;
	int startCommandCharacter, endCommandCharacter;
	uint16_t id;
	char commandString[THROTTLE_COMMAND_SIZE + 1];

public:
	Throttle* pNextThrottle;					/**< Address of the next object of this class. NULL means end of the list of Throttles. Do not change it !*/
	MessageConverter* pConverter;

	/** Creates a new instance for only one register.
	@param inName	throttle new name.
	*/
	Throttle(const String& inName);

	/** Sets the converter if any. 
	@param inpConverter		New converter, or NULL to remove the current converter.
	*/
	void setConverter(MessageConverter* inpConverter) {	this->pConverter = inpConverter;	}

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
	void setName(const String& inName) { this->name =  inName; }

	/** Gets the Throttle name.
	@return	Throttle name.
	*/
	const String& getName() const { return this->name; }

	static void pushMessageInStack(uint16_t inThrottleId, const String& inMessage);
	static const String& getMessageFromStackMessage(const String& inMessage, String& inToReturn);
	static Throttle* getThrottleFromStackMessage(const String& inMessage);
	static bool getCharacter(char inC, Throttle* inpThrottle);

	virtual bool begin() = 0;
	virtual void end() = 0;
	virtual bool loop() = 0;
	virtual bool sendMessage(const String& inMessage) = 0;
	virtual bool isConnected() = 0;
	virtual void setCommandCharacters(int inStartCharacter, int inEndCharacter);
	virtual bool sendNewline();

	void Print(const char line[]);
	void Println(const char line[]);
	void Print(const String &line);
	void Println(const String &line);
	void Print(int value);
	void Print(int value, int i);
	void Println(int value);
	void Println(int value, int i);
	void Print(IPAddress inIp);
	void Println(IPAddress inIp);

	static void print(const char line[]);
	static void println(const char line[]);
	static void print(const String &line);
	static void println(const String &line);
	static void print(int value);
	static void print(int value, int i);
	static void println(int value);
	static void println(int value, int i);
	static void print(IPAddress inIp);
	static void println(IPAddress inIp);

#ifdef VISUALSTUDIO
	static void test();
#endif

#ifdef DCCPP_DEBUG_MODE
	/** Print the status of the Throttle.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	virtual void printThrottle();
#endif

protected:
	virtual bool pushMessage(const String& inMessage);
};

#include "SerialInterface.hpp"
#include "ThrottleSerial.hpp"
#include "ThrottleEthernet.hpp"
#include "ThrottleWifi.hpp"
#include "ThrottleWifiAPClient.hpp"		 

#endif
#endif