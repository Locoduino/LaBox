/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <FunctionsState class>
*************************************************************/

#include "DCCpp.h"

#ifdef USE_LOCOMOTIVES
#include "ArduinoJson.h"

Locomotive *FunctionsState::currentLocomotive = NULL;
byte FunctionsState::currentFunctionBlock = 0;
#endif

FunctionsState::FunctionsState()
{
	this->clear();
}

void FunctionsState::clear()
{
	// Clear all functions
	this->activeFlags[0] = 0;
	this->activeFlags[1] = 0;
	this->activeFlags[2] = 0;
	this->activeFlags[3] = 0;

	this->dccFirstByte[0] = this->dccFirstByte[1] = this->dccFirstByte[2] = this->dccFirstByte[3] = this->dccFirstByte[4] = -1;
	this->dccSecondByte[0] = this->dccSecondByte[1] = this->dccSecondByte[2] = this->dccSecondByte[3] = this->dccSecondByte[4] = -1;

	this->statesSent();
}

void FunctionsState::activate(byte inFunctionNumber)
{
	bitSet(this->activeFlags[inFunctionNumber / 8], inFunctionNumber % 8);
	if (inFunctionNumber <= 4)
		this->buildDCCF0F4bytes();
	else if (inFunctionNumber <= 8)
		this->buildDCCF5F8bytes();
	else if (inFunctionNumber <= 12)
		this->buildDCCF9F12bytes();
	else if (inFunctionNumber <= 20)
		this->buildDCCF13F20bytes();
	else
		this->buildDCCF21F28bytes();
}

void FunctionsState::inactivate(byte inFunctionNumber)
{
	bitClear(this->activeFlags[inFunctionNumber / 8], inFunctionNumber % 8);
	if (inFunctionNumber <= 4)
		this->buildDCCF0F4bytes();
	else if (inFunctionNumber <= 8)
		this->buildDCCF5F8bytes();
	else if (inFunctionNumber <= 12)
		this->buildDCCF9F12bytes();
	else if (inFunctionNumber <= 20)
		this->buildDCCF13F20bytes();
	else
		this->buildDCCF21F28bytes();
}

bool FunctionsState::isActivated(byte inFunctionNumber)
{
	return bitRead(this->activeFlags[inFunctionNumber / 8], inFunctionNumber % 8);
}

bool FunctionsState::isActivationChanged(byte inFunctionNumber)
{
	return bitRead(this->activeFlagsSent[inFunctionNumber / 8], inFunctionNumber % 8) != isActivated(inFunctionNumber);
}

void FunctionsState::statesSent()
{
	for (int i = 0; i < 4; i++)
		this->activeFlagsSent[i] = this->activeFlags[i];
}

bool FunctionsState::isFunctionBlockActivated(byte inBlock)
{
	switch (inBlock)
	{
	case 0:	//F0-4
		return this->activeFlags[0] & B00011111;
	case 1:	//F4-8
		return this->activeFlags[0] & B11100000 || this->activeFlags[1] & B00000001;
	case 2:	//F9-12
		return this->activeFlags[1] & B00011110;
	case 3:	//F13-20
		return this->activeFlags[1] & B11100000 || this->activeFlags[2] & B00011111;
	case 4:	//F21-28
		return this->activeFlags[2] & B11100000 || this->activeFlags[3] & B00011111;
	}

	return false;
}

long dateTime = 0;

