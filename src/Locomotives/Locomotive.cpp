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
	this->direction = true;	// goes forward
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
	this->direction = true;	// goes forward
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
	this->setSpeed(speed);
	DCCpp::setSpeedMain(this->speedRegisterNumber, this->address, this->getSpeedMax(), this->currentSpeed, this->direction);
}

void Locomotive::setDCCDirection(bool inForward)
{
	if (this->direction != inForward)
	{
		this->setDirection(inForward);
		DCCpp::setSpeedMain(this->speedRegisterNumber, this->address, this->getSpeedMax(), 0, this->direction);
	}
}

void Locomotive::stop()
{
	this->setSpeed(0);
	DCCpp::setSpeedMain(this->speedRegisterNumber, this->address, this->getSpeedMax(), this->currentSpeed, this->direction);
}

void Locomotive::emergencyStop()
{
	this->setSpeed(1);
	DCCpp::setSpeedMain(this->speedRegisterNumber, this->address, this->getSpeedMax(), this->currentSpeed, this->direction);
}

void Locomotive::setDCCFunction(uint8_t inFunction, bool inActivate)
{
	this->setFunction(inFunction, inActivate);
	DCCpp::setFunctionsMain(0, this->address, this->functions);
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
	Serial.print(this->currentSpeed * (this->direction == true ? 1 : -1));

	Serial.print("      functions: ");
	this->functions.printActivated();
}
#endif
#endif