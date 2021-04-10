/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <Throttle Wifi class>
*************************************************************/

#include "DCCpp.h"

#if defined(USE_TEXTCOMMAND) && defined(USE_THROTTLES) && defined(USE_WIFI)

IPAddress ThrottleWifi::wifiIp;
WiFiUDP ThrottleWifi::_ClientUDP;

ThrottleWifi::ThrottleWifi(const String& inName, int inPort) : Throttle(inName)
{
	this->port = inPort;
	this->remoteIp = INADDR_NONE;
	this->contacted = false;
}

void ThrottleWifi::connectWifi(const char* inSsid, const char* inPassword, IPAddress inIp, IPAddress inGateway, IPAddress insubnet, IPAddress inDns)
{
	bool connected = false;

#ifdef USE_HMI
	if (hmi::CurrentInterface != NULL)
	{
		hmi::CurrentInterface->WifiStartConnection(inSsid);
		hmi::CurrentInterface->HmiInterfaceUpdateDrawing();
	}
#endif

	//DO NOT TOUCH
	//  This is here to force the ESP32 to reset the WiFi and initialise correctly.
	WiFi.mode(WIFI_STA);
	delay(1000);
	WiFi.disconnect(true);
	delay(1000);
	WiFi.softAPdisconnect(true);           // disconnects AP Mode 
	delay(1000);
	// End silly stuff !!!

#ifdef USE_WIFI_LOCALSSID
	connected = WiFi.softAP(inSsid, inPassword);
#else
#ifdef VISUALSTUDIO
	connected = false;
#else
	if (!(inIp == INADDR_NONE))
	{
		// Force ip if necessary
		WiFi.config(inIp, inGateway, insubnet, inDns);
	}
	WiFi.begin(inSsid, inPassword);

#ifdef DCCPP_DEBUG_MODE
	int count = 0;
#endif
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
#ifdef DCCPP_DEBUG_MODE
		Serial.print(".");
		count++;
		if (count > 60)
		{
			Serial.println("");
			count = 0;
		}
#endif
	}
	if (WiFi.status() == WL_CONNECTED)
	{
		connected = true;
	}
#endif
#endif

#ifdef USE_WIFI_LOCALSSID
	ThrottleWifi::wifiIp = WiFi.softAPIP();
#else
	ThrottleWifi::wifiIp = WiFi.localIP();
#endif

#ifdef DCCPP_DEBUG_MODE

	if (connected)
	{
		Serial.print("Server IP address: ");
		Serial.print(wifiIp);
		Serial.print(" (");
		Serial.print(inSsid);
		Serial.println(F(") connected ! connectWifi achieved."));
#ifdef USE_HMI
		if (hmi::CurrentInterface != NULL)
		{
			hmi::CurrentInterface->WifiConnected(wifiIp);
			hmi::CurrentInterface->HmiInterfaceUpdateDrawing();
		}
#endif
	}
	else
	{
		Serial.print(" : ");
		Serial.print(WiFi.status());
		Serial.print(" on ");
		Serial.print(inSsid);
		Serial.println(F(" NOT CONNECTED. Restart Arduino !"));
#ifdef USE_HMI
		if (hmi::CurrentInterface != NULL)
		{
			hmi::CurrentInterface->WifiEndConnection(inSsid);
			hmi::CurrentInterface->HmiInterfaceUpdateDrawing();
		}
#endif
#ifndef VISUALSTUDIO
		while (true);
#endif
	}
#endif
}

bool ThrottleWifi::begin(EthernetProtocol inProtocol)
{
	this->type = TYPEWIFI;
	this->protocol = inProtocol;

	if (protocol == UDP)
	{
		this->pBuffer = new CircularBuffer(UDP_BUFFERSIZE);
		this->pServer = NULL;

		_ClientUDP.begin(this->wifiIp, this->port);

#if defined(ARDUINO_ARCH_ESP32)
		this->pBuffer->begin(true);
#else
		this->pBuffer->begin(false);
#endif

		_ClientUDP.flush();
	}
	else
	{
		this->pBuffer = NULL;
		this->pServer = new WiFiServer(this->port);
		this->pServer->begin();
	}

	return true;
}

bool ThrottleWifi::begin(MessageConverter *apConverter)
{
	this->type = TYPEWIFI;
	this->setConverter(apConverter);
	return this->begin(apConverter->getProtocol());
}

