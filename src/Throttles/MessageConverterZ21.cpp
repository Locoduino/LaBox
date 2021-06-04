/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <Message converter base class>
*************************************************************/

#include "Arduino.h"
#include "DCCpp.h"

#if defined(USE_THROTTLES) && defined (USE_WIFI)

byte MessageConverterZ21::commBuffer[100];
byte MessageConverterZ21::replyBuffer[20];

MessageConverterZ21::MessageConverterZ21()
{
}

void MessageConverterZ21::clientStart(Throttle* inpThrottle)
{
	inpThrottle->setTimeOutDelay(Z21_TIMEOUT);
//	Throttles::printThrottles();
}

void MessageConverterZ21::clientStop(Throttle* inpThrottle)
{
	inpThrottle->end();
//	Throttles::printThrottles();
}

void MessageConverterZ21::stayAlive(Throttle* inpThrottle)
{
}

// sizes : [       2        ][       2        ][inLengthData]
// bytes : [length1, length2][Header1, Header2][Data........]
bool MessageConverterZ21::notify(unsigned int inHeader, byte* inpData, unsigned int inLengthData, bool inXorInData, Throttle* inpThrottle)
{
	int realLength = (inLengthData + 4 + (inXorInData == false ? 1 : 0));

	MessageConverterZ21::commBuffer[0] = realLength % 256;
	MessageConverterZ21::commBuffer[1] = realLength / 256;
	MessageConverterZ21::commBuffer[2] = inHeader % 256;
	MessageConverterZ21::commBuffer[3] = inHeader / 256;
	memcpy(MessageConverterZ21::commBuffer + 4, inpData, inLengthData);

	if (!inXorInData)    // if xor byte not included in data, compute and write it !
	{
		byte xxor = 0;
		for (unsigned int i = 0; i < inLengthData; i++)
		{
			xxor ^= inpData[i];
		}
		MessageConverterZ21::commBuffer[inLengthData+4] = xxor;
	}

	inpThrottle->write(MessageConverterZ21::commBuffer, realLength);

	return true;
}

// sizes : [       2        ][       2        ][   1   ][inLengthData]
// bytes : [length1, length2][Header1, Header2][XHeader][Data........]
bool MessageConverterZ21::notify(unsigned int inHeader, unsigned int inXHeader, byte* inpData, unsigned int inLengthData, bool inXorInData, Throttle* inpThrottle)
{
	int realLength = (inLengthData + 5 + (inXorInData == false ? 1 : 0));

	MessageConverterZ21::commBuffer[0] = realLength % 256;
	MessageConverterZ21::commBuffer[1] = realLength / 256;
	MessageConverterZ21::commBuffer[2] = inHeader % 256;
	MessageConverterZ21::commBuffer[3] = inHeader / 256;
	MessageConverterZ21::commBuffer[4] = inXHeader;
	memcpy(MessageConverterZ21::commBuffer + 5, inpData, inLengthData);

	if (!inXorInData)    // if xor byte not included in data, compute and write it !
	{
		byte xxor = inXHeader;
		for (unsigned int i = 0; i < inLengthData; i++)
		{
			xxor ^= inpData[i];
		}
		MessageConverterZ21::commBuffer[inLengthData + 5] = xxor;
	}

	inpThrottle->write(MessageConverterZ21::commBuffer, realLength);

	return true;
}

// sizes : [       2        ][       2        ][   1   ][ 1 ][inLengthData]
// bytes : [length1, length2][Header1, Header2][XHeader][DB0][Data........]
bool MessageConverterZ21::notify(unsigned int inHeader, unsigned int inXHeader, byte inDB0, byte* inpData, unsigned int inLengthData, bool inXorInData, Throttle* inpThrottle)
{
	int realLength = (inLengthData + 6 + (inXorInData == false ? 1 : 0));

	MessageConverterZ21::commBuffer[0] = realLength % 256;
	MessageConverterZ21::commBuffer[1] = realLength / 256;
	MessageConverterZ21::commBuffer[2] = inHeader % 256;
	MessageConverterZ21::commBuffer[3] = inHeader / 256;
	MessageConverterZ21::commBuffer[4] = inXHeader;
	MessageConverterZ21::commBuffer[5] = inDB0;
	memcpy(MessageConverterZ21::commBuffer + 6, inpData, inLengthData);

	if (!inXorInData)    // if xor byte not included in data, compute and write it !
	{
		byte xxor = inXHeader^inDB0;
		for (unsigned int i = 0; i < inLengthData; i++)
		{
			xxor ^= inpData[i];
		}
		MessageConverterZ21::commBuffer[inLengthData + 6] = xxor;
	}

	inpThrottle->write(MessageConverterZ21::commBuffer, realLength);

	return true;
}

