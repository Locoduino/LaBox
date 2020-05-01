/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <Throttle Wifi AP client class>
*************************************************************/

#include "DCCpp.h"

#if defined(USE_TEXTCOMMAND) && defined(USE_THROTTLES) && defined(USE_WIFI)

WiFiMulti* ThrottleWifiAPClient::pWifiMulti = NULL;
uint8_t ThrottleWifiAPClient::wifiServerIp[4] = { 0,0,0,0 };
WiFiServer* ThrottleWifiAPClient::pServer = NULL;
EthernetProtocol ThrottleWifiAPClient::protocol = TCP;
ThrottleWifiAPClient* ThrottleWifiAPClient::pFirstClient = NULL;
int ThrottleWifiAPClient::port = 0;
int ThrottleWifiAPClient::numberOfClients = 0;

ThrottleWifiAPClient::ThrottleWifiAPClient(const String& inName) : Throttle(inName)
{
	this->wifiIp[0] = this->wifiIp[1] = this->wifiIp[2] = this->wifiIp[3] = 0;
	this->connected = false;
}

//wifi event handler
void WiFiEvent(WiFiEvent_t event)
{
	switch (event) {
/*	case SYSTEM_EVENT_STA_GOT_IP:
		//When connected set 
		Serial.print("WiFi connected! IP address: ");
		Serial.println(WiFi.localIP());
		//initializes the UDP state
		//This initializes the transfer buffer
		break;*/

	case SYSTEM_EVENT_STA_DISCONNECTED:
		Serial.println("WiFi lost connection");
		break;
	default: break;
	}

	//return 0;
}

void ThrottleWifiAPClient::connectWifi(const char* inSsid, const char* inPassword, int inPort, int inClientsNumber, EthernetProtocol inProtocol)
{
	bool connected = false;
	port = inPort;
	numberOfClients = inClientsNumber;
	protocol = inProtocol;

	//DO NOT TOUCH
	//  This is here to force the ESP32 to reset the WiFi and initialise correctly.
	WiFi.mode(WIFI_STA);
	delay(1000);
	WiFi.disconnect(true);
	delay(1000);
	WiFi.softAPdisconnect(true);           // disconnects AP Mode 
	delay(1000); 
	// End silly stuff !!!

	WiFi.onEvent(WiFiEvent);

#ifdef USE_WIFI_LOCALSSID
	connected = WiFi.softAP(inSsid, inPassword);
	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.print(myIP);
	Serial.print(":");
	Serial.println(inPort);
#else
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
			Serial.print(WiFi.localIP());
			Serial.print(":");
			Serial.println(inPort);
#endif
			connected = true;
			break;
		}
		else {
#ifdef DCCPP_DEBUG_MODE
			Serial.println(loops);
#endif
			delay(1000);
		}
	}
#endif

#ifdef DCCPP_DEBUG_MODE
	Serial.println("");
	Serial.print(inSsid);

	if (connected)
	{
		Serial.println(F(" connected ! connectWifi achieved."));
	}
	else
	{
		Serial.print(" : ");
		Serial.print(WiFi.status());
		Serial.print(" on ");
		Serial.print(inSsid);
		Serial.println(F(" NOT CONNECTED. Stop execution !"));
		while (true);
	}
#endif

	pServer = new WiFiServer(port);
	pServer->begin();
	pServer->setNoDelay(true);

	for (int i = 0; i < inClientsNumber; i++)
	{
		char buffer[10];
		sprintf(buffer, "Client %d", i);
		ThrottleWifiAPClient*throttle = new ThrottleWifiAPClient(buffer);
		if (pFirstClient == NULL)
		{
			pFirstClient = throttle;
		}
	}
}

bool ThrottleWifiAPClient::begin()
{
	if (protocol == UDP)
	{
		this->ClientUDP.begin(WiFi.localIP(), port + this->id);
	}
	return true;
}

