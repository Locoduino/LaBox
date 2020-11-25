/*************************************************************
project: <DCCpp library extension>
author: <Thierry PARIS>
description: <Registers class>
*************************************************************/

#include "Arduino.h"
#include "DCCpp.h"

#ifdef USE_LOCOMOTIVES
byte Registers::registers[MAXIMUM_REGISTER_SIZE / 8];

void Registers::clear()
{
	for (int i = 0; i < MAXIMUM_REGISTER_SIZE / 8; i++)
		registers[i] = 0;
}

uint8_t Registers::allocateRegister()
{
	for (int i = 1; i < MAXIMUM_REGISTER_SIZE; i++)
	{
		if (bitRead(registers[i / 8], i % 8) == 0)
		{
			bitSet(registers[i / 8], i % 8);
			return i;
		}
	}

	return 0;	// No register free !
}

void Registers::freeRegister(uint8_t inRegister)
{
	if (inRegister < 1)
	{
		return;
	}

	bitClear(registers[inRegister / 8], inRegister % 8);
}

bool Registers::isAllocated(byte inRegister)
{
	if (inRegister == 0)
	{
		return true;
	}

	return bitRead(registers[inRegister / 8], inRegister % 8);
}

#ifdef DCCPP_DEBUG_MODE
#ifdef VISUALSTUDIO
/** Unit test function
*/
void Registers::Test()
{
	for (int i = 1; i < MAXIMUM_REGISTER_SIZE; i++)
	{
		allocateRegister();
	}

	Serial.println("After all initialized");
	printRegisters();

	for (int i = 1; i < MAXIMUM_REGISTER_SIZE; i++)
	{
		freeRegister(i);
	}

	Serial.println("After all freed");
	printRegisters();

	for (int i = 1; i < MAXIMUM_REGISTER_SIZE/4; i++)
	{
		allocateRegister();
	}

	Serial.println("After 128 / 4 initialized");
	printRegisters();

	freeRegister(0);	// error !
	freeRegister(1);
	freeRegister(10);
	freeRegister(MAXIMUM_REGISTER_SIZE/5);

	Serial.println("After 1, 10 and 25 freed");
	printRegisters();

	int reg = Registers::allocateRegister();
	Serial.println("Allocate first free (1)");
	printRegisters();
	reg = Registers::allocateRegister();
	Serial.println("Allocate first free (10)");
	printRegisters();
	reg = Registers::allocateRegister();
	Serial.println("Allocate first free (25)");
	printRegisters();
	reg = Registers::allocateRegister();
	Serial.println("Allocate first free (32)");
	printRegisters();
}
#endif

void Registers::printRegisters()
{
	for (int i = 1; i < MAXIMUM_REGISTER_SIZE; i++)
	{
		if (isAllocated(i))
		{
			Serial.print(i);
			Serial.print(" ");
		}
	}

	Serial.println("");
}
#endif
#endif