/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <FunctionsState class>
*************************************************************/

#include "Arduino.h"
#include "DCCpp.h"

#ifdef USE_THROTTLES
Throttle* Throttles::pFirstThrottle = NULL;
uint16_t Throttles::throttlesCounter = 0;

Throttle* Throttles::getFirst()
{		
	return pFirstThrottle;
}

Throttle* Throttles::get(uint16_t inId)
	{
		Throttle* pCurr = pFirstThrottle;

	while (pCurr != NULL)
	{
		if (pCurr->getId() == inId)
		{
			return pCurr;
		}
		pCurr = pCurr->pNextThrottle;
	}

	return NULL;
}

Throttle* Throttles::get(const char* inName)
{
	Throttle* pCurr = pFirstThrottle;

	while (pCurr != NULL)
	{
		if (strcmp(pCurr->getName(), inName) == 0)
		{
			return pCurr;
		}
		pCurr = pCurr->pNextThrottle;
	}

	return NULL;
}

bool Throttles::add(Throttle *inpThrottle)
{
	if (inpThrottle != NULL)
	{
		inpThrottle->setId(throttlesCounter++);
		if (pFirstThrottle == NULL)
		{
			// First throttle in the list !
			pFirstThrottle = inpThrottle;
			return true;
		}

		// Look for the last throttle, and add the new at the end of the list.
		Throttle* pCurr = pFirstThrottle;

		while (pCurr != NULL)
		{
			if (pCurr->pNextThrottle == NULL)
			{
				pCurr->pNextThrottle = inpThrottle;
				return true;
			}
			pCurr = pCurr->pNextThrottle;
		}
	}

	return false;
}

void Throttles::remove(uint16_t inId)
{
	Throttle* pCurr = pFirstThrottle;
	Throttle* pPrev = NULL;

	// look for the loco to remove
	while (pCurr != NULL)
	{
		if (pCurr->getId() == inId)
		{
			if (pPrev == NULL)
			{
				// if this is the first loco of the list, the only loco of the list or not.
				pFirstThrottle = pCurr->pNextThrottle;
			}
			else
			{
				pPrev->pNextThrottle = pCurr->pNextThrottle;
			}

			delete pCurr;
			return;
		}
		pPrev = pCurr;
		pCurr = pCurr->pNextThrottle;
	}
}

void Throttles::remove(const char* inName)
{
	Throttle* pToRemove = get(inName);

	if (pToRemove != NULL)
	{
		remove(pToRemove->getId());
	}
}

int Throttles::count()
{
	uint8_t count = 0;

	Throttle* pCurr = pFirstThrottle;

	while (pCurr != NULL)
	{
		count++;
		pCurr = pCurr->pNextThrottle;
	}

	return count;
}

#ifdef DCCPP_DEBUG_MODE
/** Print the list of assigned Throttles.
@remark Only available if DCCPP_DEBUG_MODE is defined.
*/
void Throttles::printThrottles()
{
	Throttle* pCurr = pFirstThrottle;
	uint8_t count = 1;

	while (pCurr != NULL)
	{
		Serial.print(count);
		Serial.print(" : ");
		pCurr->printThrottle();
		count++;
		pCurr = pCurr->pNextThrottle;
	}
}
#endif
#endif