#ifdef USE_LOCOMOTIVES
void FunctionsState::functionsLoop()
{
	if (!DCCpp::mainRegs.isRegisterEmpty())  // do not add a new packet if there is a Register already waiting to be updated -- nextReg will be reset to NULL by interrupt when prior Register updated fully processed
		return;

	// Limit the number of functions sent !
	if (millis() - dateTime < 50)
		return;

	// First call, or locomotives list empty...
	if (currentLocomotive == NULL)
	{
		currentLocomotive = Locomotives::getFirstLocomotive();
		if (currentLocomotive == NULL)
			return;
	}

	currentFunctionBlock++;

	// Last block passed, jump to the next loco.
	if (currentFunctionBlock > 4)
	{
		currentLocomotive = Locomotives::getNextLocomotive(currentLocomotive);
		currentFunctionBlock = 0;
		if (currentLocomotive == NULL)
			return;
	}

	// Try to find the next acitvated functions group.
	while (currentFunctionBlock <= 4 && !currentLocomotive->functions.isFunctionBlockActivated(currentFunctionBlock))
		currentFunctionBlock++;

	// Last block passed, jump to the next loco and wait for the next loop.
	if (currentFunctionBlock > 4)
	{
		currentLocomotive = Locomotives::getNextLocomotive(currentLocomotive);
		currentFunctionBlock = 0;
	}

	// Something activated...
	if (currentLocomotive != NULL && currentLocomotive->functions.dccFirstByte[currentFunctionBlock] != -1)
	{
		DCCpp::mainRegs.setFunction(0,
			currentLocomotive->getAddress(),
			currentLocomotive->functions.dccFirstByte[currentFunctionBlock],
			currentLocomotive->functions.dccSecondByte[currentFunctionBlock],
			false);
#ifdef DCCPP_DEBUG_VERBOSE_MODE
		Serial.print(millis()-dateTime);
		Serial.print(F(" Loco #"));
		Serial.print(currentLocomotive->getAddress());
		Serial.print(F(" Block "));
		Serial.print(currentFunctionBlock);
		Serial.println("");
		dateTime = millis();
#endif
	}
}
#endif

bool FunctionsState::buildDCCF0F4bytes()
{
	/*
		To set functions F0 - F4 on(= 1) or off(= 0) :

		BYTE1 : 128 + F1 * 1 + F2 * 2 + F3 * 4 + F4 * 8 + F0 * 16
		BYTE2 : omitted
	*/

	byte oneByte1 = 128;
	bool activationChanged = false;

	for (byte func = 0; func <= 4; func++)
	{
		if (this->isActivationChanged(func))
			activationChanged = true;
		if (this->isActivated(func))
		{
			if (func == 0)
				oneByte1 += 16;
			else
				oneByte1 += (1 << (func - 1));
		}
	}

	this->dccFirstByte[0] = this->dccSecondByte[0] = -1;

	if (oneByte1 > 128)
		this->dccFirstByte[0] = oneByte1;

	return activationChanged;
}

bool FunctionsState::buildDCCF5F8bytes()
{
	/*
		To set functions F5 - F8 on(= 1) or off(= 0) :

		BYTE1 : 176 + F5 * 1 + F6 * 2 + F7 * 4 + F8 * 8
		BYTE2 : omitted
	*/

	byte twoByte1 = 176;
	bool activationChanged = false;

	for (byte func = 5; func <= 8; func++)
	{
		if (this->isActivationChanged(func))
			activationChanged = true;
		if (this->isActivated(func))
			twoByte1 += (1 << (func - 5));
	}

	this->dccFirstByte[1] = this->dccSecondByte[1] = -1;

	if (twoByte1 > 176)
		this->dccFirstByte[1] = twoByte1;

	return activationChanged;
}

bool FunctionsState::buildDCCF9F12bytes()
{
	/*
		To set functions F9 - F12 on(= 1) or off(= 0) :

		BYTE1 : 160 + F9 * 1 + F10 * 2 + F11 * 4 + F12 * 8
		BYTE2 : omitted
	*/

	byte threeByte1 = 160;
	bool activationChanged = false;

	for (byte func = 9; func <= 12; func++)
	{
		if (this->isActivationChanged(func))
			activationChanged = true;
		if (this->isActivated(func))
			threeByte1 += (1 << (func - 9));
	}

	this->dccFirstByte[2] = this->dccSecondByte[2] = -1;

	if (threeByte1 > 160)
		this->dccFirstByte[2] = threeByte1;

	return activationChanged;
}

bool FunctionsState::buildDCCF13F20bytes()
{
	/*
			To set functions F13 - F20 on(= 1) or off(= 0) :

			BYTE1 : 222
			BYTE2 : F13 * 1 + F14 * 2 + F15 * 4 + F16 * 8 + F17 * 16 + F18 * 32 + F19 * 64 + F20 * 128
	*/

	byte fourByte2 = 0;
	bool activationChanged = false;

	for (byte func = 13; func <= 20; func++)
	{
		if (this->isActivationChanged(func))
			activationChanged = true;
		if (this->isActivated(func))
			fourByte2 += (1 << (func - 13));
	}

	this->dccFirstByte[3] = this->dccSecondByte[3] = -1;

	if (fourByte2 > 0)
	{
		this->dccFirstByte[3] = 222;
		this->dccSecondByte[3] = fourByte2;
	}

	return activationChanged;
}