void MessageConverterZ21::notifyStatus(Throttle* inpThrottle)
{
	MessageConverterZ21::replyBuffer[0] = 0;	// main current 1
	MessageConverterZ21::replyBuffer[1] = 0; // main current 2
	MessageConverterZ21::replyBuffer[2] = 0; // prog current 1
	MessageConverterZ21::replyBuffer[3] = 0; // prog current 2
	MessageConverterZ21::replyBuffer[4] = 0; // filtered main current 1
	MessageConverterZ21::replyBuffer[5] = 0; // filtered main current 2
	MessageConverterZ21::replyBuffer[6] = 0; // Temperature 1
	MessageConverterZ21::replyBuffer[7] = 0; // Temperature 2
	MessageConverterZ21::replyBuffer[8] = 5; // Supply voltage 1
	MessageConverterZ21::replyBuffer[9] = 0; // supply voltage 2
	MessageConverterZ21::replyBuffer[10] = 16; // VCC voltage 1 
	MessageConverterZ21::replyBuffer[11] = 0; // VCC voltage 2
	MessageConverterZ21::replyBuffer[12] = 0b00000000;	// CentralState 
	MessageConverterZ21::replyBuffer[13] = 0b00000000; // CentralStateEx
	MessageConverterZ21::replyBuffer[14] = 0;
	MessageConverterZ21::replyBuffer[15] = 0;
	notify(HEADER_LAN_SYSTEMSTATE, MessageConverterZ21::replyBuffer, 16, true, inpThrottle);
}

