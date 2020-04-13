/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <Throttle Serial class>
*************************************************************/

#include "DCCpp.h"

#if defined(USE_THROTTLES) && defined(USE_WIFI)
WiFiMulti* ThrottleWifiAPClient::pWifiMulti = NULL;
uint8_t ThrottleWifiAPClient::wifiServerIp[4] = { 0,0,0,0 };
WiFiServer* ThrottleWifiAPClient::pServer = NULL;
ThrottleWifiAPClient* ThrottleWifiAPClient::pFirstClient = NULL;
int ThrottleWifiAPClient::port = 0;
int ThrottleWifiAPClient::numberOfClients = 0;

ThrottleWifiAPClient::ThrottleWifiAPClient(const char* inName) : Throttle(inName)
{
}

void ThrottleWifiAPClient::connectWifi(const char* inSsid, const char* inPassword, int inPort, int inClientsNumber)
{
	port = inPort;
	numberOfClients = inClientsNumber;

	pWifiMulti = new WiFiMulti();
	pWifiMulti->addAP(inSsid, inPassword);

#ifdef DCCPP_DEBUG_MODE
	Serial.println("Connecting Wifi ");
#endif
	for (int loops = 10; loops > 0; loops--) {
		if (pWifiMulti->run() == WL_CONNECTED) {
#ifdef DCCPP_DEBUG_MODE
			Serial.println("");
			Serial.print("WiFi connected ");
			Serial.print("IP address: ");
			Serial.println(WiFi.localIP());
#endif
			break;
		}
		else {
#ifdef DCCPP_DEBUG_MODE
			Serial.println(loops);
#endif
			delay(1000);
		}
	}
	if (pWifiMulti->run() != WL_CONNECTED) {
#ifdef DCCPP_DEBUG_MODE
		Serial.println("WiFi connect failed");
#endif
		delay(1000);
		//ESP.restart();
	}

#ifdef DCCPP_DEBUG_MODE
	Serial.println("");

	Serial.print(inSsid);
	Serial.println(F(" connected ! connectWifi achieved."));
#endif

	pServer = new WiFiServer(port);
	pServer->begin();
	pServer->setNoDelay(true);

	for (int i = 0; i < inClientsNumber; i++)
	{
		char buffer[10];
		sprintf(buffer, "Client %d", i);
		ThrottleWifiAPClient *throttle = new ThrottleWifiAPClient(buffer);
		if (pFirstClient == NULL)
		{
			pFirstClient = throttle;
		}
	}
}

bool ThrottleWifiAPClient::begin()
{
	return true;
}


// Check also for new clients !
bool ThrottleWifiAPClient::receiveMessages()
{
	char commandString[MAX_COMMAND_LENGTH + 1];
	char c;
	bool added = false;

	if (pWifiMulti->run() == WL_CONNECTED) 
	{
		//check if there are any new clients
		if (pServer->hasClient()) {
			ThrottleWifiAPClient* pCurr = pFirstClient;

			for (int i = 0; i < numberOfClients; i++)
			{
				//find free/disconnected spot
				if (!pCurr->Client.connected()) 
				{
					pCurr->Client.stop();
					pCurr->Client = pServer->available();
#ifdef DCCPP_DEBUG_MODE
					if (!pCurr->Client)
						Serial.println("available broken");
					Serial.print("New client: ");
					Serial.print(i); 
					Serial.print(' ');
					Serial.println(pCurr->Client.remoteIP());

					Throttles::printThrottles();
#endif
					break;
				}
				pCurr = (ThrottleWifiAPClient*) pCurr->pNextThrottle;
			}

			if (pCurr == NULL) 
			{
				//no free/disconnected spot so reject
				pServer->available().stop();
#ifdef DCCPP_DEBUG_MODE
				Throttles::printThrottles();
#endif
			}
		}

		if (this->Client.connected())
		{
			while (this->Client.available())
			{        // while there is data on the network
				c = this->Client.read();
				if (c == '<')                    // start of new command
					commandString[0] = 0;
				else
				{
					if (c == '>')               // end of new command
					{
						this->pushMessageInStack(this->id, commandString);
						added = true;
					}
					else
						if (strlen(commandString) < MAX_COMMAND_LENGTH)    // if comamndString still has space, append character just read from network
							sprintf(commandString, "%s%c", commandString, c);     // otherwise, character is ignored (but continue to look for '<' or '>')
				}
			} // while
		}
		else
		{
			this->Client.stop();
		}
	}

	return true;
}

bool ThrottleWifiAPClient::sendMessage(const char *pMessage)
{
	this->Client.println(pMessage);
	return true;
}

void ThrottleWifiAPClient::end()
{
}

bool ThrottleWifiAPClient::isConnected()
{
	return true;
}

bool ThrottleWifiAPClient::sendNewline()
{
	return false;
}

#ifdef DCCPP_DEBUG_MODE
void ThrottleWifiAPClient::printThrottle()
{
	Serial.print("ThrottleWifiAPClient : ");
	Serial.print(this->name);
	Serial.print(" id:");
	Serial.print(this->id);
	if (this->Client.connected())
	{
		Serial.print(" ip:");
		Serial.print(this->Client.localIP());
	}
	else
	{
		Serial.print(" not connected");
	}

	Serial.println("");
}
#endif
#endif