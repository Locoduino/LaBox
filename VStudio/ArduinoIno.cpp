// ArduinoIno.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"

#include "Arduino.h"

//#include "../examples/WifiDcc/WifiDcc.ino"
//#include "../examples/SerialDcc/SerialDcc.ino"
#include "../examples/LaBox/LaBox.ino"

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
