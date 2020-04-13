/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <FunctionsState class>
*************************************************************/

#include "Arduino.h"
#include "DCCpp.h"

#ifdef USE_THROTTLES
Throttle::Throttle(const char* inName)
{
	// 'static' data, not updated during the run.
	strncpy(this->name, inName, THROTTLE_NAME_SIZE);

	this->id = 0;
	this->printMemo[0] = 0;

	// Variable data
	this->pNextThrottle = NULL;

	Throttles::add(this);
}

// The messages in the stack have the syntax:
// DDDSSSSSSSSSSSSSSSSSSSSSSSSSSSSSS
// Where DDD is the numerical form of the Throttle Id which have got the message, in ordrer to answer to this throttle only.
// SSSSSSSSSSSSSSSSSSSSSSSSSSSSSS is the classic DCC++ message received by the throttle.
void Throttle::pushMessageInStack(uint16_t inThrottleId, const char* pMessage)
{
	char buffer[MESSAGE_MAXSIZE];

	buffer[0] = '0' + inThrottleId / 100;
	buffer[1] = '0' + (inThrottleId - ((inThrottleId / 100) * 100)) / 10;
	buffer[2] = '0' + inThrottleId % 10;
	buffer[3] = 0;
	strncpy(buffer + 3, pMessage, MESSAGE_MAXSIZE - 3);
	MessageStack::MessagesStack.PushMessage(buffer);
}

const char* Throttle::getMessageFromStackMessage(const char* pMessage)
{
	return pMessage + 3;
}

Throttle* Throttle::getThrottleFromStackMessage(const char* pMessage)
{
	uint16_t id = 0;

	id = ((pMessage[0] - '0') * 100) + ((pMessage[1] - '0') * 10) + (pMessage[2] - '0');

	return Throttles::get(id);
}

bool Throttle::sendNewline()
{
	return true;
}

// Classic print part

void Throttle::Print(const char line[])
{
	strncat(this->printMemo, line, THROTTLE_PRINTMEMO_SIZE);
}

void Throttle::Println(const char line[])
{
	strncat(this->printMemo, line, THROTTLE_PRINTMEMO_SIZE);
	this->sendMessage(this->printMemo);
	this->printMemo[0] = 0;
}

void Throttle::Print(int value)
{
	this->Print(value, DEC);
}

void Throttle::Print(int value, int i)
{
	char buffer[20];         //the ASCII of the integer will be stored in this char array

	switch (i)
	{
	case DEC:
		itoa(value, buffer, 10); //(integer, yourBuffer, base)
		this->Print(buffer);
		break;
	case HEX:
		itoa(value, buffer, 16); //(integer, yourBuffer, base)
		this->Print(buffer);
		break;
	case BIN:
		itoa(value, buffer, 2); //(integer, yourBuffer, base)
		this->Print(buffer);
		break;
	}
}

void Throttle::Println(int value)
{
	this->Print(value, DEC);
	this->Println("");
}

void Throttle::Println(int value, int i)
{
	this->Print(value, i);
	this->Println("");
}

// Static print part

void Throttle::print(const char line[])
{
	if (TextCommand::pCurrentThrottle != NULL)
		TextCommand::pCurrentThrottle->Print(line);
	else
		Serial.print(line);
}

void Throttle::println(const char line[])
{
	if (TextCommand::pCurrentThrottle != NULL)
		TextCommand::pCurrentThrottle->Println(line);
	else
		Serial.println(line);
}

void Throttle::print(int value)
{
	if (TextCommand::pCurrentThrottle != NULL)
		TextCommand::pCurrentThrottle->Print(value);
	else
		Serial.print(value);
}

void Throttle::print(int value, int i)
{
	if (TextCommand::pCurrentThrottle != NULL)
		TextCommand::pCurrentThrottle->Print(value, i);
	else
		Serial.print(value, i);
}

void Throttle::println(int value)
{
	if (TextCommand::pCurrentThrottle != NULL)
		TextCommand::pCurrentThrottle->Println(value);
	else
		Serial.println(value);
}

void Throttle::println(int value, int i)
{
	if (TextCommand::pCurrentThrottle != NULL)
		TextCommand::pCurrentThrottle->Println(value, i);
	else
		Serial.println(value, i);
}

#ifdef VISUALSTUDIO
SERIAL_INTERFACE(Serial, Test);
const char* ssidTest = "VIDEOFUTUR_C56165_2.4G";
const char* passwordTest = "EenPghQD";
// the media access control (ethernet hardware) address for the shield:
uint8_t MacTest[] = { 0xBE, 0xEF, 0xBE, 0xEF, 0xBE, 0x80 };
//the IP address for the shield:
uint8_t IpTest[] = { 192, 168, 1, 100 };

void Throttle::test()
{
	ThrottleSerial throttleSerial("TestSerial 12", new SerialInterfaceTest());
	throttleSerial.setId(12);

#if !defined(ARDUINO_ARCH_ESP32)
	ThrottleEthernet throttleEthernet("TestEthernet 9", MacTest, IpTest, 2560, TCP);
	throttleEthernet.setId(9);
#endif

	ThrottleWifi throttleWifi("TestWifi 567", MacTest, IpTest, 2560, TCP);
	throttleWifi.setId(567);

	Throttles::printThrottles();

	Throttle::pushMessageInStack(0, "Throttle serial !");
	Throttle::pushMessageInStack(9, "Throttle ethernet !");
	Throttle::pushMessageInStack(567, "Throttle wifi !");
	Throttle::pushMessageInStack(12, "Throttle serial !");

	MessageStack::MessagesStack.printStack();

	char buffer[MESSAGE_MAXSIZE];
	while (true)
	{
		int index = MessageStack::MessagesStack.GetPendingMessageIndex();

		if (index == 255)
			break;

		MessageStack::MessagesStack.GetMessage(index, buffer);

		Throttle* throttle = Throttle::getThrottleFromStackMessage(buffer);
		strncpy(buffer, Throttle::getMessageFromStackMessage(buffer), MESSAGE_MAXSIZE - 3);

		Serial.print(throttle->getName());
		Serial.print(" : ");
		Serial.println(buffer);
	}
}
#endif

#ifdef DCCPP_DEBUG_MODE
void Throttle::printThrottle()
{
	Serial.print(this->name);
	Serial.print(" id:");
	Serial.print(this->id);

	Serial.println("");
}
#endif
#endif