IPAddress ThrottleWifi::remoteIP()
{
	return this->remoteIp;
}

ThrottleWifi* ThrottleWifi::GetThrottle(IPAddress inRemoteIp, int inPort, EthernetProtocol inProtocol)
{
	// Try  to find for each client is this message
	Throttle* pThrottle = Throttles::get(inRemoteIp);

	if (pThrottle != NULL)
	{
		if (pThrottle->type == TYPEWIFI)
		{
			// If already mounted, use it !
			return (ThrottleWifi*)pThrottle;
		}
	}

	// If no client found, try to affect a new one in the list of free wifi clients.
	Throttle* pCurr = Throttles::getFirst();

	while (pCurr != NULL)
	{
		if (pCurr->type == TYPEWIFI)
		{
			if ((uint32_t) (pCurr->remoteIP()) == (uint32_t) (IPAddress) INADDR_NONE)
			{
				ThrottleWifi* pWifi = (ThrottleWifi*)pCurr;
				if (inPort == pWifi->port && pWifi->protocol == inProtocol && pWifi->contacted == false)
				{
					return pWifi;
				}
			}
		}
		pCurr = pCurr->pNextThrottle;
	}

	return NULL;
}

bool ThrottleWifi::readUdpPacket(ThrottleWifi *inpThrottle)
{
	bool added = false;
	byte udp[THROTTLE_UDPBYTE_SIZE];
	memset(udp, 0, THROTTLE_UDPBYTE_SIZE);
	int len = _ClientUDP.read(udp, THROTTLE_UDPBYTE_SIZE);
	added = len > 0;
	if (added)
	{
		inpThrottle->pBuffer->PushBytes(udp, len);
	}

	return added;
}

bool ThrottleWifi::loop()
{
	bool added = false;

	if (this->type == NOTSTARTEDTHROTTLE)
		return false;

	if (this->protocol == UDP)
	{
		int bufferSize = _ClientUDP.parsePacket();
		if (bufferSize > 0)
		{
			ThrottleWifi* pWifi = GetThrottle(_ClientUDP.remoteIP(), _ClientUDP.remotePort(), UDP);

			if (pWifi != NULL)
			{
				if (pWifi->contacted == false)
				{
					pWifi->contacted = true;
					pWifi->setRemoteIP(_ClientUDP.remoteIP());
					if (pWifi->pConverter != NULL)
						pWifi->pConverter->clientStart(pWifi);
#ifdef USE_HMI
					if (hmi::CurrentInterface != NULL)
						hmi::CurrentInterface->NewClient(pWifi->id, pWifi->remoteIP(), pWifi->port);
#endif
					if (DCCpp::powerOnAtFirstClient)
					{
						TextCommand::pCurrentThrottle = pWifi;
						DCCpp::powerOn(true, true);
						TextCommand::pCurrentThrottle = NULL;
					}
#ifdef DCCPP_DEBUG_MODE
					Throttles::printThrottles();
#endif
				}
				added = readUdpPacket(pWifi);
			}
		}
	}
	else
	{
		WiFiClient foundClient = this->pServer->available();

		ThrottleWifi* pWifi = this;
		if (foundClient)
		{
			pWifi = GetThrottle(foundClient.remoteIP(), this->port, this->protocol);

			if (pWifi != NULL)
			{
				if (pWifi->contacted == false)
				{
					pWifi->contacted = true;
					pWifi->setRemoteIP(foundClient.remoteIP());
					pWifi->client = foundClient;
					if (pWifi->pConverter != NULL)
						pWifi->pConverter->clientStart(pWifi);
#ifdef USE_HMI
					if (hmi::CurrentInterface != NULL)
						hmi::CurrentInterface->NewClient(pWifi->id, pWifi->remoteIP(), pWifi->port);
#endif
					if (DCCpp::powerOnAtFirstClient)
					{
						TextCommand::pCurrentThrottle = pWifi;
						DCCpp::powerOn(true, true);
						TextCommand::pCurrentThrottle = NULL;
					}
#ifdef DCCPP_DEBUG_MODE
					Throttles::printThrottles();
#endif
				}
			}
			else
				pWifi = this;
		}

		if (pWifi->protocol == HTTP) 
		{
			if (pWifi != NULL)
			{
				pWifi->pServer->println("HTTP/1.1 200 OK");
				pWifi->pServer->println("Content-Type: text/html");
				pWifi->pServer->println("Access-Control-Allow-Origin: *");
				pWifi->pServer->println("Connection: close");
				pWifi->pServer->println("");

				while (foundClient.available())
				{        // while there is data on the network
					added = pWifi->getCharacter(foundClient.read());
				}

				foundClient.stop();
			}
		}
		else
		{
			if (pWifi->contacted && pWifi->client)
			{
				while (pWifi->client.available())
				{        // while there is data on the network
					added = pWifi->getCharacter(pWifi->client.read());
				}
			}
		}
	}

	return added;
}

