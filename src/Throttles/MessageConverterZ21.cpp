/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <Message converter base class>
*************************************************************/

#include "Arduino.h"
#include "DCCpp.h"

#if defined(USE_THROTTLES) && defined (USE_WIFI)

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

byte notifyBuffer[100];

// sizes : [       2        ][       2        ][inLengthData]
// bytes : [length1, length2][Header1, Header2][Data........]
bool notify(unsigned int inHeader, byte* inpData, unsigned int inLengthData, bool inXorInData, Throttle* inpThrottle)
{
	int realLength = (inLengthData + 4 + (inXorInData == false ? 1 : 0));

	notifyBuffer[0] = realLength % 256;
	notifyBuffer[1] = realLength / 256;
	notifyBuffer[2] = inHeader % 256;
	notifyBuffer[3] = inHeader / 256;
	memcpy(notifyBuffer + 4, inpData, inLengthData);

	if (!inXorInData)    // if xor byte not included in data, compute and write it !
	{
		byte xxor = 0;
		for (unsigned int i = 0; i < inLengthData; i++)
		{
			xxor ^= inpData[i];
		}
		notifyBuffer[inLengthData+4] = xxor;
	}

	inpThrottle->write(notifyBuffer, realLength);

	return true;
}

// sizes : [       2        ][       2        ][   1   ][inLengthData]
// bytes : [length1, length2][Header1, Header2][XHeader][Data........]
bool notify(unsigned int inHeader, unsigned int inXHeader, byte* inpData, unsigned int inLengthData, bool inXorInData, Throttle* inpThrottle)
{
	int realLength = (inLengthData + 5 + (inXorInData == false ? 1 : 0));

	notifyBuffer[0] = realLength % 256;
	notifyBuffer[1] = realLength / 256;
	notifyBuffer[2] = inHeader % 256;
	notifyBuffer[3] = inHeader / 256;
	notifyBuffer[4] = inXHeader;
	memcpy(notifyBuffer + 5, inpData, inLengthData);

	if (!inXorInData)    // if xor byte not included in data, compute and write it !
	{
		byte xxor = inXHeader;
		for (unsigned int i = 0; i < inLengthData; i++)
		{
			xxor ^= inpData[i];
		}
		notifyBuffer[inLengthData + 5] = xxor;
	}

	inpThrottle->write(notifyBuffer, realLength);

	return true;
}

// sizes : [       2        ][       2        ][   1   ][ 1 ][inLengthData]
// bytes : [length1, length2][Header1, Header2][XHeader][DB0][Data........]
bool notify(unsigned int inHeader, unsigned int inXHeader, byte inDB0, byte* inpData, unsigned int inLengthData, bool inXorInData, Throttle* inpThrottle)
{
	int realLength = (inLengthData + 6 + (inXorInData == false ? 1 : 0));

	notifyBuffer[0] = realLength % 256;
	notifyBuffer[1] = realLength / 256;
	notifyBuffer[2] = inHeader % 256;
	notifyBuffer[3] = inHeader / 256;
	notifyBuffer[4] = inXHeader;
	notifyBuffer[5] = inDB0;
	memcpy(notifyBuffer + 6, inpData, inLengthData);

	if (!inXorInData)    // if xor byte not included in data, compute and write it !
	{
		byte xxor = inXHeader^inDB0;
		for (unsigned int i = 0; i < inLengthData; i++)
		{
			xxor ^= inpData[i];
		}
		notifyBuffer[inLengthData + 6] = xxor;
	}

	inpThrottle->write(notifyBuffer, realLength);

	return true;
}

byte replyBuffer[100];

void notifyStatus(Throttle* inpThrottle)
{
	replyBuffer[0] = 0;	// main current 1
	replyBuffer[1] = 0; // main current 2
	replyBuffer[2] = 0; // prog current 1
	replyBuffer[3] = 0; // prog current 2
	replyBuffer[4] = 0; // filtered main current 1
	replyBuffer[5] = 0; // filtered main current 2
	replyBuffer[6] = 0; // Temperature 1
	replyBuffer[7] = 0; // Temperature 2
	replyBuffer[8] = 5; // Supply voltage 1
	replyBuffer[9] = 0; // supply voltage 2
	replyBuffer[10] = 16; // VCC voltage 1 
	replyBuffer[11] = 0; // VCC voltage 2
	replyBuffer[12] = 0b00000000;	// CentralState 
	replyBuffer[13] = 0b00000000; // CentralStateEx
	replyBuffer[14] = 0;
	replyBuffer[15] = 0;
	notify(HEADER_LAN_SYSTEMSTATE, replyBuffer, 16, true, inpThrottle);
}

