/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <Message converter base class>
*************************************************************/

#include "Arduino.h"
#include "DCCpp.h"

#if defined(USE_THROTTLES)
#ifdef USE_TURNOUT
tData MessageConverterWiThrottle::turnouts[512];
#endif
String MessageConverterWiThrottle::command;

MessageConverterWiThrottle::MessageConverterWiThrottle()
{
	this->heartbeatEnable = false;
	for (int i = 0; i < WIMAXTHROTLLESNUMBER; i++)
	{
		this->heartbeat[i] = 0;
	}

#ifdef USE_TURNOUT
	for (int t = 0; t < 512; t++)
	{
		this->turnouts[t].address = 0;
		this->turnouts[t].id = 0;
		this->turnouts[t].tStatus = 0;
	}
#endif
}

void MessageConverterWiThrottle::clientStart(Throttle* inpThrottle)
{
	/*this->Client.flush();
	this->Client.setTimeout(500);*/
#ifdef DCCPP_DEBUG_MODE
	Serial.println("Converter : New client");
#endif
	inpThrottle->Println("VN2.0");
	inpThrottle->Println("RL0");
	inpThrottle->Println("PPA2");
	inpThrottle->Println("RCL0");
	inpThrottle->Println("PW12080n");
#ifdef USE_TURNOUT
	inpThrottle->Println("PTT]\\[Turnouts}|{Turnout]\\[Closed}|{2]\\[Thrown}|{4");
	inpThrottle->Print("PTL");
	for (int t = 0; turnouts[t].address != 0; t++)
	{
		command = "]\\[LT";
		command.concat(turnouts[t].address);
		command.concat("}|{");
		command.concat(turnouts[t].id);
		command.concat("}|{");
		command.concat(turnouts[t].tStatus);
		inpThrottle->Print(command);
	}
	inpThrottle->Println("");
#endif

	inpThrottle->setCommandCharacters('\0', 10);

	Throttles::printThrottles();
}

void MessageConverterWiThrottle::clientStop(Throttle* inpThrottle)
{
#ifdef DCCPP_DEBUG_MODE
	Serial.print("Converter : client ");
	Serial.print(inpThrottle->getName());
	Serial.println(" disconnected");
#endif

	inpThrottle->end();
	heartbeatEnable = false;
	for (int i = 0; i < WIMAXTHROTLLESNUMBER; i++)
	{
		heartbeat[i] = 0;
		locos[i].initialize();
	}

	Throttles::printThrottles();
}

void MessageConverterWiThrottle::stayAlive(Throttle* inpThrottle)
{
		checkHeartbeat(inpThrottle);
}