bool ThrottleWifi::sendMessage(const String& inMessage)
{
	if (this->type == NOTSTARTEDTHROTTLE)
		return false;

	if (!this->replyToCommands)
	{
		size_t size = 0;
		switch (this->protocol)
		{
		case TCP:		size = this->client.println(inMessage);	break;
		case HTTP:	size = this->pServer->println(inMessage);	break;

		case UDP:
		case NOTSIGNIFICANT:		
			return true;
		}

#ifdef DCCPP_DEBUG_MODE
		Serial.print(this->id);
		Serial.print(" -> ");
		Serial.print(inMessage);

		if (size == 0)
			Serial.println(" *** MESSAGE NOT SENT !");
		else
			Serial.println("");
#endif

	}
	return true;
}

void ThrottleWifi::write(byte* inpData, int inLengthData)
{
	size_t size = 0;

//	if (this->dontReply)
//		return;
	
	_ClientUDP.beginPacket(this->remoteIp, this->port);
	size = _ClientUDP.write(inpData, inLengthData);
	_ClientUDP.endPacket();

#ifdef DCCPP_DEBUG_MODE
	if (size == 0)
	{
		Serial.print(this->remoteIp);
		Serial.print(" -> ");
		char val[10];
		for (int i = 0; i < inLengthData; i++)
		{
			sprintf(val, "0x%02X ", inpData[i]);
			Serial.print(val);
		}

		Serial.println(" *** BINARY NOT SENT !");
	}
#endif

#ifdef DCCPP_DEBUG_VERBOSE_MODE
	char val[10];
	Serial.print(this->remoteIp);
	Serial.print(" -> ");
	for (int i = 0; i < inLengthData; i++)
	{
		sprintf(val, "0x%02X ", inpData[i]);
		Serial.print(val);
	}
	Serial.println("");
#endif
}

void ThrottleWifi::end()
{
	Throttle::end();

	this->remoteIp = INADDR_NONE;
#ifdef USE_HMI
	if (hmi::CurrentInterface != NULL)
		hmi::CurrentInterface->CloseClient(this->id);
#endif
}

bool ThrottleWifi::isConnected()
{
	if (this->type == NOTSTARTEDTHROTTLE)
		return false;

	return WiFi.status() == WL_CONNECTED || this->contacted;
}

bool ThrottleWifi::SendNewline() const
{
	return false;
}

#ifdef DCCPP_DEBUG_MODE
void ThrottleWifi::printThrottle()
{
	Serial.print(this->id);
	Serial.print(" : ThrottleWifi: ");
	Serial.print(this->name);
	Serial.print("  WifiPort: ");
	Serial.print(this->port);
	Serial.print("  WifiProtocol: ");
	switch (this->protocol)
	{
	case NOTSIGNIFICANT: Serial.print("NOTSIGNIFICANT"); break;
	case HTTP: Serial.print("HTTP"); break;
	case TCP: Serial.print("TCP"); break;
	case UDP: Serial.print("UDP"); break;
	}
	if (this->timeOutDelay != 0)
	{
		Serial.print("  timeout: ");
		Serial.print((int)this->timeOutDelay);
	}
	if (this->pConverter != NULL)
	{
		Serial.print(" (");
		this->pConverter->printConverter();
		Serial.print(") ");
	}

	if (this->protocol != UDP)
	{
		Serial.print(" start:");
		Serial.print(this->startCommandCharacter, DEC);
		Serial.print(" end:");
		Serial.print(this->endCommandCharacter, DEC);
	}

	if (this->isConnected())
	{
		Serial.print(" ip:");
		Serial.print(this->remoteIP());
	}
	else
	{
		Serial.print(" not connected");
	}

	Serial.println("");
}
#endif
#endif