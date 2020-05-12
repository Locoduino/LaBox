//-------------------------------------------------------------------
#ifndef __Throttle_hpp__
#define __Throttle_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

#if defined(USE_THROTTLES)
#include "CircularBuffer.hpp"

#include "MessageConverter.hpp"		 
#include "MessageConverterWiThrottle.hpp"		 
#include "MessageConverterZ21.hpp"		 

#define THROTTLE_COMMAND_SIZE		64
#define THROTTLE_UDPBYTE_SIZE		64

enum ThrottleType : byte
{
	SerialThrottle,
	Wifi,
	Ethernet,
	UserDefined
};

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
	bool dontReply;	// if true, the Throttle will not send replies to the controller.

public:
	ThrottleType type;
	Throttle* pNextThrottle;					/**< Address of the next object of this class. NULL means end of the list of Throttles. Do not change it !*/
	MessageConverter* pConverter;
	CircularBuffer* pBuffer;
	bool contacted; // if true, this throttle has established contact.
	unsigned int lastActivityDate;	// If 0 and timeOutDelay != 0, the throttle is not connected...
	unsigned int timeOutDelay;			// 0 if no timeout available.

	/** Create a new instance for only one register.
	@param inName	throttle new name.
	@param inTimeOutDelay		value of timeout delay in ms. A value of 0 means no timeout at all.
	*/
	Throttle(const String& inName, unsigned int inTimeOutDelay = 0);

	/** Set the converter if any.
	@param inpConverter		New converter, or NULL to remove the current converter.
	*/
	void setConverter(MessageConverter* inpConverter) { this->pConverter = inpConverter; }

	/** Set the timeout delay. 0 for no timeout.
	@param inTimeOutDelay		value of timeout delay in ms. A value of 0 means no timeout at all.
	*/
	void setTimeOutDelay(unsigned int inTimeOutDelay) { this->timeOutDelay = inTimeOutDelay; }

	/** Set if the Throttle should reply to the controller.
	@param inpConverter		New converter, or NULL to remove the current converter.
	*/
	void setDontReply(MessageConverter* inpConverter) { this->pConverter = inpConverter; }

	/** Sets the throttle id
	@param inId		Throttle new id.
	*/
	void setId(uint16_t inId) { this->id = inId; }

	/** Gets the throttle id.
	@return	Throttle id.
	*/
	uint16_t getId() const { return this->id; }

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

	virtual bool begin(EthernetProtocol inProtocol) = 0;
	virtual void end();
	// Loop to get text and binary messages
	virtual bool loop() = 0;
	// Treat messages
	virtual bool processBuffer();
	virtual bool sendMessage(const String& inMessage) = 0;
	virtual bool isConnected() = 0;	// abstract functions cannot be const !
	virtual void setCommandCharacters(int inStartCharacter, int inEndCharacter);
	virtual bool sendNewline() const;
	virtual CircularBuffer* getCircularBuffer() const {	return NULL; }
	virtual void write(byte* inpData, int inLengthData) {}
	virtual IPAddress remoteIP() {	return INADDR_NONE; }

	bool isContacted() { return this->contacted || this->lastActivityDate != 0; }

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