bool MessageConverterWiThrottle::convert(Throttle* inpThrottle, const String& inCommand)
{
#ifdef DCCPP_DEBUG_MODE
	Serial.print("Command to convert: ");
	Serial.println(inCommand);
#endif

	if (inCommand[0] == 'Q')
	{
		clientStop(inpThrottle);
		return true;
	}

	if (inCommand[0] == '*' && inCommand[1] == '+')
	{
		this->heartbeatEnable = true;
		return true;
	}

	if (inCommand[0] == '*' && inCommand[1] == '-')
	{
		this->heartbeatEnable = false;
		return true;
	}

	if (inCommand[0] == 'N' || inCommand[0] == '*')
	{
		String buff = "*";
		buff += String(HEARTBEATTIMEOUT);
		inpThrottle->println(buff);
		return true;
	}

	if (inCommand[0] == 'P' && inCommand[1] == 'P' && inCommand[2] == 'A')
	{
		String command = inCommand.substring(3);
		DCCpp::IsPowerOnMain = command.toInt() == 0 ? false : true;
		Serial.println("<" + command + ">");            // vers dcc++ <1> ou <0>

		// TODO : loop on all clients...
		inpThrottle->println("PPA" + command + "\n\n");
		return true;
	}

	if (inCommand[0] == 'P' && inCommand[1] == 'T' && inCommand[2] == 'A')
	{
		String aStatus = inCommand.substring(3, 4);
		int aAddr = inCommand.substring(6).toInt();

		accessoryToggle(aAddr, aStatus);

		return true;
	}

	if (inCommand[0] == 'M')
	{
		int locoNumber = -1;
		String locoDescriptor = inCommand.substring(1, 2);

		switch (locoDescriptor[0])
		{
			case 'T': locoNumber = 0;		break;

			case 'S': locoNumber = 1;		break;

			case '0': 
			case '1':
			case '2':
			case '3':
			case '4':
			case '5': locoNumber = locoDescriptor[0] - '0'; break;
		}

		// M0+S22<;>S22  MTAS22<;>F127  M0A*<;>F002  M1AS22<;>V100
		String action = inCommand.substring(2, 3);	// + / - / A ...
		String actionData = inCommand.substring(3);	// S22<;>S22
		int delimiter = actionData.indexOf(";");		// 4
		String actionKey = actionData.substring(0, delimiter - 1); // S22 L255 ou * pour toutes les locos
		String actionVal = actionData.substring(delimiter + 2);	// S22  L255  F027  V100
		
		switch (action[0])
		{
		case '+':	locoAdd(inpThrottle, locoNumber, locoDescriptor, actionKey);	break;				// M0+S11<;>S11
		case '-':	locoRelease(inpThrottle, locoNumber, locoDescriptor, actionKey);	break;		// M1-S12<;>S12

		case 'A':
			if (locoNumber == -1)
			{
				for (int i = 0; i < WIMAXTHROTLLESNUMBER; i++)
					this->locoAction(inpThrottle, i, locoDescriptor, actionKey, actionVal);
			}
			else
			{
				this->locoAction(inpThrottle, locoNumber, locoDescriptor, actionKey, actionVal);
			}
			//locoAction(th, actionKey, actionVal, i);

			break;
		}

		if (locoNumber != -1)
			this->heartbeat[locoNumber] = millis();
	}

	return false;
}

void MessageConverterWiThrottle::locoAction(Throttle* inpThrottle, int inLocoNumber, const String& inLocoDescriptor, const String& inActionKey, const String& inActionVal) // fonctions F0 � F28 et autres fonctions S, L ou qV, qR, R, V F ..
{
	if (inActionVal[0] == 'F')
	{
		uint8_t function = (uint8_t)inActionVal.substring(2).toInt(); // numero de fonction dans la commande
		bool activate = inActionVal[1] == '1';

		this->locos[inLocoNumber].setDCCFunction(function, activate);
		return;
	}

	if (inActionVal.startsWith("qV")) // ask for current speed  // actionKey remplace LocoThrottle[Throttle]
	{
		inpThrottle->println("M" + inLocoDescriptor + "A" + inActionKey + "<;>" + "V" + String(this->locos[inLocoNumber].getSpeed()) + "\n\n");
		return;
	} //qV

	if (inActionVal.startsWith("V")) // sets the speed (velocity)
	{
		int speed = inActionVal.substring(1).toInt(); //  vitesse
		this->locos[inLocoNumber].setDCCSpeed(speed);
		return;
	} //V

	if (inActionVal.startsWith("qR")) // ask for current direction
	{ 
		inpThrottle->println("M" + inLocoDescriptor + "A" + inActionKey + "<;>" + "R" + (this->locos[inLocoNumber].isDirectionForward()?"1":"0") + "\n\n");
	} //qR

	if (inActionVal.startsWith("R"))   // set direction.
	{
		int dir = inActionVal.substring(1).toInt();
		this->locos[inLocoNumber].setDCCDirection(dir > 0);  // arret loco obligatoire (mais le curseur de WT ne revient pas � 0 !!)
		return;
	} //R

	if (inActionVal.startsWith("X"))   // emergency STOP : set speed to 1
	{
		this->locos[inLocoNumber].emergencyStop();
	} //X

	if (inActionVal.startsWith("I"))  // IDLE : set speed to 0
	{
		this->locos[inLocoNumber].stop();
	} //I

	if (inActionVal.startsWith("Q")) // the loco QUIT, set speed to 0
	{  
		this->locos[inLocoNumber].setDCCSpeed(0);
	} //Q
}