void MessageConverterZ21::notifyLocoInfo(Throttle* inpThrottle, byte inMSB, byte inLSB)
{
	int locoAddress = ((inMSB & 0x3F) << 8) + inLSB;
	Locomotive* pLoco = Locomotives::get(locoAddress);

	if (pLoco == NULL)
		pLoco = Locomotives::add("Loco", locoAddress, 128);		// default

	MessageConverterZ21::replyBuffer[0] = inMSB;	// loco address msb
	MessageConverterZ21::replyBuffer[1] = inLSB; // loco address lsb
	MessageConverterZ21::replyBuffer[2] = B00000100; // 0000CKKK	 C = already controlled    KKK = speed steps 000:14, 010:28, 100:128
	MessageConverterZ21::replyBuffer[3] = pLoco->getSpeed(); // RVVVVVVV  R = forward    VVVVVVV = speed
	if (pLoco->isDirectionForward()) bitSet(MessageConverterZ21::replyBuffer[3], 7);

	MessageConverterZ21::replyBuffer[4] = B00000000; // 0DSLFGHJ  D = double traction    S = Smartsearch   L = F0   F = F4   G = F3   H = F2   J = F1
	if (pLoco->functions.isActivated(0)) bitSet(MessageConverterZ21::replyBuffer[4], 4);
	if (pLoco->functions.isActivated(1)) bitSet(MessageConverterZ21::replyBuffer[4], 0);
	if (pLoco->functions.isActivated(2)) bitSet(MessageConverterZ21::replyBuffer[4], 1);
	if (pLoco->functions.isActivated(3)) bitSet(MessageConverterZ21::replyBuffer[4], 2);
	if (pLoco->functions.isActivated(4)) bitSet(MessageConverterZ21::replyBuffer[4], 3);

	MessageConverterZ21::replyBuffer[5] = B00000000;	// function F5 to F12    F5 is bit0
	if (pLoco->functions.isActivated(5)) bitSet(MessageConverterZ21::replyBuffer[5], 0);
	if (pLoco->functions.isActivated(6)) bitSet(MessageConverterZ21::replyBuffer[5], 1);
	if (pLoco->functions.isActivated(7)) bitSet(MessageConverterZ21::replyBuffer[5], 2);
	if (pLoco->functions.isActivated(8)) bitSet(MessageConverterZ21::replyBuffer[5], 3);
	if (pLoco->functions.isActivated(9)) bitSet(MessageConverterZ21::replyBuffer[5], 4);
	if (pLoco->functions.isActivated(10)) bitSet(MessageConverterZ21::replyBuffer[5],5);
	if (pLoco->functions.isActivated(11)) bitSet(MessageConverterZ21::replyBuffer[5],6);
	if (pLoco->functions.isActivated(12)) bitSet(MessageConverterZ21::replyBuffer[5],7);

	MessageConverterZ21::replyBuffer[6] = B00000000;	// function F13 to F20   F13 is bit0
	if (pLoco->functions.isActivated(13)) bitSet(MessageConverterZ21::replyBuffer[6], 0);
	if (pLoco->functions.isActivated(14)) bitSet(MessageConverterZ21::replyBuffer[6], 1);
	if (pLoco->functions.isActivated(15)) bitSet(MessageConverterZ21::replyBuffer[6], 2);
	if (pLoco->functions.isActivated(16)) bitSet(MessageConverterZ21::replyBuffer[6], 3);
	if (pLoco->functions.isActivated(17)) bitSet(MessageConverterZ21::replyBuffer[6], 4);
	if (pLoco->functions.isActivated(18)) bitSet(MessageConverterZ21::replyBuffer[6], 5);
	if (pLoco->functions.isActivated(19)) bitSet(MessageConverterZ21::replyBuffer[6], 6);
	if (pLoco->functions.isActivated(20)) bitSet(MessageConverterZ21::replyBuffer[6], 7);

	MessageConverterZ21::replyBuffer[7] = B00000000;	// function F21 to F28   F21 is bit0
	if (pLoco->functions.isActivated(21)) bitSet(MessageConverterZ21::replyBuffer[7], 0);
	if (pLoco->functions.isActivated(22)) bitSet(MessageConverterZ21::replyBuffer[7], 1);
	if (pLoco->functions.isActivated(23)) bitSet(MessageConverterZ21::replyBuffer[7], 2);
	if (pLoco->functions.isActivated(24)) bitSet(MessageConverterZ21::replyBuffer[7], 3);
	if (pLoco->functions.isActivated(25)) bitSet(MessageConverterZ21::replyBuffer[7], 4);
	if (pLoco->functions.isActivated(26)) bitSet(MessageConverterZ21::replyBuffer[7], 5);
	if (pLoco->functions.isActivated(27)) bitSet(MessageConverterZ21::replyBuffer[7], 6);
	if (pLoco->functions.isActivated(28)) bitSet(MessageConverterZ21::replyBuffer[7], 7);

	notify(HEADER_LAN_XPRESS_NET, LAN_X_HEADER_LOCO_INFO, MessageConverterZ21::replyBuffer, 8, false, inpThrottle);
}

void MessageConverterZ21::notifyTurnoutInfo(Throttle* inpThrottle, byte inMSB, byte inLSB)
{
	MessageConverterZ21::replyBuffer[0] = inMSB;	// turnout address msb
	MessageConverterZ21::replyBuffer[1] = inLSB; // turnout address lsb
	MessageConverterZ21::replyBuffer[2] = B00000000; // 000000ZZ	 ZZ : 00 not switched   01 pos1  10 pos2  11 invalid
	notify(HEADER_LAN_XPRESS_NET, LAN_X_HEADER_TURNOUT_INFO, MessageConverterZ21::replyBuffer, 3, false, inpThrottle);
}

void MessageConverterZ21::notifyLocoMode(Throttle* inpThrottle, byte inMSB, byte inLSB)
{
	MessageConverterZ21::replyBuffer[0] = inMSB;	// loco address msb
	MessageConverterZ21::replyBuffer[1] = inLSB; // loco address lsb
	MessageConverterZ21::replyBuffer[2] = B00000000; // 00000000	DCC   00000001 MM
	notify(HEADER_LAN_GET_LOCOMODE, MessageConverterZ21::replyBuffer, 3, true, inpThrottle);
}

