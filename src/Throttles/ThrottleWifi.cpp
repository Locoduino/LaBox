/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <Throttle Wifi class>
*************************************************************/

#include "DCCpp.h"

#if defined(USE_TEXTCOMMAND) && defined(USE_THROTTLES) && defined(USE_WIFI)
ThrottleWifi::ThrottleWifi(const String& inName, uint8_t* inMac, uint8_t* inIp, int inPort, EthernetProtocol inProtocol) : Throttle(inName)
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
	WiFi.softAP(inSsid, inPassword);
	IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(myIP);
#else
	WiFi.begin(inSsid, inPassword);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
#ifdef DCCPP_DEBUG_MODE
		Serial.print(".");
		Serial.print(WiFi.status());
#endif
	}
#endif

#ifdef DCCPP_DEBUG_MODE
	Serial.println("");

	Serial.print(inSsid);
	Serial.println(F(" connected ! beginWifi achieved."));
#endif
}

bool ThrottleWifi::begin()
{
	Serial.println("begin");
	this->pServer->begin();
	return true;
}

bool ThrottleWifi::loop()
{
	bool added = false;

	WiFiClient foundClient = this->pServer->available();

	if (!this->client && foundClient) {
		this->client = foundClient;
		Serial.println("Client found");
		if (this->protocol == EthernetProtocol::HTTP) {
			this->pServer->println("HTTP/1.1 200 OK");
			this->pServer->println("Content-Type: text/html");
			this->pServer->println("Access-Control-Allow-Origin: *");
			this->pServer->println("Connection: close");
			this->pServer->println("");
		}
	}

	if (this->client) 
	{
		while (this->client.available()) 
		{        // while there is data on the network
			added = Throttle::getCharacter(this->client.read(), this);
		}

		if (this->protocol == EthernetProtocol::HTTP)
			this->client.stop();
	}

	return added;
}

bool ThrottleWifi::sendMessage(const String& inMessage)
{
#ifdef DCCPP_DEBUG_MODE
	Serial.print("-> ");
	Serial.println(inMessage);
#endif
	this->pServer->println(inMessage);
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
	Serial.print(this->id);
	Serial.print(" : ");
	Serial.print("ThrottleWifi : ");
	Serial.print(this->name);
	if (this->pConverter != NULL)
	{
		Serial.print(" (");
		this->pConverter->printConverter();
		Serial.print(") ");
	}
	if (this->isConnected())
	{
		Serial.print(" ip:");
		Serial.print(this->client.remoteIP());
	}
	else
	{
		Serial.print(" not connected");
	}

	Serial.println("");
}
#endif
#endif