void MessageConverterWiThrottle::checkHeartbeat(Throttle* inpThrottle)
{
	if (heartbeatEnable)
	{
		for (int i = 0; i < WIMAXTHROTLLESNUMBER; i++)
		{
			if (this->locos[i].getAddress() != 0)
			{
				if (heartbeat[i] > 0 && millis() > heartbeat[i] + HEARTBEATTIMEOUT * 1000)
				{
					Serial.print("Too long heartbeat delay for the loco " );
					Serial.println(this->locos[i].getAddress());
					this->locos[i].setSpeed(0);
					heartbeat[i] = 0;
					String buff = "MTA";
					buff += this->locos[i].getAddress();
					buff += "<;>V0";
					inpThrottle->println(buff);
				}
				else
				{
					if (heartbeat[i] == 0)
					{
						heartbeat[i] = millis();
					}
				}
			}
		}
	}
}

void MessageConverterWiThrottle::locoAdd(Throttle* inpThrottle, int inLocoNumber, String th, String actionKey)
{
	this->locos[inLocoNumber].setAddress((uint16_t) actionKey.substring(1).toInt());
	inpThrottle->println("M" + th + "+" + actionKey + "<;>");
	for (int fKey = 0; fKey < 29; fKey++) 
	{
		this->locos[inLocoNumber].functions.inactivate(fKey);
		inpThrottle->println("M" + th + "A" + actionKey + "<;>F0" + fKey);
	}
	this->locos[inLocoNumber].setSpeed(0);
	this->locos[inLocoNumber].setDirection(true);
	inpThrottle->println("M" + th + "+" + actionKey + "<;>V0");
	inpThrottle->println("M" + th + "+" + actionKey + "<;>R1");
	inpThrottle->println("M" + th + "+" + actionKey + "<;>s1");

#ifdef DCCPP_DEBUG_MODE
	inpThrottle->printThrottle();
#endif
	heartbeat[inLocoNumber] = millis();
}

void MessageConverterWiThrottle::locoRelease(Throttle* inpThrottle, int inLocoNumber, String th, String actionKey)
{
	int address = this->locos[inLocoNumber].getAddress();	
	heartbeat[inLocoNumber] = 0;
	this->locos[inLocoNumber].initialize();
	inpThrottle->println("M" + th + "-" + actionKey + "<;>");

#ifdef DCCPP_DEBUG_MODE
	inpThrottle->printThrottle();
#endif
}

#ifdef DCCPP_DEBUG_MODE
void MessageConverterWiThrottle::printConverter()
{
	Serial.print("WiThrottle converter : locos ");
	for (int i = 0; i < WIMAXTHROTLLESNUMBER; i++)
	{
			Serial.print(this->locos[i].getAddress());
			Serial.print("/");
	}
}

void MessageConverterWiThrottle::accessoryToggle(int aAddr, String aStatus) 
{
	/*
	int newStat;
	if (aStatus == "T")
		newStat = 1;
	else if (aStatus == "C")
		newStat = 0;
	else
		newStat = -1;
	int t = 0;
	for (t = 0; tt[t].address != 0 && tt[t].address != aAddr; t++);
	if (tt[t].address == 0 && newStat > -1) {
		int addr = ((aAddr - 1) / 4) + 1;
		int sub = aAddr - addr * 4 + 3;
		for (int i = 0; i < maxClient; i++) {
			client[i].println("PTA2LT" + String(aAddr));
		}
		Serial.print("<a " + String(addr) + " " + String(sub) + " " + String(newStat) + ">");
	}
	else {
		if (newStat == -1) {
			switch (tt[t].tStatus) {
			case 2:
				tt[t].tStatus = 4;
				newStat = 0;
				break;
			case 4:
				tt[t].tStatus = 2;
				newStat = 1;
				break;
			}
		}
		else {
			switch (newStat) {
			case 0:
				tt[t].tStatus = 2;
				break;
			case 1:
				tt[t].tStatus = 4;
				break;
			}
		}
		for (int i = 0; i < maxClient; i++) {
			client[i].println("PTA" + String(tt[t].tStatus) + "LT" + String(tt[t].address));
		}
		Serial.print("<T " + String(tt[t].id) + " " + String(newStat) + ">");
		String response = loadResponse();
	}
	*/
}

#endif
#endif