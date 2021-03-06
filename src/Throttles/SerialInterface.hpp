//-------------------------------------------------------------------
#ifndef __serialInterface_H__
#define __serialInterface_H__
//-------------------------------------------------------------------

#include <DCCpp.h>

#ifndef NO_THROTTLE

/** @file SerialInterface.hpp

This is the most curious throttle, because the class itself does not exists until you add the macro SERIAL_INTERFACE to your sketch.

    #include "Labox.h"

    SERIAL_INTERFACE(Serial3, MyName);

this macro will create a class with the name SerialInterfaceMyName. 
To use it in a ThrottleSerial, juste use the other macro during the ThrottleSerial declaration :

		ThrottleSerial throttleSerial("Serial3-name", SERIAL_INTERFACE_INSTANCE(MyName));

You should avoid to declare a SERIAL_INTERFACE if not necessary, because the Serial library delivered with the Arduino IDE
allocates a lot of memory for each serial channel used.
The only variable thing in this macro is the name of the serial channel to use : Serial for a UNO or a NANO, but Serial or
Serial1 to 3 on a Mega or a Due. Even a SoftwareSerial (from SoftwareSerial library (https://www.arduino.cc/en/Reference/SoftwareSerial) ) or a AltSoftSerial
(from the well named library.. AltSoftSerial (https://github.com/PaulStoffregen/AltSoftSerial) !) can be used !
*/

//-------------------------------------------------------------------

class SerialInterface
{
public:
	SerialInterface() { }
	
	virtual void begin() { }
	
	virtual int available() {	return 0;	}
	virtual char read() { return 0; }
	virtual size_t println(const char* pMessage) { return 0; }
	virtual size_t println(const String& inMessage) { return 0; }
};

/** Use this macro to define a SerialInterface.
@param SERIAL_PORT	name of the serial port. See the file description for more information.
@param NAME Suffix of the created class name. See the file description for more information.
*/
#define SERIAL_INTERFACE(SERIAL_PORT, NAME) \
\
class SerialInterface ## NAME : public SerialInterface \
{\
public:\
	inline SerialInterface ## NAME() { }\
	\
	inline void begin() { }\
	\
	inline int available() \
	{\
		return SERIAL_PORT.available();\
	}\
	inline char read() \
	{\
		return SERIAL_PORT.read();\
	}\
	inline size_t println(const char *pMessage) \
	{\
		return SERIAL_PORT.println(pMessage);\
	}\
	inline size_t println(const String& inMessage) \
	{\
		return SERIAL_PORT.println(inMessage);\
	}\
\
};

/** Use this macro to define a new named SerialInterface object.
@param NAME Suffix of the created class name used in the previous SERIALINTERFACE. See the file description for more information.
*/
#define SERIAL_INTERFACE_INSTANCE(NAME) new SerialInterface ## NAME()

//-------------------------------------------------------------------
#endif
#endif
