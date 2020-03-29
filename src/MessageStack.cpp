#include "MessageStack.hpp"

MessageStack MessageStack::MessagesStack;

void MessageStack::FreeMessage(byte inMessageIndex) 
{
	this->messages[inMessageIndex][0] = 0;
}

void MessageStack::PushMessage(const char *inMessage)
{
	for (int i = 0; i < MESSAGE_MAXNUMBER; i++)
		if (this->messages[i][0] == 0)
		{
			strncpy(this->messages[i], inMessage, MESSAGE_MAXSIZE);
			return;
		}

#ifdef DCCPP_DEBUG_MODE
	Serial.println(F("Error : a message has been lost ! Stack is full !"));
#endif
}

byte MessageStack::GetPendingMessageIndex()
{
	for (int i = 0; i < MESSAGE_MAXNUMBER; i++)
		if (this->messages[i][0] != 0)
			return i;

	return 255;
}

const char *MessageStack::GetMessage(byte inMessage, char *inMessageBuffer)
{
	strncpy(inMessageBuffer, this->messages[inMessage], MESSAGE_MAXSIZE);

	this->FreeMessage(inMessage);

	return inMessageBuffer;
}

#ifdef DCCPP_DEBUG_MODE
#ifdef VISUALSTUDIO
/** Unit test function
*/
void MessageStack::Test()
{
	char buffer[MESSAGE_MAXSIZE];

	Serial.println("After all initialized");
	printStack();

	this->PushMessage("mess1");
	Serial.println("After one pushed");
	printStack();

	this->PushMessage("mess2");
	Serial.println("After second pushed");
	printStack();

	int pending = this->GetPendingMessageIndex();
	Serial.print("Message available :");
	Serial.print(pending);
	Serial.println("");

	this->GetMessage(pending, buffer);
	Serial.print("Got message :\"");
	Serial.print(buffer);
	Serial.println("\"");
	printStack();

	pending = this->GetPendingMessageIndex();
	Serial.print("Message available :");
	Serial.print(pending);
	Serial.println("");

	this->GetMessage(pending, buffer);
	Serial.print("Got message :\"");
	Serial.print(buffer);
	Serial.println("\"");
	printStack();

	this->PushMessage("mess0");
	this->PushMessage("mess1");
	this->PushMessage("mess2");
	this->PushMessage("mess3");
	this->PushMessage("mess4");
	this->PushMessage("mess5");
	this->PushMessage("mess6");
	this->PushMessage("mess7");
	this->PushMessage("mess8");
	this->PushMessage("mess9");
	this->PushMessage("mess10");	// This one cant be added !
	printStack();
}
#endif

void MessageStack::printStack()
{
	for (int i = 0; i < MESSAGE_MAXNUMBER; i++)
	{
		Serial.print(i);
		Serial.print(" : \"");
		Serial.print(messages[i]);
		Serial.println("\"");
	}

	Serial.println("");
}
#endif
