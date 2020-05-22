/*
 * La Box Project
 * Configuration of Wifi Server
 *
 * @Author : Cedric Bellec
 * @Organization : Locoduino.org
 */
 
#ifndef __WIFISERVERCONFIG__
#define __WIFISERVERCONFIG__

#include <arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>

/*---------------------- Overview --------------------------
The ESP can be configured at the wifi level in 2 modes:
 * Access Point (enumWifiMode  _wifiMode = wifiAp
 * Your router Client  (enumWifiMode  _wifiMode = wifiClient)

In AP mode, you must set the name of this network SSID and the password through 2 macros SSID and PASSWD
You must define the network parameters such as the ESP IP address, its subnet mask and its subnet. In AP Mode, @IP et @gateway are the same 
so don't worry about seeing in the code this: WiFi.softAPConfig(gateway, gateway, subnet);  (2x gateway)
DNS is not usefull.

In client mode, the ESP will connect to your organization's wifi network. Uou must define the name of the network SSID of your installation as well as the WPA / WPA2 key to connect through the 2 macros SSID and PASSWD
The connection will be done in static IP, ie you will have to choose the IP address in your organization's network. Be careful not to choose an address already taken!
You need to set 4 @IP : 
* IP address of the device
* IP address of you DNS server. If you don't know it, write the same address of you gateway
* IP of you gateway : it's probably the IP of you Internet router
* Network mask : If you don't know what it is, put 255.255.255.0 and generally it will work. If it's not work, call a friend who is familiar with the network. You can also watch on your computer the netmask (ipconfig command on a PC)

*/
//--------- Common network settings ----------

#define	SSID				"Locoduino La Box"
#define PASSWD				""
#define	WTSERVER_PORT		44444
#define	Z21SERVER_PORT		44445

#define DEVICENAME			"LocoduinoBox"			// Hostname in the network

IPAddress ip		  (192, 	168, 	1, 	 10);     // fix IP of the server il client mode
IPAddress gateway	(192, 	168, 	1, 	254);  		// WiFi router's IP. If you are in AP mode, it's the ESP IP. If you are in client, it's your own gateway
IPAddress subnet	(255, 	255,  255, 	0);     // Subnet mask
IPAddress dns		  (192, 	168, 	1, 	254);  		// DNS server, generally, the WiFi access point is a DNS server, but we can define another one.

#endif