void MessageConverterZ21::notifyFirmwareVersion(Throttle* inpThrottle)
{
	MessageConverterZ21::replyBuffer[0] = 0x01;	// Version major in BCD
	MessageConverterZ21::replyBuffer[1] = 0x23;	// Version minor in BCD
	notify(HEADER_LAN_XPRESS_NET, LAN_X_HEADER_FIRMWARE_VERSION, 0x0A, MessageConverterZ21::replyBuffer, 2, false, inpThrottle);
}

void MessageConverterZ21::notifyHWInfo(Throttle* inpThrottle)
{
	MessageConverterZ21::replyBuffer[0] = 0x00;	// Hardware type in BCD on int32
	MessageConverterZ21::replyBuffer[1] = 0x02;	// Hardware type in BCD on int32
	MessageConverterZ21::replyBuffer[2] = 0x00;	// Hardware type in BCD on int32
	MessageConverterZ21::replyBuffer[3] = 0x00;	// Hardware type in BCD on int32
	MessageConverterZ21::replyBuffer[4] = 0x23;	// Firmware version in BCD on int32
	MessageConverterZ21::replyBuffer[5] = 0x01;	// Firmware version in BCD on int32
	MessageConverterZ21::replyBuffer[6] = 0x00;	// Firmware version in BCD on int32
	MessageConverterZ21::replyBuffer[7] = 0x00;	// Firmware version in BCD on int32
	notify(HEADER_LAN_GET_HWINFO, MessageConverterZ21::replyBuffer, 8, true, inpThrottle);
}

void MessageConverterZ21::notifyCvNACK(Throttle* inpThrottle, int inCvAddress)
{
	MessageConverterZ21::replyBuffer[0] = highByte(inCvAddress); // cv address msb
	MessageConverterZ21::replyBuffer[1] = lowByte(inCvAddress); // cv address lsb
	notify(HEADER_LAN_XPRESS_NET, LAN_X_HEADER_CV_NACK, LAN_X_DB0_CV_NACK, MessageConverterZ21::replyBuffer, 0, false, inpThrottle);
}

void MessageConverterZ21::notifyCvRead(Throttle* inpThrottle, int inCvAddress, int inValue)
{
	MessageConverterZ21::replyBuffer[0] = highByte(inCvAddress); // cv address msb
	MessageConverterZ21::replyBuffer[1] = lowByte(inCvAddress); // cv address lsb
	MessageConverterZ21::replyBuffer[2] = inValue; // cv value
	notify(HEADER_LAN_XPRESS_NET, LAN_X_HEADER_CV_RESULT, 0x14, MessageConverterZ21::replyBuffer, 3, false, inpThrottle);
}

//
// TODO Passer par un message texte pour éviter des bugs de collision core 1/0 !
//

void MessageConverterZ21::setSpeed(Throttle* inpThrottle, byte inNbSteps, byte inDB1, byte inDB2, byte inDB3)
{
#ifdef DCCPP_DEBUG_MODE
	Serial.print("Z21 Throttle ");
	Serial.print(inpThrottle->getId());
	Serial.println(" : speed");
#endif

	int locoAddress = ((inDB1 & 0x3F) << 8) + inDB2;

	Locomotive* pLoco = Locomotives::get(locoAddress);

	if (pLoco == NULL)
		pLoco = Locomotives::add("Loco", locoAddress, inNbSteps);

	bool isForward = bitRead(inDB3, 7);
	byte speed = inDB3;
	bitClear(speed, 7);
	pLoco->setDirection(isForward);
	pLoco->setDCCSpeed(speed);
	if ((this->broadcastFlags & BROADCAST_BASE) != 0)
		notifyLocoInfo(inpThrottle, inDB1, inDB2);
}

//
// TODO Passer par un message texte pour éviter des bugs de collision core 1/0 !
//