void notifyLocoInfo(Throttle* inpThrottle, byte inMSB, byte inLSB)
{
	int locoAddress = ((inMSB & 0x3F) << 8) + inLSB;
	Locomotive* pLoco = Locomotives::get(locoAddress);

	if (pLoco == NULL)
		pLoco = Locomotives::add("Loco", locoAddress, 128);		// default

	replyBuffer[0] = inMSB;	// loco address msb
	replyBuffer[1] = inLSB; // loco address lsb
	replyBuffer[2] = B00000100; // 0000CKKK	 C = already controlled    KKK = speed steps 000:14, 010:28, 100:128
	replyBuffer[3] = pLoco->getSpeed(); // RVVVVVVV  R = forward    VVVVVVV = speed
	if (pLoco->isDirectionForward()) bitSet(replyBuffer[3], 7);

	replyBuffer[4] = B00000000; // 0DSLFGHJ  D = double traction    S = Smartsearch   L = F0   F = F4   G = F3   H = F2   J = F1
	if (pLoco->functions.isActivated(0)) bitSet(replyBuffer[4], 4);
	if (pLoco->functions.isActivated(1)) bitSet(replyBuffer[4], 0);
	if (pLoco->functions.isActivated(2)) bitSet(replyBuffer[4], 1);
	if (pLoco->functions.isActivated(3)) bitSet(replyBuffer[4], 2);
	if (pLoco->functions.isActivated(4)) bitSet(replyBuffer[4], 3);

	replyBuffer[5] = B00000000;	// function F5 to F12    F5 is bit0
	if (pLoco->functions.isActivated(5)) bitSet(replyBuffer[5], 0);
	if (pLoco->functions.isActivated(6)) bitSet(replyBuffer[5], 1);
	if (pLoco->functions.isActivated(7)) bitSet(replyBuffer[5], 2);
	if (pLoco->functions.isActivated(8)) bitSet(replyBuffer[5], 3);
	if (pLoco->functions.isActivated(9)) bitSet(replyBuffer[5], 4);
	if (pLoco->functions.isActivated(10)) bitSet(replyBuffer[5],5);
	if (pLoco->functions.isActivated(11)) bitSet(replyBuffer[5],6);
	if (pLoco->functions.isActivated(12)) bitSet(replyBuffer[5],7);

	replyBuffer[6] = B00000000;	// function F13 to F20   F13 is bit0
	if (pLoco->functions.isActivated(13)) bitSet(replyBuffer[6], 0);
	if (pLoco->functions.isActivated(14)) bitSet(replyBuffer[6], 1);
	if (pLoco->functions.isActivated(15)) bitSet(replyBuffer[6], 2);
	if (pLoco->functions.isActivated(16)) bitSet(replyBuffer[6], 3);
	if (pLoco->functions.isActivated(17)) bitSet(replyBuffer[6], 4);
	if (pLoco->functions.isActivated(18)) bitSet(replyBuffer[6], 5);
	if (pLoco->functions.isActivated(19)) bitSet(replyBuffer[6], 6);
	if (pLoco->functions.isActivated(20)) bitSet(replyBuffer[6], 7);

	replyBuffer[7] = B00000000;	// function F21 to F28   F21 is bit0
	if (pLoco->functions.isActivated(21)) bitSet(replyBuffer[7], 0);
	if (pLoco->functions.isActivated(22)) bitSet(replyBuffer[7], 1);
	if (pLoco->functions.isActivated(23)) bitSet(replyBuffer[7], 2);
	if (pLoco->functions.isActivated(24)) bitSet(replyBuffer[7], 3);
	if (pLoco->functions.isActivated(25)) bitSet(replyBuffer[7], 4);
	if (pLoco->functions.isActivated(26)) bitSet(replyBuffer[7], 5);
	if (pLoco->functions.isActivated(27)) bitSet(replyBuffer[7], 6);
	if (pLoco->functions.isActivated(28)) bitSet(replyBuffer[7], 7);

	notify(HEADER_LAN_XPRESS_NET, LAN_X_HEADER_LOCO_INFO, replyBuffer, 8, false, inpThrottle);
}

