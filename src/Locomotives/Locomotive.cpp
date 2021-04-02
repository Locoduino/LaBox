/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <FunctionsState class>
*************************************************************/

#include "Arduino.h"
#include "DCCpp.h"

#ifdef USE_LOCOMOTIVES

Locomotive::Locomotive()
{
	this->initialize();
}

Locomotive::Locomotive(const String& inName, uint8_t inSpeedRegister, uint16_t inAddress, uint8_t inSpeedMax)
{
	// 'static' data, not updated during the run.
	this->name = inName;
	this->address = inAddress;
	this->speedMax = inSpeedMax;
	this->speedRegisterNumber = inSpeedRegister;

	// Variable data
	this->currentSpeed = 0; // regular stop
	this->forwardDirection = true;	// goes forward
	this->tag = -1;
	this->pNextLocomotive = NULL;
}

void Locomotive::initialize()
{
	// 'static' data, not updated during the run.
	this->name[0] = 0;
	this->address = 0;
	this->speedMax = 128;
	this->speedRegisterNumber = 0;

	// Variable data
	this->currentSpeed = 0; // regular stop
	this->forwardDirection = true;	// goes forward
	this->tag = -1;
	this->pNextLocomotive = NULL;
	this->functions.clear();
}

void Locomotive::setFunction(uint8_t inFunction, bool inActivate)
{
	if (inActivate)
		this->functions.activate(inFunction);
	else
		this->functions.inactivate(inFunction);
}

void Locomotive::setDCCSpeed(uint8_t speed)
{
	this->currentSpeed = speed;
	DCCpp::setSpeedMain(this->speedRegisterNumber, this->address, this->getSpeedMax(), this->currentSpeed, this->forwardDirection);
	if (Locomotives::notifySpeedDir != NULL)
		Locomotives::notifySpeedDir(this->address, speed, this->forwardDirection);
}

void Locomotive::setDCCDirection(bool inForward)
{
	if (this->forwardDirection != inForward)
	{
		this->setDirection(inForward);
		DCCpp::setSpeedMain(this->speedRegisterNumber, this->address, this->getSpeedMax(), 0, this->forwardDirection);
		if (Locomotives::notifySpeedDir != NULL)
			Locomotives::notifySpeedDir(this->address, 0, this->forwardDirection);
	}
}

void Locomotive::stop()
{
	DCCpp::setSpeedMain(this->speedRegisterNumber, this->address, this->getSpeedMax(), this->currentSpeed, this->forwardDirection);
	if (Locomotives::notifySpeedDir != NULL)
		Locomotives::notifySpeedDir(this->address, this->currentSpeed, this->forwardDirection);
}

void Locomotive::emergencyStop()
{
	this->currentSpeed = 0;
	DCCpp::setSpeedMain(this->speedRegisterNumber, this->address, this->getSpeedMax(), 1, this->forwardDirection);
	if (Locomotives::notifyEmergencyStop!= NULL)
		Locomotives::notifyEmergencyStop(this->address, true);
}

void Locomotive::setDCCFunction(uint8_t inFunction, bool inActivate)
{
	this->setFunction(inFunction, inActivate);
	DCCpp::setFunctionsMain(0, this->address, this->functions);
	if (Locomotives::notifyFunction != NULL)
		Locomotives::notifyFunction(this->address, inFunction, inActivate);
}

bool Locomotive::Save(JsonObject inLoco)
{
	inLoco["name"] = this->name;
	inLoco["Id"] = this->address;
	inLoco["Register"] = this->speedRegisterNumber;
	inLoco["SpeedMax"] = this->speedMax;
	inLoco["Speed"] = this->currentSpeed;
	inLoco["direction"] = this->forwardDirection;

	this->functions.Save(inLoco);

	return true;
}

#ifdef DCCPP_DEBUG_MODE
void Locomotive::printLocomotive()
{
	Serial.print(this->name);
	Serial.print(" reg:");
	Serial.print(this->speedRegisterNumber);
	Serial.print(" id:");
	Serial.print(this->address);
	Serial.print(" max:");
	Serial.print(this->speedMax);

	Serial.print("      +/-speed:");
	Serial.print(this->currentSpeed * (this->forwardDirection == true ? 1 : -1));

	Serial.print("      functions: ");
	this->functions.printActivated();
}
#endif
#endif