/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <FunctionsState class>
*************************************************************/

#include "Arduino.h"
#include "DCCpp.h"

#ifdef USE_LOCOMOTIVES

#include "FS.h"
#include "SPIFFS.h"

/* You only need to format SPIFFS the first time you run a
	 test or else use the SPIFFS plugin to create a partition
	 https://github.com/me-no-dev/arduino-esp32fs-plugin */
#define FORMAT_SPIFFS_IF_FAILED true

notifySpeedDirType Locomotives::notifySpeedDir = NULL;
notifyFunctionType Locomotives::notifyFunction = NULL;
notifyEmergencyStopType Locomotives::notifyEmergencyStop = NULL;

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

Locomotive* Locomotives::getByRegister(int8_t inRegister)
{
	Locomotive* pCurr = pFirstLocomotive;

	while (pCurr != NULL)
	{
		if (pCurr->getSpeedRegister() == inRegister)
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

Locomotive* Locomotives::add(const String& inName, uint16_t inAddress, uint8_t inSpeedMax, int8_t inRegister)
{
	Locomotive* pNewLocomotive = NULL;

	if (inRegister == 0)
	{
		inRegister = Registers::allocateRegister();
	}

	if (inRegister == 0)
	{
		// No more registers available.
		return NULL;
	}

	pNewLocomotive = new Locomotive(inName, inRegister, inAddress, inSpeedMax);

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

			Registers::freeRegister(pCurr->getSpeedRegister());

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

Locomotive* Locomotives::getNextLocomotive(Locomotive* inCurrent)
{
	if (inCurrent != NULL && inCurrent->pNextLocomotive != NULL)
		return inCurrent->pNextLocomotive;

	return pFirstLocomotive;
}

void Locomotives::emergencyStop()
{
	Locomotive* pCurr = pFirstLocomotive;

	while (pCurr != NULL)
	{
		pCurr->emergencyStop();
		pCurr = pCurr->pNextLocomotive;
	}
}

bool Locomotives::SaveAll()
{
	// SPIFFS begin
	if (SPIFFS.totalBytes() == 0)
		if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
			Serial.println("SPIFFS Mount Failed");
			return false;
		}

	// New file created
	File file = SPIFFS.open("/Locomotives.json", FILE_WRITE);
	if (!file) {
		Serial.println("- failed to open file for writing");
		return false;
	}

	//////////////////// Json data building
	StaticJsonDocument<4096> locoDocument;

	JsonArray data = locoDocument.createNestedArray("Locomotives");

	Locomotive* pCurr = pFirstLocomotive;

	while (pCurr != NULL)
	{
		JsonObject loco = data.createNestedObject();

		pCurr->Save(loco);
		//bool ret = data.add(loco);

		pCurr = pCurr->pNextLocomotive;
	}

	char buffer[4096];

#ifdef VISUALSTUDIO
	serializeJsonPretty(locoDocument, buffer);
#else
	serializeJson(locoDocument, buffer);
#endif

	// Write file
	if (file.print(buffer)) {
		Serial.println("- file written");
	}
	else {
		Serial.println("- file.print failed");
	}

	return true;
}

#ifdef DCCPP_DEBUG_MODE
/** Print the list of assigned locomotives.
@remark Only available if DCCPP_DEBUG_MODE is defined.
*/
void Locomotives::printLocomotives()
{
	Locomotive* pCurr = pFirstLocomotive;
	uint8_t count = 0;

	Serial.println("Locomotives ------------------");

	while (pCurr != NULL)
	{
		Serial.print(count);
		Serial.print(" : ");
		pCurr->printLocomotive();
		count++;
		pCurr = pCurr->pNextLocomotive;
	}
}

void Locomotives::debugLocomotivesSet()
{
	Locomotive *pLoco = NULL;
	
	pLoco = Locomotives::add("BB65000", 3, 128);
	pLoco->setSpeed(0);
	pLoco->setDirection(1);	
	//pLoco->setFunction(0, true);
	//pLoco->setFunction(28, true);

	pLoco = Locomotives::add("CC72000", 72, 128);
	pLoco->setSpeed(127);
	pLoco->setDirection(0);
	pLoco->setFunction(6, true);
	pLoco->setFunction(11, true);

	pLoco = Locomotives::add("150x", 150, 128);
	pLoco->setSpeed(30);
	pLoco->setDirection(1);

	pLoco = Locomotives::add("Y7200", 7200, 14);
	pLoco->setSpeed(30);		// Error !
	pLoco->setDirection(0);
}
#endif
#endif