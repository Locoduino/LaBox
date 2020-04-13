/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <Throttle Wifi class>
*************************************************************/

#include "DCCpp.h"

#if defined(USE_THROTTLES) && defined(USE_WIFI)
ThrottleWifi::ThrottleWifi(const char* inName, uint8_t* inMac, uint8_t* inIp, int inPort, EthernetProtocol inProtocol) : Throttle(inName)
{
	for (int i = 0; i < 4; i++)
	{
		if (inIp != NULL)
			this->wifiIp[i] = inIp[i];
		else
			this->wifiIp[i] = 0;
	}

	for (int i = 0; i < 6; i++)
	{
		if (inMac != NULL)
			this->wifiIpMac[i] = inMac[i];
		else
			this->wifiIpMac[i] = 0;
	}

	this->port = inPort;
	this->protocol = inProtocol;

	this->pServer = new WiFiServer(inPort);
}

void ThrottleWifi::connectWifi(const char* inSsid, const char* inPassword)
{
	WiFi.begin(inSsid, inPassword);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
#ifdef DCCPP_DEBUG_MODE
		Serial.print(".");
#endif
	}
#ifdef DCCPP_DEBUG_MODE
	Serial.println("");

	Serial.print(inSsid);
	Serial.println(F(" connected ! beginWifi achieved."));
#endif
}

bool ThrottleWifi::begin()
{
	this->pServer->begin();
	return true;
}

bool ThrottleWifi::receiveMessages()
{
	char commandString[MAX_COMMAND_LENGTH + 1];
	char c;
	bool added = false;

	WiFiClient client = this->pServer->available();

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
			else if (strlen(commandString) < MAX_COMMAND_LENGTH)    // if comamndString still has space, append character just read from network
				sprintf(commandString, "%s%c", commandString, c);     // otherwise, character is ignored (but continue to look for '<' or '>')
		} // while

		if (this->protocol == EthernetProtocol::HTTP)
			client.stop();
	}

	return true;
}

bool ThrottleWifi::sendMessage(const char *pMessage)
{
	this->pServer->println(pMessage);
	return true;
}

void ThrottleWifi::end()
{
}

bool ThrottleWifi::isConnected()
{
	return WiFi.status() == WL_CONNECTED;
}

bool ThrottleWifi::sendNewline()
{
	return false;
}

#ifdef DCCPP_DEBUG_MODE
void ThrottleWifi::printThrottle()
{
	Serial.print("ThrottleWifi : ");
	Serial.print(this->name);
	Serial.print(" id:");
	Serial.print(this->id);
	if (this->isConnected())
	{
		Serial.print(" ip:");
		Serial.print(WiFi.localIP());
	}
	else
	{
		Serial.print(" not connected");
	}

	Serial.println("");
}
#endif
#endif