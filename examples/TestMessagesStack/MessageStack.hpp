//-------------------------------------------------------------------
#ifndef __MessageStack_Hpp__
#define __MessageStack_Hpp__
//-------------------------------------------------------------------

#if 1
#include <Arduino.h>

#define MESSAGE_MAXNUMBER	10
#define MESSAGE_MAXSIZE	10

/** The message stack is a small location for a few messages received and ready to be send to the caller.
This is a barely a list. If the string for the given index is empty, so no this index is free.
Each PushMessage will push a new one in the first free index of the list.
Each GetMessage will returns the first not free message content, and free the message list for this index.
There is no priority in the list...
*/
class MessageStack
{
private:
	char messages[MESSAGE_MAXNUMBER][MESSAGE_MAXSIZE];
	SemaphoreHandle_t xSemaphore; // semaphore d'exclusion mutuelle

	MessageStack();

	void FreeMessage(byte inMessageIndex); 

public:
	/** Stack of messages itself.*/
	static MessageStack MessagesStack;

  void begin();
  
	/** Add a new message	to the stack.
	@param inMessage	Message to add to the stack.
	@param inType	type
	@param inData	associated data
  @return true if the massage has benn pushed. false if it is lost due to max stack size reached...
	*/
	bool PushMessage(const char *inMessage);
	/** Gets the given message from the stack, at the given index.
	@param inIndex	index of the message to get.
	@param inBufferToFill	buffer to fill. Must be allocated at size MESSAGE_MAXSIZE.
	@return	string pointer to the found message, or an empty string if the given index is free.
	@remark A previous call to GetPendingMessageIndex() will give the first available message index.
	*/
	const char *GetMessage(byte inIndex, char *inBufferToFill);
	/** Gets the first available message index in the stack. There is no history of stack filling, so the 
	first one is not necessary the oldest in the list...
	@return First available message, or 255 if no message available.
	*/
	byte GetPendingMessageIndex();

  byte GetCount();
  
#ifdef DCCPP_DEBUG_MODE
#ifdef VISUALSTUDIO
	/** Unit test function
	*/
	void Test();
#endif

	/** Print the list of messages in the stack.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	void printStack();
#endif
};

#define START_SEMAPHORE()	\
	{ \
		byte semaphoreTaken = this->xSemaphore == NULL?1:0; \
		if (this->xSemaphore != NULL) \
			if (xSemaphoreTake(this->xSemaphore, (TickType_t)100) == pdTRUE) \
				semaphoreTaken = 1; \
		if (semaphoreTaken == 1)

#define END_SEMAPHORE()	\
		xSemaphoreGive(this->xSemaphore); \
	}

#define ABORT_SEMAPHORE()	\
		xSemaphoreGive(this->xSemaphore);


#endif
#endif