void notifyTurnoutInfo(Throttle* inpThrottle, byte inMSB, byte inLSB)
{
	replyBuffer[0] = inMSB;	// turnout address msb
	replyBuffer[1] = inLSB; // turnout address lsb
	replyBuffer[2] = B00000000; // 000000ZZ	 ZZ : 00 not switched   01 pos1  10 pos2  11 invalid
	notify(HEADER_LAN_XPRESS_NET, LAN_X_HEADER_TURNOUT_INFO, replyBuffer, 3, false, inpThrottle);
}

void notifyLocoMode(Throttle* inpThrottle, byte inMSB, byte inLSB)
{
	replyBuffer[0] = inMSB;	// loco address msb
	replyBuffer[1] = inLSB; // loco address lsb
	replyBuffer[2] = B00000000; // 00000000	DCC   00000001 MM
	notify(HEADER_LAN_GET_LOCOMODE, replyBuffer, 3, true, inpThrottle);
}

void notifyFirmwareVersion(Throttle* inpThrottle)
{
	replyBuffer[0] = 0x01;	// Version major in BCD
	replyBuffer[1] = 0x23;	// Version minor in BCD
	notify(HEADER_LAN_XPRESS_NET, LAN_X_HEADER_FIRMWARE_VERSION, 0x0A, replyBuffer, 2, false, inpThrottle);
}

void notifyHWInfo(Throttle* inpThrottle)
{
	replyBuffer[0] = 0x00;	// Hardware type in BCD on int32
	replyBuffer[1] = 0x02;	// Hardware type in BCD on int32
	replyBuffer[2] = 0x00;	// Hardware type in BCD on int32
	replyBuffer[3] = 0x00;	// Hardware type in BCD on int32
	replyBuffer[4] = 0x23;	// Firmware version in BCD on int32
	replyBuffer[5] = 0x01;	// Firmware version in BCD on int32
	replyBuffer[6] = 0x00;	// Firmware version in BCD on int32
	replyBuffer[7] = 0x00;	// Firmware version in BCD on int32
	notify(HEADER_LAN_GET_HWINFO, replyBuffer, 8, true, inpThrottle);
}

void notifyCvNACK(Throttle* inpThrottle, byte incvMSB, byte incvLSB)
{
	notify(HEADER_LAN_XPRESS_NET, LAN_X_HEADER_CV_NACK, LAN_X_DB0_CV_NACK, replyBuffer, 0, false, inpThrottle);
}

void notifyCvRead(Throttle* inpThrottle, byte incvMSB, byte incvLSB, int inValue)
{
	replyBuffer[0] = incvMSB;	// cv address msb
	replyBuffer[1] = incvLSB; // cv address lsb
	replyBuffer[2] = inValue; // cv value
	notify(HEADER_LAN_XPRESS_NET, LAN_X_HEADER_CV_RESULT, 0x14, replyBuffer, 3, false, inpThrottle);
}