// Check also for new clients !
bool ThrottleWifiAPClient::loop()
{
	ThrottleWifiAPClient* pCurrent = this;
	bool added = false;

#ifdef USE_WIFI_LOCALSSID
	//if (WiFi.status() == WL_CONNECTED)
#else
	if (pWifiMulti->run() == WL_CONNECTED)
#endif
	{
		if (protocol == TCP)
		{
			WiFiClient avail = pServer->available();
			if (avail)
			{
				Serial.print(this->id);
				Serial.print(" / ");
				Serial.print(avail.remoteIP());
				Serial.println(" Available ");

				// if the client is not the one of 'this' throttle, try to connect to it.
				if (!this->Client.connected() || this->Client.remoteIP() != avail.remoteIP())
				{
					// check if not already used by another throttle...
					ThrottleWifiAPClient* pLoop = ThrottleWifiAPClient::pFirstClient;
					for (int i = 0; i < ThrottleWifiAPClient::numberOfClients; i++)
					{
						if (pLoop == NULL)
							break;

						if (pLoop == this)
							continue;

						if (pLoop->Client.connected() && pLoop->Client.remoteIP() == avail.remoteIP())
						{
							Serial.print("Already found : ");
							Serial.print(pLoop->id);
							Serial.print(" Throttle: ");
							Serial.print(pLoop->Client.remoteIP());
							Serial.print(" new: ");
							Serial.println(avail.remoteIP());
							return false;
						}
						pLoop = (ThrottleWifiAPClient*)pLoop->pNextThrottle;
					}

					if (!this->Client.connected())
					{
						this->Client = avail;
						Serial.print(this->id);
						Serial.print(" : ");
						Serial.println(this->Client.remoteIP());

						connected = true;
						//loadTurnouts();
						if (this->pConverter != NULL)
						{
							this->Client.flush();
							this->Client.setTimeout(500);
							this->pConverter->clientStart(this);
						}
					}
					else
					{
						if (!this->Client.connected())
						{
							Serial.println("not connected");
							connected = false;
							if (this->pConverter != NULL)
							{
								this->pConverter->clientStop(this);
							}
						}
					}
				}
			}

			if (this->Client.connected())
			{
				while (this->Client.available())
				{  // while there is data on the network
					added = Throttle::getCharacter(this->Client.read(), this);
				}

				if (this->pConverter != NULL)
				{
					this->pConverter->stayAlive(this);
				}
			}
		}

		if (protocol == UDP)
		{
/*			WiFiUDP avail = WiFiUDP::available();
			if (avail)
			{
				Serial.print(this->id);
				Serial.print(" : ");
				Serial.println("Available ");

				// if the client is not the one of 'this' throttle, try to connect to it.
				if (!this->Client.connected() || this->Client.remoteIP() != avail.remoteIP())
				{
					// check if not already used by another throttle...
					ThrottleWifiAPClient* pLoop = ThrottleWifiAPClient::pFirstClient;
					for (int i = 0; i < ThrottleWifiAPClient::numberOfClients; i++)
					{
						if (pLoop == NULL)
							break;

						if (pLoop == this)
							continue;

						if (pLoop->Client.connected() && pLoop->Client.remoteIP() == avail.remoteIP())
						{
							Serial.print("Already found : ");
							Serial.print(pLoop->id);
							Serial.print(" Throttle: ");
							Serial.print(pLoop->Client.remoteIP());
							Serial.print(" new: ");
							Serial.println(avail.remoteIP());
							return false;
						}
						pLoop = (ThrottleWifiAPClient*)pLoop->pNextThrottle;
					}

					if (!this->Client.connected())
					{
						this->Client = avail;
						Serial.print(this->id);
						Serial.print(" : ");
						Serial.println(this->Client.remoteIP());

						connected = true;
						//loadTurnouts();
						if (this->pConverter != NULL)
						{
							this->pConverter->clientStart(this);
						}
					}
					else
					{
						if (!this->Client.connected())
						{
							Serial.println("not connected");
							connected = false;
							if (this->pConverter != NULL)
							{
								this->pConverter->clientStop(this);
							}
						}
					}
				}
			}

			if (this->Client.connected())
			{
				while (this->Client.available())
				{        // while there is data on the network
					char c = this->Client.read();
					if (c == (char)this->startCommandCharacter)                    // start of new command
					{
						commandString[0] = 0;
					}
					else
					{
						if (c == (char)this->endCommandCharacter)               // end of new command
						{
	#ifdef DCCPP_DEBUG_MODE
							Serial.print(this->id);
							Serial.print(" From WiThrottle : ");
							Serial.println(commandString);
	#endif
							this->pushMessage(commandString);
							commandString[0] = 0;
							added = true;
						}
						else
						{
							if (c > ' ' && strlen(commandString) < MAX_COMMAND_LENGTH)    // if comamndString still has space, append character just read from network
								sprintf(commandString, "%s%c", commandString, c);     // otherwise, character is ignored (but continue to look for start or end characters)}
						}
					}
				} // while

				if (this->pConverter != NULL)
				{
					this->pConverter->stayAlive(this);
				}
			}*/
		}
	}

	return added;
}

bool ThrottleWifiAPClient::sendMessage(const String& inMessage)
{
#ifdef DCCPP_DEBUG_MODE
	Serial.print("Send : ");
	Serial.println(inMessage);
#endif
	this->Client.println(inMessage);
	return true;
}

void ThrottleWifiAPClient::end()
{
	this->Client.stop();
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
#ifdef DCCPP_PRINT_DCCPP
void ThrottleWifiAPClient::printThrottle()
{
	Serial.print(this->id);
	Serial.print(" : ");
	Serial.print("ThrottleWifiAPClient : ");
	Serial.print(this->name);
	if (this->Client.connected())
	{
		Serial.print(" ip:");
		Serial.print(this->Client.remoteIP());
	}
	else
	{
		Serial.print(" not connected");
	}

	Serial.print(" startChar:");
	Serial.print((int)this->startCommandCharacter);
	Serial.print(" endChar:");
	Serial.print((int)this->endCommandCharacter);

	if (this->pConverter != NULL)
	{
		Serial.print(" (");
		this->pConverter->printConverter();
		Serial.print(") ");
	}

	Serial.println("");
}
#endif
#endif
#endif