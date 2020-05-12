//-------------------------------------------------------------------
#ifndef __CircularBuffer_Hpp__
#define __CircularBuffer_Hpp__
//-------------------------------------------------------------------

#include <Arduino.h>

/** The message stack is a small location for a few messages received and ready to be send to the caller.
This is a barely a list. If the string for the given index is empty, so no this index is free.
Each PushMessage will push a new one in the first free index of the list.
Each GetMessage will returns the first not free message content, and free the message list for this index.
There is no priority in the list...
*/
class CircularBuffer
{
private:
	byte *buffer;
	int size;
	int head;
	int tail;
	bool full;
	int peakCount;
	SemaphoreHandle_t xSemaphore; // semaphore d'exclusion mutuelle

public:
	CircularBuffer(int inSize);

	/** Initialize the list.
	@param inMultiThread	If the buffer is used in multi-thread environnement, initialize the semaphore.
	*/
	void begin(bool inMultiThread);

	/** Close the usage of this buffer and free the allocated memory.
	*/
	void end();

	/** Remove the full content of the buffer, ready for the next push.
	*/
	void clear();

	/** Add some bytes in the buffer.
	@param inpData	pointer to bytes to add.
	@param inDataLength	number of bytes to add.
  @return true if all bytes have been pushed. false if one or more bytes are lost due to max size reached...
	*/
	bool PushBytes(byte *inpData, int inDataLength);

	/** Get the next byte from the buffer.
	@return first available byte, or 0.
	*/
	byte GetByte();

	/** Get the next two bytes from the buffer, to form an integer.
	@return integer created from two available bytes, or 0.
	*/
	int16_t GetInt16();

	/** Get the next four bytes from the buffer, to form an integer.
	@return integer created from two available bytes, or 0.
	*/
	int32_t GetInt32();

	/** Get some bytes.
	@param inpData	buffer to fill.
	@param inDataLength	number of bytes to get.
	@return true if all bytes have been get. false if there were not enough bytes in th buffer.
	*/
	bool GetBytes(byte* inpData, int inDataLength);

	/** Get the next two bytes from the given buffer starting from the given position, to form an integer.
	@return integer created from two available bytes, or 0.
	*/
	static int16_t GetInt16(byte* pBuffer, int inPos);

	/** Get the next four bytes from the given buffer starting from the given position, to form a 32 bits integer.
	@return integer created from four available bytes, or 0.
	*/
	static int32_t GetInt32(byte* pBuffer, int inPos);

	/** Get some bytes from the given buffer starting from the given position.
	@param inpData	buffer to fill.
	@param inDataLength	number of bytes to get.
	@return true if all bytes have been get. false if there were not enough bytes in th buffer.
	*/
	static void GetBytes(byte *pBuffer, int inPos, byte* inpData, int inDataLength);

	/** Count the number of bytes in the buffer.
	@return number of bytes in the buffer.
	*/
	int GetCount();

	/** Get the maximum size used by the buffer since the beggining of its usage.
	@return maximum number of bytes in the buffer.
	*/
	int GetPeakCount() { return this->peakCount; }

	/** Check if the buffer is empty or not.
	*/
	bool isEmpty() const
	{
		//if head and tail are equal, we are empty
		return (!this->full && (this->head == this->tail));
	}

	/** Check if the buffer is full or not.
	*/
	bool isFull() const
	{
		//If tail is ahead the head by 1, we are full
		return this->full;
	}

#ifdef DCCPP_DEBUG_MODE
#ifdef VISUALSTUDIO
	/** Unit test function
	*/
	static void Test();
#endif

	/** Print the list of messages in the stack.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	void printCircularBuffer();
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