//-------------------------------------------------------------------
#ifndef __FunctionsState_hpp__
#define __FunctionsState_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

#ifdef USE_LOCOMOTIVES
#include "ArduinoJson.h"
class Locomotive;
#endif

/**
This is a class to handle decoder functions.
An instance of this class handle the status of the functions of one decoder.
A function can be active or not, and modal or not.
An active function has been activated by a message received from a Throttle.
A modal function is a permanent fonction, like lights. Non modal function are those working
with a time limit, like horn/whistle or coupler noise.
DCC function communication is organized by block of several functions : 
block 0 : FL + F1->F4
block 1 : F5->F8
block 2 : F9->F12
block 3 : F13->F20
block 4 : F21->F28
If all the functions of a DCC block are all modals, so these functions can be repeated on register 0.
By default, only FL (F0) is marked as modal, so there will be no repetition of function DCC packet.
*/
class Functions
{
private:
	/**Four bytes : 32 bits : from function 0 to 28, maximum for DCC.
	A bit at true is an activated function.*/
	byte modalFunctions[4];

	/**Four bytes : 32 bits : from function 0 to 28, maximum for DCC.
	A bit at true is an activated function.*/
	byte activeFlags[4];

	/**This is the last states sent to the decoder by DCC.
	Used to be sure to send only changed states.
	*/
	byte activeFlagsSent[4];

	/**Memo of the bytes to send to dcc orders according to actual activated functions.
	* These bytes has to be reconstructed at each modification of function activation.
	* The values are coded on 'int' just to be able to keep a -1 value !
	*/
	int dccFirstByte[5], dccSecondByte[5];

	//inline byte byteNumber(byte inFunctionNumber) { return inFunctionNumber / 8; }
	//inline byte bitNumber(byte inFunctionNumber) { return inFunctionNumber % 8; }

#ifdef USE_LOCOMOTIVES
	static Locomotive *currentLocomotive;
	static byte currentFunctionBlock;		// F0-4 : 0, F5-8 : 1, F9-12 : 2, F13-20 : 3, F20-28 : 4
#endif

public:
	/** Initialize the instance.
	*/
	Functions();

	// Modal part
	/** Get one byte from the modalFunctions array.
	@param inIndex	Number of the byte to get, from 0 to 3.
	@return byte describing a part of the flags.
	*/
	byte getModalFlags(byte inIndex) { return this->modalFunctions[inIndex]; }
	/** Reset all modal functions to false.
		*/
	void clearModalFunctions();
	/** Declare one function modal. The allowed number goes from 0 to 28, maximum for DCC.
	@param inFunctionNumber	Number of the function to declare modal.
	*/
	void setModal(byte inFunctionNumber);
	/** Declare one function not modal. The allowed number goes from 0 to 28, maximum for DCC.
	@param inFunctionNumber	Number of the function to declare not modal.
	*/
	void unsetModal(byte inFunctionNumber);
	/** Check if the given function is activated. The allowed number goes from 0 to 28, maximum for DCC.
	@param inFunctionNumber	Number of the function to activate.
	@return True if the given function is activated.
	*/
	bool isModalFunction(byte inFunctionNumber);

	// Activation part
	/** Get one byte from the activeFlags array.
	@param inIndex	Number of the byte to get, from 0 to 3.
	@return byte describing a part of the flags.
	*/
	byte getActiveFlags(byte inIndex) { 	return this->activeFlags[inIndex]; }

	/** Reset all functions to inactive.
	*/
	void clear();
	/** Activate one function. The allowed number goes from 0 to 28, maximum for DCC.
	@param inFunctionNumber	Number of the function to activate.
	*/
	void activate(byte inFunctionNumber);
	/** Inactivate one function. The allowed number goes from 0 to 28, maximum for DCC.
	@param inFunctionNumber	Number of the function to inactivate.
	*/
	void inactivate(byte inFunctionNumber);
	/** Check if the given function is activated. The allowed number goes from 0 to 28, maximum for DCC.
	@param inFunctionNumber	Number of the function to activate.
	@return True if the given function is activated.
	*/
	bool isActivated(byte inFunctionNumber);
	/**Copy the current active flags into 'sent' active flags.*/
	void statesSent();
	/** Check if the given function had its activation flag changed when sent to the decoder last time.
	The allowed number goes from 0 to 28, maximum for DCC.
	@param inFunctionNumber	Number of the function to check.
	@return True if the given function activation flag is different between activFlags and sentActiveFlags.
	*/
	bool isActivationChanged(byte inFunctionNumber);

	/** Check if the given function block contains at least one finction activated.
	The allowed number goes from 0 to 4 : Functions 0 to 4 : block 0, F5-8 : 1, F9-12 : 2, F13-20 : 3, F20-28 : 4.
	@param inBlock	Number of the block function to check.
	@return True if at least one function is activated in the given block.
	*/
	bool isFunctionBlockActivated(byte inBlock);

	/** Build the bytes to send to DCC according to modified functions.
	@param inBlock	Number of the function to check. The allowed number goes from 0 to 4 : Functions 0 to 4 : block 0, F5-8 : 1, F9-12 : 2, F13-20 : 3, F20-28 : 4.	
	@param pByte1	first byte address for DCC
	@param pByte2	second byte address for DCC
	@param storeBytes	if true, the computed bytes will be stored in firstDccBytes and secondDccBytes arrays.
	@return True if at least one function is activated in the given block.
	*/
	bool buildDCCbytes(byte blockNb, int* pByte1, int* pByte2, bool storeBytes = false);

	/** Build the bytes to send to DCC according to modified functions of the first DCC block from F0 to F4.
	@return True if at least one function is activated in the given block.
	*/
	bool buildDCCF0F4bytes();
	/** Build the bytes to send to DCC according to modified functions of the first DCC block from F5 to F8.
	@return True if at least one function is activated in the given block.
	*/
	bool buildDCCF5F8bytes();
	/** Build the bytes to send to DCC according to modified functions of the first DCC block from F9 to F12.
	@return True if at least one function is activated in the given block.
	*/
	bool buildDCCF9F12bytes();
	/** Build the bytes to send to DCC according to modified functions of the first DCC block from F14 to F20.
	@return True if at least one function is activated in the given block.
	*/
	bool buildDCCF13F20bytes();
	/** Build the bytes to send to DCC according to modified functions of the first DCC block from F21 to F28.
	@return True if at least one function is activated in the given block.
	*/
	bool buildDCCF21F28bytes();

#ifdef USE_LOCOMOTIVES
	/**This function will send packets on DCC track at regular intervals to restore functions on machines which suffered of a long short-circuit.*/
	static void functionsLoop();

	/* Save this data in JSON format.
	*/
	bool Save(JsonObject inObject);
#endif

#ifdef DCCPP_DEBUG_MODE
	/** Print the list of activated functions.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	void printActivated();

	static bool test();
#endif
};

#endif