void MessageConverterZ21::setSpeed(Throttle* inpThrottle, byte inNbSteps, byte inDB1, byte inDB2, byte inDB3)
{
	CircularBuffer* pBuffer = inpThrottle->getCircularBuffer();

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

void MessageConverterZ21::setFunction(Throttle* inpThrottle, byte inDB1, byte inDB2, byte inDB3)
{
	CircularBuffer* pBuffer = inpThrottle->getCircularBuffer();

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

void MessageConverterZ21::cvReadProg(Throttle* inpThrottle, byte inDB1, byte inDB2)
{
	int cvAddress = ((inDB1 & 0x3F) << 8) + inDB2;
	
#ifdef DCCPP_DEBUG_MODE
	Serial.print("Z21 Throttle ");
	Serial.print(inpThrottle->getId());
	Serial.println(" : cvReadProg");
#endif

	int val = -1;
	if (DCCpp::IsProgTrackDeclared())
		val = DCCpp::readCvProg(cvAddress);
	else
		val = DCCpp::readCvMain(cvAddress);

	if (val == -1)
		notifyCvNACK(inpThrottle, inDB1, inDB2);
	else
		notifyCvRead(inpThrottle, inDB1, inDB2, val);
}

void MessageConverterZ21::cvWriteProg(Throttle* inpThrottle, byte inDB1, byte inDB2, byte inDB3)
{
	int cvAddress = ((inDB1 & 0x3F) << 8) + inDB2;

#ifdef DCCPP_DEBUG_MODE
	Serial.print("Z21 Throttle ");
	Serial.print(inpThrottle->getId());
	Serial.println(" : cvWriteProg");
#endif

	bool ret = false;
	if (DCCpp::IsProgTrackDeclared())
		ret = DCCpp::writeCvProg(cvAddress, inDB3);
	else
		ret = DCCpp::writeCvMain(cvAddress, inDB3);

	if (!ret)
		notifyCvNACK(inpThrottle, inDB1, inDB2);
	else
		notifyCvRead(inpThrottle, inDB1, inDB2, inDB3);
}

/*void MessageConverterZ21::cvReadPom(Throttle* inpThrottle, byte inDB1, byte inDB2, byte inDB3, byte inDB4)
{
	int locoAddress = ((inDB1 & 0x3F) << 8) + inDB2;
	int cvAddress = ((inDB3 & 0x3F) << 8) + inDB4;

#ifdef DCCPP_DEBUG_MODE
	Serial.print("Z21 Throttle ");
	Serial.print(inpThrottle->getId());
	Serial.println(" : cvReadMain");
#endif

	int val = DCCpp::readCvMain(cvAddress);
	if (val == -1)
		notifyCvNACK(inpThrottle, inDB1, inDB2);
	else
		notifyCvRead(inpThrottle, inDB1, inDB2, val);
}

void MessageConverterZ21::cvWritePom(Throttle* inpThrottle, byte inDB1, byte inDB2, byte inDB3, byte inDB4, byte inDB5)
{
	int locoAddress = ((inDB1 & 0x3F) << 8) + inDB2;
	int cvAddress = ((inDB3 & 0x3F) << 8) + inDB4;

#ifdef DCCPP_DEBUG_MODE
	Serial.print("Z21 Throttle ");
	Serial.print(inpThrottle->getId());
	Serial.println(" : cvWriteMain");
#endif

	bool ret = DCCpp::writeCvMain(cvAddress, inDB5);
	if (!ret)
		notifyCvNACK(inpThrottle, inDB1, inDB2);
	else
		notifyCvRead(inpThrottle, inDB1, inDB2, inDB3);
}
*/
bool MessageConverterZ21::processBuffer(Throttle* inpThrottle)
{
	bool done = false;
	byte DB[100];
	CircularBuffer* pBuffer = inpThrottle->getCircularBuffer();

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
				DCCpp::powerOff();
				done = true;
				break;
			case LAN_X_DB0_SET_TRACK_POWER_ON:
#ifdef DCCPP_DEBUG_VERBOSE_MODE
				Serial.println(" POWER_ON");
#endif
				DCCpp::powerOn();
				done = true;
				break;
			}
			break;
		case LAN_X_HEADER_SET_STOP:
#ifdef DCCPP_DEBUG_VERBOSE_MODE
			Serial.println(" EMERGENCY_STOP");
#endif
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
#ifdef DCCPP_DEBUG_MODE
			Serial.print(" CV READ PROG ");
#endif
			// DB0 should be 0x11
			cvReadProg(inpThrottle, DB[2], DB[3]);
			done = true;
			break;
		case LAN_X_HEADER_CV_POM:
#ifdef DCCPP_DEBUG_MODE
			Serial.println(" CV READ POM : NOT IMPLEMENTED");
#endif
			// DB0 should be 0x11
			//cvReadMain(inpThrottle, DB[2], DB[3], DB[4], DB[5]);
			//done = true;
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