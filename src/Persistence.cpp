/*************************************************************
project: <LaBox library>
author: <Thierry PARIS>
description: <Persistence>
*************************************************************/

#include "Arduino.h"
#include "Persistence.h"

PersistenceFile::PersistenceFile()
{
	this->pFirstObject = NULL;
}

PersistenceObject* PersistenceFile::getFirst()
{		
	return this->pFirstObject;
}

bool PersistenceFile::write()
{
	PersistenceObject* pCurr = this->pFirstObject;

	while (pCurr != NULL)
	{
		if (!pCurr->write(this))
			return false;

		pCurr = pCurr->pNextObject;
	}

	return true;
}

bool PersistenceFile::read()
{
	PersistenceObject* pCurr = this->pFirstObject;

	while (pCurr != NULL)
	{
		if (!pCurr->read(this))
			return false;

		pCurr = pCurr->pNextObject;
	}

	return true;
}

void PersistenceFile::add(PersistenceObject* inpObject)
{
	if (inpObject != NULL)
	{
		if (this->pFirstObject == NULL)
		{
			// First throttle in the list !
			this->pFirstObject = inpObject;
			return;
		}

		// Look for the last throttle, and add the new at the end of the list.
		PersistenceObject* pCurr = this->pFirstObject;

		while (pCurr != NULL)
		{
			if (pCurr->pNextObject== NULL)
			{
				pCurr->pNextObject = inpObject;
				return;
			}
			pCurr = pCurr->pNextObject;
		}
	}
}

bool PersistenceFile::remove(PersistenceObject* inpObject)
{
	PersistenceObject* pCurr = this->pFirstObject;
	PersistenceObject* pPrev = NULL;

	// look for the loco to remove
	while (pCurr != NULL)
	{
		if (pCurr == inpObject)
		{
			if (pPrev == NULL)
			{
				// if this is the first loco of the list, the only loco of the list or not.
				this->pFirstObject = pCurr->pNextObject;
			}
			else
			{
				pPrev->pNextObject = pCurr->pNextObject;
			}

			delete pCurr;
			return true;
		}
		pPrev = pCurr;
		pCurr = pCurr->pNextObject;
	}

	return false;	// object not found !
}

int PersistenceFile::count()
{
	uint8_t count = 0;

	PersistenceObject* pCurr = this->pFirstObject;

	// look for the loco to remove
	while (pCurr != NULL)
	{
		count++;
		pCurr = pCurr->pNextObject;
	}

	return count;
}

#ifdef DCCPP_DEBUG_MODE
/** Print the list of assigned Throttles.
@remark Only available if DCCPP_DEBUG_MODE is defined.
*/
void PersistenceFile::printObjects()
{
	Serial.println("PersistenceObject of this file ------------------");

	PersistenceObject* pCurr = this->pFirstObject;
	int count = 0;

	// look for the loco to remove
	while (pCurr != NULL)
	{
		Serial.print(count);
		Serial.print(": ");
		Serial.print((unsigned long) pCurr, HEX);
		pCurr = pCurr->pNextObject;
	}
}
#endif
