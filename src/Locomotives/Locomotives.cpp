/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <FunctionsState class>
*************************************************************/

#include "Arduino.h"
#include "DCCpp.h"

#ifdef USE_LOCOMOTIVES
Locomotive* Locomotives::pFirstLocomotive = NULL;

Locomotive* Locomotives::get(uint16_t inAddress)
{
	Locomotive* pCurr = pFirstLocomotive;

	while (pCurr != NULL)
	{
		if (pCurr->getAddress() == inAddress)
		{
			return pCurr;
		}
		pCurr = pCurr->pNextLocomotive;
	}

	return NULL;
}

Locomotive* Locomotives::get(const String& inName)
{
	Locomotive* pCurr = pFirstLocomotive;

	while (pCurr != NULL)
	{
		if (pCurr->getName() ==  inName)
		{
			return pCurr;
		}
		pCurr = pCurr->pNextLocomotive;
	}

	return NULL;
}

Locomotive* Locomotives::add(const String& inName, uint16_t inAddress, uint8_t inSpeedMax)
{
	Locomotive* pNewLocomotive = NULL;

	uint8_t speedReg = Registers::allocateRegister();

	if (speedReg == 0)
	{
		// No more registers available.
		return NULL;
	}

	pNewLocomotive = new Locomotive(inName, speedReg, inAddress, inSpeedMax);

	if (pNewLocomotive != NULL)
	{
		if (pFirstLocomotive == NULL)
		{
			// First loco in the list !
			pFirstLocomotive = pNewLocomotive;
			return pNewLocomotive;
		}

		// Look for the last loco, and add the new at the end of the list.
		Locomotive* pCurr = pFirstLocomotive;

		while (pCurr != NULL)
		{
			if (pCurr->pNextLocomotive == NULL)
			{
				pCurr->pNextLocomotive = pNewLocomotive;
				return pNewLocomotive;
			}
			pCurr = pCurr->pNextLocomotive;
		}
	}

	Registers::freeRegister(speedReg);

	return NULL;
}

void Locomotives::remove(uint16_t inAddress)
{
	Locomotive* pCurr = pFirstLocomotive;
	Locomotive* pPrev = NULL;

	// look for the loco to remove
	while (pCurr != NULL)
	{
		if (pCurr->getAddress() == inAddress)
		{
			if (pPrev == NULL)
			{
				// if this is the first loco of the list, the only loco of the list or not.
				pFirstLocomotive = pCurr->pNextLocomotive;
			}
			else
			{
				pPrev->pNextLocomotive = pCurr->pNextLocomotive;
			}

			delete pCurr;
			return;
		}
		pPrev = pCurr;
		pCurr = pCurr->pNextLocomotive;
	}
}

void Locomotives::remove(const String& inName)
{
	Locomotive* pToRemove = get(inName);

	if (pToRemove != NULL)
	{
		remove(pToRemove->getAddress());
	}
}

int Locomotives::count()
{
	uint8_t count = 0;

	Locomotive* pCurr = pFirstLocomotive;

	while (pCurr != NULL)
	{
		count++;
		pCurr = pCurr->pNextLocomotive;
	}

	return count;
}

#ifdef DCCPP_DEBUG_MODE
/** Print the list of assigned locomotives.
@remark Only available if DCCPP_DEBUG_MODE is defined.
*/
void Locomotives::printLocomotives()
{
	Locomotive* pCurr = pFirstLocomotive;
	uint8_t count = 1;

	while (pCurr != NULL)
	{
		Serial.print(count);
		Serial.print(" : ");
		pCurr->printLocomotive();
		count++;
		pCurr = pCurr->pNextLocomotive;
	}
}
#endif
#endif