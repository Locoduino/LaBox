// ArduinoIno.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "stdafx.h"

#include "Arduino.h"

#include "../examples/WifiDcc/WifiDcc.ino"

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