bool FunctionsState::buildDCCF21F28bytes()
{
	/*
		To set functions F21 - F28 on(= 1) of off(= 0) :

		BYTE1 : 223
		BYTE2 : F21 * 1 + F22 * 2 + F23 * 4 + F24 * 8 + F25 * 16 + F26 * 32 + F27 * 64 + F28 * 128
	*/

	byte fiveByte2 = 0;
	bool activationChanged = false;

	for (byte func = 21; func <= 28; func++)
	{
		if (this->isActivationChanged(func))
			activationChanged = true;
		if (this->isActivated(func))
			fiveByte2 += (1 << (func - 21));
	}

	this->dccFirstByte[4] = this->dccSecondByte[4] = -1;

	if (fiveByte2 > 0)
	{
		this->dccFirstByte[4] = 223;
		this->dccSecondByte[4] = fiveByte2;
	}

	return activationChanged;
}

bool FunctionsState::buildDCCbytes(byte block, int* pByte1, int* pByte2, bool storeBytes)
{
	byte flags = 0;

	byte oneByte1 = 128;	// Group one functions F0-F4
	byte twoByte1 = 176;	// Group two F5-F8
	byte threeByte1 = 160;	// Group three F9-F12
	byte fourByte2 = 0;		// Group four F13-F20
	byte fiveByte2 = 0;		// Group five F21-F28

	for (byte func = 0; func <= 28; func++)
	{
		if (func <= 4 && block == 0)
		{
			/*
				To set functions F0 - F4 on(= 1) or off(= 0) :

					 BYTE1 : 128 + F1 * 1 + F2 * 2 + F3 * 4 + F4 * 8 + F0 * 16
				BYTE2 : omitted
			*/

			if (this->isActivationChanged(func))
				flags |= 1;
			if (this->isActivated(func))
			{
				if (func == 0)
					oneByte1 += 16;
				else
					oneByte1 += (1 << (func - 1));
			}
		}
		else if (func <= 8 && block == 1)
		{
			/*
				To set functions F5 - F8 on(= 1) or off(= 0) :

					 BYTE1 : 176 + F5 * 1 + F6 * 2 + F7 * 4 + F8 * 8
				BYTE2 : omitted
			*/

			if (this->isActivationChanged(func))
				flags |= 2;
			if (this->isActivated(func))
				twoByte1 += (1 << (func - 5));
		}
		else if (func <= 12 && block == 2)
		{
			/*
					 To set functions F9 - F12 on(= 1) or off(= 0) :

					 BYTE1 : 160 + F9 * 1 + F10 * 2 + F11 * 4 + F12 * 8
				BYTE2 : omitted
			*/

			if (this->isActivationChanged(func))
				flags |= 4;
			if (this->isActivated(func))
				threeByte1 += (1 << (func - 9));
		}
		else if (func <= 20 && block == 3)
		{
			/*
					 To set functions F13 - F20 on(= 1) or off(= 0) :

					 BYTE1 : 222
				BYTE2 : F13 * 1 + F14 * 2 + F15 * 4 + F16 * 8 + F17 * 16 + F18 * 32 + F19 * 64 + F20 * 128
			*/

			if (this->isActivationChanged(func))
				flags |= 8;
			if (this->isActivated(func))
				fourByte2 += (1 << (func - 13));
		}
		else if (func <= 28 && block == 4)
		{
			/*
					 To set functions F21 - F28 on(= 1) of off(= 0) :

					 BYTE1 : 223
				BYTE2 : F21 * 1 + F22 * 2 + F23 * 4 + F24 * 8 + F25 * 16 + F26 * 32 + F27 * 64 + F28 * 128
			*/

			if (this->isActivationChanged(func))
				flags |= 16;
			if (this->isActivated(func))
				fiveByte2 += (1 << (func - 21));
		}
	}

	*pByte1 = *pByte2 = -1;

	if (flags & 1)
		*pByte1 = oneByte1;
	if (flags & 2)
		*pByte1 = twoByte1;
	if (flags & 4)
		*pByte1 = threeByte1;
	if (flags & 8)
	{
		*pByte1 = 222;
		*pByte2 = fourByte2;
	}
	if (flags & 16)
	{
		*pByte1 = 223;
		*pByte2 = fiveByte2;
	}

	if (storeBytes)
	{
		this->dccFirstByte[block] = *pByte1;
		this->dccSecondByte[block] = *pByte2;
	}

	return flags > 0;
}

