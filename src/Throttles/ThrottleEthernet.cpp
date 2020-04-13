/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <Throttle Ethernet class>
*************************************************************/

#include "DCCpp.h"

#if !defined(ARDUINO_ARCH_ESP32)
#if defined(USE_THROTTLES) && defined(USE_ETHERNET)
ThrottleEthernet::ThrottleEthernet(const char* inName, uint8_t* inMac, uint8_t* inIp, int inPort, enum EthernetProtocol inProtocol) : Throttle(inName)
{
	for (int i = 0; i < 4; i++)
	{
		if (inIp != NULL)
			this->ethernetIp[i] = inIp[i];
		else
			this->ethernetIp[i] = 0;
	}

	for (int i = 0; i < 6; i++)
	{
		if (inMac != NULL)
			this->ethernetMac[i] = inMac[i];
		else
			this->ethernetMac[i] = 0;
	}

	this->port = inPort;
	this->protocol = inProtocol;

	this->pServer = new EthernetServer(inPort);
}

bool ThrottleEthernet::begin()
{

	if (this->ethernetIp[0] == 0)
		Ethernet.begin(this->ethernetMac);                  // Start networking using DHCP to get an IP Address
	else
		Ethernet.begin(this->ethernetMac, this->ethernetIp);           // Start networking using STATIC IP Address

	this->pServer->begin();
	return true;
}

bool ThrottleEthernet::receiveMessages()
{
	char commandString[MAX_COMMAND_LENGTH + 1];
	char c;
	bool added = false;

	EthernetClient client = this->pServer->available();

	if (client) {
		if (this->protocol == EthernetProtocol::HTTP) {
			this->pServer->println("HTTP/1.1 200 OK");
			this->pServer->println("Content-Type: text/html");
			this->pServer->println("Access-Control-Allow-Origin: *");
			this->pServer->println("Connection: close");
			this->pServer->println("");
		}

		while (client.connected() && client.available()) {        // while there is data on the network

			c = client.read();
			if (c == '<')                    // start of new command
				commandString[0] = 0;
			else if (c == '>')               // end of new command
			{
				this->pushMessageInStack(this->id, commandString);
				added = true;
			}
			else if (strlen(commandString) < MAX_COMMAND_LENGTH)    // if comandString still has space, append character just read from network
				sprintf(commandString, "%s%c", commandString, c);     // otherwise, character is ignored (but continue to look for '<' or '>')
		} // while

		if (this->protocol == EthernetProtocol::HTTP)
			client.stop();
	}

	return added;
}

bool ThrottleEthernet::sendMessage(const char *pMessage)
{
	this->pServer->println(pMessage);
	return true;
}

void ThrottleEthernet::end()
{
}

bool ThrottleEthernet::isConnected()
{
	return true;
}

bool ThrottleEthernet::sendNewline()
{
	return false;
}

#ifdef DCCPP_DEBUG_MODE
void ThrottleEthernet::printThrottle()
{
	Serial.print(this->name);
	Serial.print(" id:");
	Serial.print(this->id);

	Serial.println("");
}
#endif
#endif
#endif
