/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <Throttle Automation class>
*************************************************************/

#include "DCCpp.h"

#if defined(USE_TEXTCOMMAND) && defined(USE_THROTTLES)

// ITEM

ThrottleAutomationItem::ThrottleAutomationItem(int inDelay, const char* inCommand, const char* inComment)
{
	this->delay = inDelay;
	strcpy(this->command, inCommand);
	strcpy(this->comment, inComment);
	this->next = NULL;
}

ThrottleAutomation::ThrottleAutomation(const String& inName) : Throttle(inName)
{
	this->first = NULL;
	this->currentItem = NULL;
	this->timeValue = 0;
	this->currentState = stopped;
}

// THROTTLE

bool ThrottleAutomation::sendMessage(const String& inMessage)
{
	return true;
}

bool ThrottleAutomation::isConnected()
{
	return true;
}

ThrottleAutomationItem* ThrottleAutomation::GetLastItem()
{
	ThrottleAutomationItem* curr = first;
	while (curr != NULL && curr->next != NULL)
		curr = curr->next;

	return curr;
}

void ThrottleAutomation::AddItem(int inDelay, const char* inCommand, const char* inComment)
{
	ThrottleAutomationItem* pItem = new ThrottleAutomationItem(inDelay, inCommand, inComment);
	if (first == NULL)
		first = pItem;
	else
		GetLastItem()->next = pItem;
}

bool ThrottleAutomation::begin(EthernetProtocol inProtocol)
{
	this->currentItem = NULL;
	this->timeValue = 0;
	this->currentState = stopped;
	this->type = TYPEAUTOMATION;

	return true;
}

bool ThrottleAutomation::loop()
{
	bool added = false;

	if (this->type == NOTSTARTEDTHROTTLE)
		return false;

	if (this->currentState == started)
	{
		if (currentItem == NULL)
			currentItem = this->first;
		else
		{
			if (currentItem->delay >= AUTOMATIONIDSSTART)
			{
#ifdef USE_SENSOR

				if (ISSENSOR(currentItem->delay))
				{
					int id = SENSORID(currentItem->delay);

					Sensor* pSensor = Sensor::get(id);

					if (pSensor != NULL)
					{
						int state = SENSORSTATE(currentItem->delay);
						if (state == HIGH && !pSensor->isActive())
						{
							return false;
						}
						if (state == LOW && pSensor->isActive())
						{
							return false;
						}
					}
				}
#endif
				if (ISPIN(currentItem->delay))
				{
					int pinState = digitalRead(PINNUMBER(currentItem->delay));
					int state = PINSTATE(currentItem->delay);
					if (state != pinState)
					{
						return false;
					}
				}
			}
			else
			{
				if ((int)(millis() - timeValue) < currentItem->delay)
					return false;
			}
			currentItem = currentItem->next;
			if (currentItem == NULL)
			{
				currentItem = first;
#ifdef DCCPP_DEBUG_MODE
				Serial.println("*** Restart");
#endif
			}
		}
		if (currentItem->delay < AUTOMATIONIDSSTART)
			timeValue = millis();
		Throttle::pushMessageInStack(this->id, currentItem->command);
#ifdef DCCPP_DEBUG_MODE
		Serial.println(currentItem->comment);
#endif
	}

	return added;
}

void ThrottleAutomation::end()
{
}

void ThrottleAutomation::Start()
{
	this->currentItem = NULL;
	this->timeValue = 0;
	this->currentState = started;
}

void ThrottleAutomation::Stop()
{
	this->currentState = stopped;
}

void ThrottleAutomation::Pause()
{
	this->currentState = paused;
}

void ThrottleAutomation::ReStart()
{
	this->currentState = started;
}

#ifdef DCCPP_DEBUG_MODE
void ThrottleAutomation::printThrottle()
{
	Serial.print(this->id);
	Serial.print(" : ");
	Serial.print(this->name);
	if (this->pConverter != NULL)
	{
		Serial.print(" (");
		this->pConverter->printConverter();
		Serial.print(") ");
	}

	Serial.println("");
}

void ThrottleAutomation::printThrottleItems()
{
	Serial.print(this->name);
	Serial.println(" items :");

	ThrottleAutomationItem* curr = first;
	while (curr != NULL && curr->next != NULL)
	{
		if (curr->delay >= AUTOMATIONIDSSTART)
		{
#if USE_SENSOR
			if (ISSENSOR(curr->delay))
			{
				Serial.print("  When sensor ");
				Serial.print(SENSORID(curr->delay));
				Serial.print(" is ");
				Serial.print(SENSORSTATE(curr->delay) ? "HIGH" : "LOW");
				Serial.print(" do ");
			}
#endif
			if (ISPIN(curr->delay))
			{
				Serial.print("  When pin ");
				Serial.print(PINNUMBER(curr->delay));
				Serial.print(" is ");
				Serial.print(PINSTATE(curr->delay) ? "HIGH" : "LOW");
				Serial.print(" do ");
			}
		}
		else
		{
			Serial.print("  After ");
			Serial.print(curr->delay);
			Serial.print("ms do ");
		}
		Serial.print(curr->command);
		Serial.print(" (");
		Serial.print(curr->comment);
		Serial.println(")");

		curr = curr->next;
	}
}

#endif
#endif