#ifdef USE_LOCOMOTIVES
bool FunctionsState::Save(JsonObject inLoco)
{
	JsonArray functions = inLoco.createNestedArray("Functions");
	functions.add(this->activeFlags[0]);
	functions.add(this->activeFlags[1]);
	functions.add(this->activeFlags[2]);
	functions.add(this->activeFlags[3]);

	return true;
}
#endif

#ifdef DCCPP_DEBUG_MODE
void FunctionsState::printActivated()
{
	for (int i = 0; i < 32; i++)
	{
		if (this->isActivated(i))
		{
			Serial.print(i);
			Serial.print(" ");
		}
	}

	Serial.println("");
}

bool FunctionsState::test()
{
	FunctionsState test;
	bool ret = false;
	
	ret = test.isFunctionBlockActivated(0);	// false;
	ret = test.isFunctionBlockActivated(1);	// false;
	ret = test.isFunctionBlockActivated(2);	// false;
	ret = test.isFunctionBlockActivated(3);	// false;
	ret = test.isFunctionBlockActivated(4);	// false;

	// block 0
	test.activate(0);
	ret = test.isFunctionBlockActivated(0);	// true;
	ret = test.isFunctionBlockActivated(1);	// false;

	test.activate(4);
	ret = test.isFunctionBlockActivated(0);	// true;
	ret = test.isFunctionBlockActivated(1);	// false;

	test.inactivate(0);
	ret = test.isFunctionBlockActivated(0);	// true;
	ret = test.isFunctionBlockActivated(1);	// false;
	test.inactivate(4);

	// block 1
	test.activate(5);
	ret = test.isFunctionBlockActivated(0);	// false;
	ret = test.isFunctionBlockActivated(1);	// true;
	ret = test.isFunctionBlockActivated(2);	// false;

	test.activate(8);
	ret = test.isFunctionBlockActivated(0);	// false;
	ret = test.isFunctionBlockActivated(1);	// true;
	ret = test.isFunctionBlockActivated(2);	// false;

	test.inactivate(5);
	ret = test.isFunctionBlockActivated(0);	// false;
	ret = test.isFunctionBlockActivated(1);	// true;
	ret = test.isFunctionBlockActivated(2);	// false;
	test.inactivate(8);

	// block 2
	test.activate(9);
	ret = test.isFunctionBlockActivated(1);	// false;
	ret = test.isFunctionBlockActivated(2);	// true;
	ret = test.isFunctionBlockActivated(3);	// false;

	test.activate(12);
	ret = test.isFunctionBlockActivated(1);	// false;
	ret = test.isFunctionBlockActivated(2);	// true;
	ret = test.isFunctionBlockActivated(3);	// false;

	test.inactivate(9);
	ret = test.isFunctionBlockActivated(1);	// false;
	ret = test.isFunctionBlockActivated(2);	// true;
	ret = test.isFunctionBlockActivated(3);	// false;
	test.inactivate(12);

	// block 3
	test.activate(13);
	ret = test.isFunctionBlockActivated(2);	// false;
	ret = test.isFunctionBlockActivated(3);	// true;
	ret = test.isFunctionBlockActivated(4);	// false;

	test.activate(20);
	ret = test.isFunctionBlockActivated(2);	// false;
	ret = test.isFunctionBlockActivated(3);	// true;
	ret = test.isFunctionBlockActivated(4);	// false;

	test.inactivate(13);
	ret = test.isFunctionBlockActivated(2);	// false;
	ret = test.isFunctionBlockActivated(3);	// true;
	ret = test.isFunctionBlockActivated(4);	// false;
	test.inactivate(20);

	// block 4
	test.activate(21);
	ret = test.isFunctionBlockActivated(3);	// false;
	ret = test.isFunctionBlockActivated(4);	// true;

	test.activate(28);
	ret = test.isFunctionBlockActivated(3);	// false;
	ret = test.isFunctionBlockActivated(4);	// true;

	test.inactivate(21);
	ret = test.isFunctionBlockActivated(3);	// false;
	ret = test.isFunctionBlockActivated(4);	// true;
	test.inactivate(28);

	return ret;
}
#endif
