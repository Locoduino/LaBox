// ArduinoIno.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"

#include "Arduino.h"

//#include "C:\Users\Thierry\AppData\Local\Arduino15\packages\esp32\hardware\esp32\1.0.4\libraries\SPIFFS\examples\SPIFFS_Test\SPIFFS_Test.ino"
//#include "D:\Mes documents\Trusty\Arduino\Libraries\ArduinoJson\examples\JsonConfigFile\JsonConfigFile.ino"

#include "../examples/Autotest/Autotest.ino"
//#include "../examples/WifiDcc/WifiDcc.ino"
//#include "../examples/SerialDcc/SerialDcc.ino"
//#include "../examples/LaBox/LaBox.ino"
//#include "WiThrottle-Server-ESP32-057.ino"

//const char* ssid = "VIDEOFUTUR_C56165_2.4G";
//const char* password = "EenPghQD";

int main()
{
	emulatorSetup();
	setup();

	while (1)
	{
		emulatorLoop();
		loop();
	}
}