void MessageConverterZ21::setFunction(Throttle* inpThrottle, byte inDB1, byte inDB2, byte inDB3)
{
	int locoAddress = ((inDB1 & 0x3F) << 8) + inDB2;
	Locomotive* pLoco = Locomotives::get(locoAddress);

	if (pLoco == NULL)
	{
		Serial.println("Function on unknown loco !");
		return;
	}

#ifdef DCCPP_DEBUG_MODE
	Serial.print("Z21 Throttle ");
	Serial.print(inpThrottle->getId());
	Serial.println(" : function");
#endif

	// inDB3 :  TTNN NNNN		TT:00 off, TT:01 on; TT:10 toggle   NNNNNN function number

	byte action = bitRead(inDB3, 6) + 2 * bitRead(inDB3, 7);
	byte function = inDB3;
	bitClear(function, 6);
	bitClear(function, 7);

	bool activeFlag = action == 0b01;
	if (action == 0b10)
	{	// toggle
		bool isActivated = pLoco->functions.isActivated(function);
		activeFlag = !isActivated;
	}

	pLoco->setDCCFunction(function, activeFlag);
	if ((this->broadcastFlags & BROADCAST_BASE) != 0)
		notifyLocoInfo(inpThrottle, inDB1, inDB2);
}

//
// TODO Passer par un message texte pour éviter des bugs de collision core 1/0 !
//

void MessageConverterZ21::cvReadProg(Throttle* inpThrottle, byte inDB1, byte inDB2)
{
	int cvAddress = ((inDB1 & 0x3F) << 8) + inDB2 + 1;

#ifdef DCCPP_DEBUG_MODE
	Serial.print("Z21 Throttle ");
	Serial.print(inpThrottle->getId());
	Serial.print(" : cvReadProg  Cv:");
	Serial.println(cvAddress);
#endif

	int val = DCCpp::readCvProg(cvAddress);

	if (val == -1)
		notifyCvNACK(inpThrottle, cvAddress - 1);
	else
		notifyCvRead(inpThrottle, cvAddress - 1, val);
}

void MessageConverterZ21::cvReadMain(Throttle* inpThrottle, byte inDB1, byte inDB2)
{
	int cvAddress = ((inDB1 & 0x3F) << 8) + inDB2 + 1;

#ifdef DCCPP_DEBUG_MODE
	Serial.print("Z21 Throttle ");
	Serial.print(inpThrottle->getId());
	Serial.print(" : cvReadMain  Cv:");
	Serial.println(cvAddress);
#endif

	int val = DCCpp::readCvMain(cvAddress);

	if (val == -1)
		notifyCvNACK(inpThrottle, cvAddress - 1);
	else
		notifyCvRead(inpThrottle, cvAddress - 1, val);
}

//
// TODO Passer par un message texte pour éviter des bugs de collision core 1/0 !
//

void MessageConverterZ21::cvWriteProg(Throttle* inpThrottle, byte inDB1, byte inDB2, byte inDB3)
{
	int cvAddress = ((inDB1 & 0x3F) << 8) + inDB2 + 1;

#ifdef DCCPP_DEBUG_MODE
	Serial.print("Z21 Throttle ");
	Serial.print(inpThrottle->getId());
	Serial.print(" : cvWriteProg  Cv:");
	Serial.print(cvAddress);
	Serial.print(" Value:");
	Serial.println(inDB3);
#endif

	bool ret = false;
	if (DCCpp::IsProgTrackDeclared())
		ret = DCCpp::writeCvProg(cvAddress, inDB3);
	else
		ret = DCCpp::writeCvMain(cvAddress, inDB3);

	if (!ret)
		notifyCvNACK(inpThrottle, cvAddress - 1);
	else
		notifyCvRead(inpThrottle, cvAddress - 1, inDB3);
}

void MessageConverterZ21::cvReadPom(Throttle* inpThrottle, byte inDB1, byte inDB2, byte inDB3, byte inDB4)
{
	int locoAddress = ((inDB1 & 0x3F) << 8) + inDB2;
	int cvAddress = ((inDB3 & B00000011) << 8) + inDB4 + 1;

#ifdef DCCPP_DEBUG_MODE
	Serial.print("Z21 Throttle ");
	Serial.print(inpThrottle->getId());
	Serial.print(" : cvReadPom  Loco:");
	Serial.print(locoAddress);
	Serial.print("  Cv:");
	Serial.println(cvAddress);
#endif

	int val = DCCpp::readCvMain(cvAddress);
	if (val == -1)
		notifyCvNACK(inpThrottle, cvAddress - 1);
	else
	{
		notifyCvRead(inpThrottle, cvAddress - 1, val);
	}
}

/*void MessageConverterZ21::cvWritePom(Throttle* inpThrottle, byte inDB1, byte inDB2, byte inDB3, byte inDB4, byte inDB5)
{
	int locoAddress = ((inDB1 & 0x3F) << 8) + inDB2;
	int cvAddress = ((inDB3 & 0x3F) << 8) + inDB4 + 1;

#ifdef DCCPP_DEBUG_MODE
	Serial.print("Z21 Throttle ");
	Serial.print(inpThrottle->getId());
	Serial.println(" : cvWriteMain");
#endif

	bool ret = DCCpp::writeCvMain(cvAddress, inDB5);
	if (!ret)
		notifyCvNACK(inpThrottle, cvAddress - 1);
	else
		notifyCvRead(inpThrottle, cvAddress - 1, inDB3);
}
*/
bool MessageConverterZ21::processBuffer(Throttle* inpThrottle)
{
	bool done = false;
	byte DB[100];
	CircularBuffer* pBuffer = inpThrottle->pBuffer;

	if (pBuffer == NULL)
		return false;
	if (pBuffer->isEmpty())
		return false;

	int lengthData = pBuffer->GetInt16() - 4;	// length of the data = total length - length of length (!) - length of header
	int header = pBuffer->GetInt16();
	byte Xheader = 0;
	byte DB0 = 0;
	int nbLocos = Locomotives::count();

#ifdef DCCPP_DEBUG_VERBOSE_MODE
	char val[40];
	Serial.print(inpThrottle->getId());
	Serial.print(" <- ");
	sprintf(val, "0x%02X 0x%02X 0x%02X 0x%02X ", (lengthData + 4) % 256, (lengthData + 4) / 256, header % 256, header / 256);
	Serial.print(val);
#endif

	if (lengthData > 0)
	{
		pBuffer->GetBytes(DB, lengthData);
#ifdef DCCPP_DEBUG_VERBOSE_MODE
		for (int i = 0; i < lengthData; i++)
		{
			sprintf(val, "0x%02X ", DB[i]);
			Serial.print(val);
		}
#endif
	}

	switch (header)
	{
	case HEADER_LAN_XPRESS_NET:
		Xheader = DB[0];
		switch (Xheader)
		{
		case LAN_X_HEADER_GENERAL:
			DB0 = DB[1];
			switch (DB0)
			{
			case LAN_X_DB0_GET_VERSION:
#ifdef DCCPP_DEBUG_VERBOSE_MODE
				Serial.println(" GET_VERSION");
#endif
				break;
			case LAN_X_DB0_GET_STATUS:
#ifdef DCCPP_DEBUG_VERBOSE_MODE
				Serial.print(" GET_STATUS  ");
#endif
				notifyStatus(inpThrottle);
				done = true;
				break;
			case LAN_X_DB0_SET_TRACK_POWER_OFF:
#ifdef DCCPP_DEBUG_VERBOSE_MODE
				Serial.println(" POWER_OFF");
#endif
				//
				// TODO Passer par un message texte pour éviter des bugs de collision core 1/0 !
				//
				DCCpp::powerOff();
				done = true;
				break;
			case LAN_X_DB0_SET_TRACK_POWER_ON:
#ifdef DCCPP_DEBUG_VERBOSE_MODE
				Serial.println(" POWER_ON");
#endif
				//
				// TODO Passer par un message texte pour éviter des bugs de collision core 1/0 !
				//
				DCCpp::powerOn();
				done = true;
				break;
			}
			break;
		case LAN_X_HEADER_SET_STOP:
#ifdef DCCPP_DEBUG_VERBOSE_MODE
			Serial.println(" EMERGENCY_STOP");
#endif
			//
			// TODO Passer par un message texte pour éviter des bugs de collision core 1/0 !
			//
			Locomotives::emergencyStop();
			done = true;
			break;
		case LAN_X_HEADER_SET_LOCO:
			DB0 = DB[1];
			switch (DB0)
			{
			case LAN_X_DB0_LOCO_DCC14:
#ifdef DCCPP_DEBUG_VERBOSE_MODE
				Serial.println(" LOCO DCC 14 SPEED");
#endif
				setSpeed(inpThrottle, 14, DB[2], DB[3], DB[4]);
				done = true;
				break;
			case LAN_X_DB0_LOCO_DCC28:
#ifdef DCCPP_DEBUG_VERBOSE_MODE
				Serial.println(" LOCO DCC 28 SPEED");
#endif
				setSpeed(inpThrottle, 28, DB[2], DB[3], DB[4]);
				done = true;
				break;
			case LAN_X_DB0_LOCO_DCC128:
#ifdef DCCPP_DEBUG_VERBOSE_MODE
				Serial.println(" LOCO DCC 128 SPEED");
#endif
				setSpeed(inpThrottle, 128, DB[2], DB[3], DB[4]);
				done = true;
				break;
			case LAN_X_DB0_SET_LOCO_FUNCTION:
#ifdef DCCPP_DEBUG_VERBOSE_MODE
				Serial.println(" LOCO DCC FUNCTION");
#endif
				setFunction(inpThrottle, DB[2], DB[3], DB[4]);
#ifdef DCCPP_DEBUG_MODE
				byte function = DB[4];
				bitClear(function, 6);
				bitClear(function, 7);
				if (function == 12) // why not ?
				{
					Throttles::printThrottles();
					Locomotives::printLocomotives();
				}
#endif
				done = true;
				break;
			}
			break;
		case LAN_X_HEADER_GET_LOCO_INFO:
#ifdef DCCPP_DEBUG_VERBOSE_MODE
			Serial.print(" LOCO INFO: ");
#endif
			notifyLocoInfo(inpThrottle, DB[2], DB[3]);
			done = true;

			break;
		case LAN_X_HEADER_GET_TURNOUT_INFO:
#ifdef DCCPP_DEBUG_VERBOSE_MODE
			Serial.print(" TURNOUT INFO  ");
#endif
			notifyTurnoutInfo(inpThrottle, DB[1], DB[2]);
			done = true;

			break;
		case LAN_X_HEADER_GET_FIRMWARE_VERSION:
#ifdef DCCPP_DEBUG_VERBOSE_MODE
			Serial.print(" FIRMWARE VERSION  ");
#endif
			notifyFirmwareVersion(inpThrottle);
			done = true;
			break;
		case LAN_X_HEADER_CV_READ:
			if (DCCpp::IsProgTrackDeclared())
			{
#ifdef DCCPP_DEBUG_MODE
				Serial.print(" CV READ PROG ");
#endif
				// DB0 should be 0x11
				cvReadProg(inpThrottle, DB[2], DB[3]);
			}
			else
			{
				// If no prog track, read oin the main track !
#ifdef DCCPP_DEBUG_MODE
				Serial.print(" CV READ MAIN ");
#endif
				// DB0 should be 0x11
				cvReadMain(inpThrottle, DB[2], DB[3]);
			}
			done = true;
			break;
		case LAN_X_HEADER_CV_POM:
#ifdef DCCPP_DEBUG_MODE
			Serial.println(" CV READ POM");
#endif
			// DB0 should be 0x11
			cvReadPom(inpThrottle, DB[2], DB[3], DB[4], DB[5]);
			done = true;
			break;
		case LAN_X_HEADER_CV_WRITE:
#ifdef DCCPP_DEBUG_VERBOSE_MODE
			Serial.print(" CV WRITE ");
#endif
			notifyFirmwareVersion(inpThrottle);
			done = true;
			break;
		case LAN_X_HEADER_SET_TURNOUT:
		case 0x22:
			break;
		}
		break;

	case HEADER_LAN_SET_BROADCASTFLAGS:
		this->broadcastFlags = CircularBuffer::GetInt32(DB, 0);
#ifdef DCCPP_DEBUG_MODE
		Serial.print(" BROADCAST FLAGS : ");
		if (this->broadcastFlags & BROADCAST_BASE)	Serial.print("BASE /");
		if (this->broadcastFlags & BROADCAST_RBUS)	Serial.print("RBUS /");
		if (this->broadcastFlags & BROADCAST_RAILCOM)	Serial.print("RAILCOM /");
		if (this->broadcastFlags & BROADCAST_SYSTEM)	Serial.print("SYSTEM /");
		if (this->broadcastFlags & BROADCAST_BASE_LOCOINFO)	Serial.print("LOCOINFO /");
		if (this->broadcastFlags & BROADCAST_LOCONET)	Serial.print("LOCONET /");
		if (this->broadcastFlags & BROADCAST_LOCONET_LOCO)	Serial.print("LOCONET_LOCO /");
		if (this->broadcastFlags & BROADCAST_LOCONET_SWITCH)	Serial.print("LOCONET_SWTICH /");
		if (this->broadcastFlags & BROADCAST_LOCONET_DETECTOR)	Serial.print("LOCONET_DETECTOR /");
		if (this->broadcastFlags & BROADCAST_RAILCOM_AUTO)	Serial.print("RAILCOM_AUTO /");
		if (this->broadcastFlags & BROADCAST_CAN)	Serial.print("CAN /");
		Serial.println("");
#endif
		done = true;
		break;
	case HEADER_LAN_GET_LOCOMODE:
#ifdef DCCPP_DEBUG_VERBOSE_MODE
		Serial.print(" GETLOCOMODE ");
#endif
		notifyLocoMode(inpThrottle, DB[0], DB[1]);	// big endian here, but resend the same as received, so no problem.
		done = true;
		break;

	case HEADER_LAN_SET_LOCOMODE:
#ifdef DCCPP_DEBUG_VERBOSE_MODE
		Serial.println(" SETLOCOMODE ");
#endif
		done = true;
		break;
	case HEADER_LAN_GET_HWINFO:
#ifdef DCCPP_DEBUG_VERBOSE_MODE
		Serial.print(" GET_HWINFO ");
#endif
		notifyHWInfo(inpThrottle);	// big endian here, but resend the same as received, so no problem.
		done = true;
		break;
	case HEADER_LAN_LOGOFF:
#ifdef DCCPP_DEBUG_MODE
		Serial.println(" LOGOFF ");
#endif
		this->clientStop(inpThrottle);
		done = true;
		break;
	case HEADER_LAN_SYSTEMSTATE_GETDATA:
#ifdef DCCPP_DEBUG_VERBOSE_MODE
		Serial.print(" SYSTEMSTATE ");
#endif
		notifyStatus(inpThrottle);	// big endian here, but resend the same as received, so no problem.
		done = true;
		break;
	case HEADER_LAN_GET_SERIAL_NUMBER:
	case HEADER_LAN_GET_BROADCASTFLAGS:
	case HEADER_LAN_GET_TURNOUTMODE:
	case HEADER_LAN_SET_TURNOUTMODE:
	case HEADER_LAN_RMBUS_DATACHANGED:
	case HEADER_LAN_RMBUS_GETDATA:
	case HEADER_LAN_RMBUS_PROGRAMMODULE:
	case HEADER_LAN_RAILCOM_DATACHANGED:
	case HEADER_LAN_RAILCOM_GETDATA:
	case HEADER_LAN_LOCONET_DISPATCH_ADDR:
	case HEADER_LAN_LOCONET_DETECTOR:
		break;
	}

	if (!done)
	{
#ifdef DCCPP_DEBUG_MODE
		Serial.print("Z21 Throttle ");
		Serial.print(inpThrottle->getId());
		Serial.print(" : ");
		Serial.print(" not treated :  header:");
		Serial.print(header, HEX);
		Serial.print("  Xheader:");
		Serial.print(Xheader, HEX);
		Serial.print("  DB0:");
		Serial.println(DB0, HEX);
#endif
	}
	else
	{
		int newNbLocos = Locomotives::count();
		if (nbLocos != newNbLocos)
			Locomotives::printLocomotives();
	}
	return true;
}

#ifdef DCCPP_DEBUG_MODE
void MessageConverterZ21::printConverter()
{
	Serial.print("Z21 converter : ");
}
#endif
#endif