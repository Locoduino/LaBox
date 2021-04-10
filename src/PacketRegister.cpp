/**********************************************************************

PacketRegister.cpp
COPYRIGHT (c) 2013-2016 Gregg E. Berman
Adapted by Thierry PARIS

Part of DCC++ BASE STATION for the Arduino
**********************************************************************/

#include "Arduino.h"

#include "DCCpp.h"
//#include "DCCpp_Uno.h"
//#include "PacketRegister.h"
//#include "Comm.h"

byte DCCppConfig::SignalEnablePinMain = UNDEFINED_PIN;
byte DCCppConfig::CurrentMonitorMain = UNDEFINED_PIN;
					
byte DCCppConfig::SignalEnablePinProg = UNDEFINED_PIN;
byte DCCppConfig::CurrentMonitorProg = UNDEFINED_PIN;
					
#ifndef USE_ONLY1_INTERRUPT
byte DCCppConfig::DirectionMotorA = UNDEFINED_PIN;
byte DCCppConfig::DirectionMotorB = UNDEFINED_PIN;
#else
uint8_t	DCCppConfig::SignalPortMaskMain = 0;
uint8_t	DCCppConfig::SignalPortMaskProg = 0;

volatile uint8_t *DCCppConfig::SignalPortInMain = 0;
volatile uint8_t *DCCppConfig::SignalPortInProg = 0;
#endif

///////////////////////////////////////////////////////////////////////////////

void Register::initPackets(){
  activePacket = packet;
  updatePacket = packet+1;
} // Register::initPackets

///////////////////////////////////////////////////////////////////////////////
	
RegisterList::RegisterList(int maxNumRegs){
  this->maxNumRegs = maxNumRegs;
  this->reg = (Register *)calloc((maxNumRegs + 1), sizeof(Register));
  for(int i = 0; i <= maxNumRegs; i++)
		this->reg[i].initPackets();
  this->regMap = (Register **)calloc((maxNumRegs + 1), sizeof(Register *));
  this->speedTable = (int *)calloc((maxNumRegs + 1), sizeof(int *));
  this->currentReg = this->reg;
  this->regMap[0] = this->reg;
  this->maxLoadedReg = this->reg;
  this->nextReg = NULL;
  this->currentBit = 0;
  this->nRepeat = 0;
} // RegisterList::RegisterList
  
///////////////////////////////////////////////////////////////////////////////

// LOAD DCC PACKET INTO TEMPORARY REGISTER 0, OR PERMANENT REGISTERS 1 THROUGH DCC_PACKET_QUEUE_MAX (INCLUSIVE)
// CONVERTS 2, 3, 4, OR 5 BYTES INTO A DCC BIT STREAM WITH PREAMBLE, CHECKSUM, AND PROPER BYTE SEPARATORS
// BITSTREAM IS STORED IN UP TO A 10-BYTE ARRAY (USING AT MOST 76 OF 80 BITS)

void RegisterList::loadPacket(int nReg, byte *b, int nBytes, int nRepeat, int printFlag) volatile 
{  
#ifdef VISUALSTUDIO
  return;
#endif
  nReg = nReg % ((this->maxNumRegs + 1));          // force nReg to be between 0 and maxNumRegs, inclusive

  while(this->nextReg != NULL);              // pause while there is a Register already waiting to be updated -- nextReg will be reset to NULL by interrupt when prior Register updated fully processed
 
  if(this->regMap[nReg] == NULL)              // first time this Register Number has been called
   this->regMap[nReg] = this->maxLoadedReg+1;       // set Register Pointer for this Register Number to next available Register
 
  Register *r = this->regMap[nReg];           // set Register to be updated
  Packet *p = r->updatePacket;          // set Packet in the Register to be updated
  byte *buf = p->buf;                   // set byte buffer in the Packet to be updated
		  
  b[nBytes] = b[0];                        // copy first byte into what will become the checksum byte  
  for(int i = 1; i < nBytes; i++)              // XOR remaining bytes into checksum byte
		b[nBytes] ^= b[i];
  nBytes++;                              // increment number of bytes in packet to include checksum byte
	  
  buf[0] = 0xFF;                        // first 8 bytes of 22-byte preamble
  buf[1] = 0xFF;                        // second 8 bytes of 22-byte preamble
  buf[2] = 0xFC + bitRead(b[0], 7);      // last 6 bytes of 22-byte preamble + data start bit + b[0], bit 7
  buf[3] = b[0] << 1;                     // b[0], bits 6-0 + data start bit
  buf[4] = b[1];                        // b[1], all bits
  buf[5] = b[2] >> 1;                     // b[2], bits 7-1
  buf[6] = b[2] << 7;                     // b[2], bit 0
  
  if(nBytes == 3) {
		p->nBits=49;
  } 
	else {
		buf[6]+=b[3]>>2;                  // b[3], bits 7-2
		buf[7]=b[3]<<6;                   // b[3], bit 1-0
		if(nBytes==4){
			p->nBits=58;
		} 
		else {
			buf[7]+=b[4]>>3;                // b[4], bits 7-3
			buf[8]=b[4]<<5;                 // b[4], bits 2-0
			if(nBytes==5) {
				p->nBits=67;
			} 
			else {
				buf[8]+=b[5]>>4;              // b[5], bits 7-4
				buf[9]=b[5]<<4;               // b[5], bits 3-0
				p->nBits=76;
			} // >5 bytes
		} // >4 bytes
  } // >3 bytes
  
  this->nextReg = r;
  this->nRepeat = nRepeat;
  this->maxLoadedReg = max(this->maxLoadedReg, this->nextReg);
  
#ifdef DCCPP_DEBUG_MODE
#ifdef DCCPP_DEBUG_VERBOSE_MODE
  if(printFlag)       // for debugging purposes
		printPacket(nReg,b,nBytes,nRepeat);  
#endif
#endif

} // RegisterList::loadPacket

///////////////////////////////////////////////////////////////////////////////

void RegisterList::setThrottle(int nReg, int cab, int tSpeed, int tDirection) volatile 
{
	byte b[5];                      // save space for checksum byte
	byte nB = 0;

	if (cab>127)
		b[nB++] = highByte(cab) | 0xC0;      // convert train number into a two-byte address

	b[nB++] = lowByte(cab);
	b[nB++] = 0x3F;                        // 128-step speed control byte
	if (tSpeed >= 0)
		b[nB++] = tSpeed + (tSpeed>0) + tDirection * 128;   // max speed is 126, but speed codes range from 2-127 (0=stop, 1=emergency stop)
	else {
		b[nB++] = 1;
		tSpeed = 0;
	}

	loadPacket(nReg, b, nB, 0, 1);

#if defined(USE_TEXTCOMMAND)
	DCCPP_INTERFACE.print("<T");
	DCCPP_INTERFACE.print(nReg); DCCPP_INTERFACE.print(" ");
	DCCPP_INTERFACE.print(cab); DCCPP_INTERFACE.print(" ");
	DCCPP_INTERFACE.print(tSpeed); DCCPP_INTERFACE.print(" ");
	DCCPP_INTERFACE.print(tDirection);
	DCCPP_INTERFACE.print(">");
	DCCPP_INTERFACE.sendNewline();
#endif
	speedTable[nReg] = tDirection == 1 ? tSpeed : -tSpeed;

#ifdef USE_HMI
	if (hmi::CurrentInterface != NULL)
	{
		hmi::CurrentInterface->ChangeDirection(cab, tDirection);
		hmi::CurrentInterface->ChangeSpeed(cab, tSpeed);
	}
#endif
#ifdef USE_LOCOMOTIVES
	Locomotive* loco = Locomotives::get(cab);
	if (loco == NULL)
	{
		char name[10];
		sprintf(name, "%04d", cab);
		Locomotives::add(name, cab, 128, nReg);
		loco = Locomotives::get(cab);
	}
	if (loco != NULL)
	{
		if (tSpeed < 2)
			loco->setSpeed(0);
		else
			loco->setSpeed(tSpeed - 1);
		loco->setDirection(tDirection);
	}
#endif
} // RegisterList::setThrottle(ints)

#ifdef USE_TEXTCOMMAND
void RegisterList::setThrottle(const char *s) volatile
{
  int nReg;
  int cab;
  int tSpeed;
  int tDirection;
  
  if (sscanf(s, "%d %d %d %d", &nReg, &cab, &tSpeed, &tDirection) != 4)
  {
#ifdef DCCPP_DEBUG_MODE
    Serial.println(F("t Syntax error"));
#endif
	return;
  }

  this->setThrottle(nReg, cab, tSpeed, tDirection);
} // RegisterList::setThrottle(string)
#endif

///////////////////////////////////////////////////////////////////////////////

void checkFunction(int cab, int theByte, int startFunc, int endFunc,
#ifdef USE_LOCOMOTIVES
	Locomotive* pLoco)
#else
	int dummyLoco)
#endif
{
	bool state = false;
	for (int func = startFunc; func <= endFunc; func++)
	{
		state = (theByte & (1 << (func - startFunc))) > 0;
#ifdef USE_HMI
		if (hmi::CurrentInterface != NULL)
		{
			hmi::CurrentInterface->ChangeFunction(cab, func, state);
		}
#endif
#ifdef USE_LOCOMOTIVES
		if (pLoco != NULL)
		{
			pLoco->setFunction(func, state);
		}
#endif
	}
}

void RegisterList::setFunction(int nReg, int cab, int fByte, int eByte, bool returnMessages) volatile
{
	byte b[5];                      // save space for checksum byte
	byte nB = 0;

	if (cab > 127)
		b[nB++] = highByte(cab) | 0xC0;      // convert train number into a two-byte address

	b[nB++] = lowByte(cab);

	if (eByte < 0) {                      // this is a request for functions FL,F1-F12  
		b[nB++] = (fByte | 0x80) & 0xBF;     // for safety this guarantees that first nibble of function byte will always be of binary form 10XX which should always be the case for FL,F1-F12  
	}
	else {                             // this is a request for functions F13-F28
		b[nB++] = (fByte | 0xDE) & 0xDF;     // for safety this guarantees that first byte will either be 0xDE (for F13-F20) or 0xDF (for F21-F28)
		b[nB++] = eByte;
	}

#if defined(USE_TEXTCOMMAND)
	if (returnMessages)
	{
		DCCPP_INTERFACE.print("<F");
		DCCPP_INTERFACE.print(nReg); DCCPP_INTERFACE.print(" ");
		DCCPP_INTERFACE.print(cab); DCCPP_INTERFACE.print(" ");
		DCCPP_INTERFACE.print(fByte); DCCPP_INTERFACE.print(" ");
		DCCPP_INTERFACE.print(eByte);
		DCCPP_INTERFACE.print(">");
		DCCPP_INTERFACE.sendNewline();
	}
#endif

	bool state = false;
#ifdef USE_LOCOMOTIVES
	Locomotive* loco = Locomotives::get(cab);
	if (loco == NULL)
	{
		char name[10];
		sprintf(name, "%04d", cab);
		Locomotives::add(name, cab, 128);
		loco = Locomotives::get(cab);
	}
#else
	int loco = 0;
#endif

	/* Block 0
	*
	* To set functions F0 - F4 on(= 1) or off(= 0) :
	*
	* BYTE1 : 128 + F1 * 1 + F2 * 2 + F3 * 4 + F4 * 8 + F0 * 16
	* BYTE2 : omitted
	*/
	if ((fByte & B11100000) == B10000000 && eByte == -1)
	{
		fByte &= B01111111;		// remove bit 7.
		for (int func = 0; func <= 4; func++)
		{
			if (func == 0)
			{
				state = (fByte & B00010000) > 0;
			}
			else
			{
				state = (fByte & (1 << (func - 1))) > 0;
			}
#ifdef USE_HMI
			if (hmi::CurrentInterface != NULL)
				hmi::CurrentInterface->ChangeFunction(cab, func, state);
#endif
#ifdef USE_LOCOMOTIVES
			if (loco != NULL)
			{
				loco->setFunction(func, state);
			}
#endif
		}
	}

	/* Block 1
	*	To set functions F5 - F8 on(= 1) or off(= 0) :
	*
	*	BYTE1 : 176 + F5 * 1 + F6 * 2 + F7 * 4 + F8 * 8
	*	BYTE2 : omitted
	*/
	if ((fByte & B11110000) == B10110000 && eByte == -1)
	{
		fByte -= 176;
		checkFunction(cab, fByte, 5, 8, loco);
	}

	/* Block 2
	*	To set functions F9 - F12 on(= 1) or off(= 0) :
	*
	*	BYTE1 : 160 + F9 * 1 + F10 * 2 + F11 * 4 + F12 * 8
	*	BYTE2 : omitted
	*/
	if ((fByte & B11110000) == B10100000 && eByte == -1)
	{
		fByte -= 160;
		checkFunction(cab, fByte, 9, 12, loco);
	}

	/* Block 3
	*	To set functions F13 - F20 on(= 1) or off(= 0) :
	*
	*	BYTE1 : 222
	*	BYTE2 : F13 * 1 + F14 * 2 + F15 * 4 + F16 * 8 + F17 * 16 + F18 * 32 + F19 * 64 + F20 * 128
	*/
	if (fByte == 222 && eByte != -1)
	{
		checkFunction(cab, eByte, 13, 20, loco);
	}

	/* Block 4
	* To set functions F21 - F28 on(= 1) of off(= 0) :

	* BYTE1 : 223
	* BYTE2 : F21 * 1 + F22 * 2 + F23 * 4 + F24 * 8 + F25 * 16 + F26 * 32 + F27 * 64 + F28 * 128
	*/
	if (fByte == 223 && eByte != -1)
	{
		checkFunction(cab, eByte, 21, 28, loco);
	}

#ifdef USE_HMI
	if (hmi::CurrentInterface != NULL)
	{
		bool ret = false;
		do {
			ret = hmi::CurrentInterface->HmiInterfaceLoop();
		} while (ret);
	}
#endif
#ifdef USE_LOCOMOTIVES
	if (loco != NULL)
	{
		loco->functions.statesSent();
	}
#endif

	/* NMRA DCC norm ask for two DCC packets instead of only one:
	"Command Stations that generate these packets, and which are not periodically refreshing these functions,
	must send at least two repetitions of these commands when any function state is changed."
	https://www.nmra.org/sites/default/files/s-9.2.1_2012_07.pdf
	*/
	loadPacket(nReg, b, nB, (DCCpp::IsResendFunctions() == true) ? 1 : 4, 1);
} // RegisterList::setFunction(ints)

#ifdef USE_TEXTCOMMAND
void RegisterList::setFunction(const char *s, bool returnMessages) volatile
{
	int reg, cab;
	int fByte, eByte;

	int a, b, c, d, e;

	switch (sscanf(s, "%d %d %d %d %d", &a, &b, &c, &d, &e))
	{
	case 2:
		reg = 0;
		cab = a;
		fByte = b;
		eByte = -1;
		break;

	case 3:
		reg = 0;
		cab = a;
		fByte = b;
		eByte = c;
		break;
	case 4:
		reg = b;
		cab = c;
		fByte = d;
		eByte = -1;
		break;
	case 5:
		reg = b;
		cab = c;
		fByte = d;
		eByte = e;
		break;

	default:
#ifdef DCCPP_DEBUG_MODE
		Serial.println(F("f Syntax error"));
#endif
		return;
	}

	this->setFunction(reg, cab, fByte, eByte, returnMessages);

} // RegisterList::setFunction(string)

#if defined(DCCPP_DEBUG_MODE) && defined(USE_LOCOMOTIVES) && defined(USE_TEXTCOMMAND)
void RegisterList::testFunctionCommands() volatile
{
	Locomotive* loco = Locomotives::get(3);
	if (loco != NULL)
		loco->printLocomotive();

	// Block 0
	setFunction("3 144", false);  // F0
	setFunction("3 145", false);  // F1
	setFunction("3 147", false);  // F2
	setFunction("3 151", false);  // F3
	setFunction("3 159", false);  // F4
	if (loco != NULL)
		loco->printLocomotive();

	// Block 1
	setFunction("3 177", false);  // F5
	setFunction("3 179", false);  // F6
	setFunction("3 183", false);  // F7
	setFunction("3 191", false);  // F8
	if (loco != NULL)
		loco->printLocomotive();

	// Block 2
	setFunction("3 161", false);  // F9
	setFunction("3 163", false);  // F10
	setFunction("3 167", false);  // F11
	setFunction("3 175", false);  // F12
	if (loco != NULL)
		loco->printLocomotive();

	// Block 3
	setFunction("3 222 1", false);  // F13
	setFunction("3 222 3", false);  // F14
	setFunction("3 222 7", false);  // F15
	setFunction("3 222 15", false);  // F16
	setFunction("3 222 31", false);  // F17
	setFunction("3 222 63", false);  // F18
	setFunction("3 222 127", false);  // F19
	setFunction("3 222 255", false);  // F20
	if (loco != NULL)
		loco->printLocomotive();

	// Block 4
	setFunction("3 223 1", false);  // F21
	setFunction("3 223 3", false);  // F22
	setFunction("3 223 7", false);  // F23
	setFunction("3 223 15", false);  // F24
	setFunction("3 223 31", false);  // F25
	setFunction("3 223 63", false);  // F26
	setFunction("3 223 127", false);  // F27
	setFunction("3 223 255", false);  // F28
	if (loco != NULL)
		loco->printLocomotive();
}
#endif

#endif

  ///////////////////////////////////////////////////////////////////////////////

void RegisterList::setAccessory(int aAdd, int aNum, int activate) volatile 
{
	byte b[3];                      // save space for checksum byte

	b[0] = aAdd % 64 + 128;         // first byte is of the form 10AAAAAA, where AAAAAA represent 6 least significant bits of accessory address  
	b[1] = ((((aAdd / 64) % 8) << 4) + (aNum % 4 << 1) + activate % 2) ^ 0xF8;      // second byte is of the form 1AAACDDD, where C should be 1, and the least significant D represent activate/deactivate

	loadPacket(0, b, 2, 4, 1);

} // RegisterList::setAccessory(ints)

#ifdef USE_TEXTCOMMAND
void RegisterList::setAccessory(const char *s) volatile
{
	int aAdd;                       // the accessory address (0-511 = 9 bits) 
	int aNum;                       // the accessory number within that address (0-3)
	int activate;                   // flag indicated whether accessory should be activated (1) or deactivated (0) following NMRA recommended convention

	if (sscanf(s, "%d %d %d", &aAdd, &aNum, &activate) != 3)
	{
#ifdef DCCPP_DEBUG_MODE
		Serial.println(F("a Syntax error"));
#endif
		return;
	}

	this->setAccessory(aAdd, aNum, activate);

} // RegisterList::setAccessory(string)
#endif

  ///////////////////////////////////////////////////////////////////////////////

void RegisterList::writeTextPacket(int nReg, byte *b, int nBytes) volatile 
{

	if (nBytes<2 || nBytes>5) {    // invalid valid packet
		DCCPP_INTERFACE.print("<mInvalid Packet>");
		DCCPP_INTERFACE.sendNewline();
		return;
	}

	loadPacket(nReg, b, nBytes, 0, 1);

} // RegisterList::writeTextPacket(bytes)

#ifdef USE_TEXTCOMMAND
void RegisterList::writeTextPacket(const char *s) volatile
{
	int nReg;
	byte b[6];
	int nBytes;

	nBytes = sscanf(s, "%d %hhx %hhx %hhx %hhx %hhx", &nReg, b, b + 1, b + 2, b + 3, b + 4) - 1;

	this->writeTextPacket(nReg, b, nBytes);

} // RegisterList::writeTextPacket(string)
#endif

  ///////////////////////////////////////////////////////////////////////////////

int RegisterList::buildBaseAcknowlegde(int inMonitorPin) volatile
{
	int base = 0;
	for (int j = 0; j < ACK_BASE_COUNT; j++)
	{
		int val = (int)analogRead(inMonitorPin);
		base += val;
	}

	return base / ACK_BASE_COUNT;
}

int RegisterList::checkAcknowlegde(int inMonitorPin, int inBase) volatile
{
	int c = 0;
	int max = 0;
	int loop = 0;

	for (int a = 0; a < 20; a++)
	{
		c = 0;
		for (int j = 0; j < ACK_SAMPLE_COUNT; j++)
		{
			int val = (int)analogRead(inMonitorPin);
			c = (int)((val - inBase) * ACK_SAMPLE_SMOOTHING + c * (1.0 - ACK_SAMPLE_SMOOTHING));
			if (c > max)
			{
				max = c;
				loop = a;
			}
		}
	}

#ifdef DCCPP_DEBUG_MODE
	Serial.print(F(" iter : "));
	Serial.print(loop);
	Serial.print(", max : ");
	Serial.println(max);
#endif

	return (max > ACK_SAMPLE_THRESHOLD);
}

int RegisterList::readCVraw(int cv, int callBack, int callBackSub) volatile
{
	byte bRead[4];
	int bValue;
	int ret, base;

	cv--;                              // actual CV addresses are cv-1 (0-1023)

	byte MonitorPin = DCCppConfig::CurrentMonitorProg;
	if (DCCpp::IsMainTrack(this))
		MonitorPin = DCCppConfig::CurrentMonitorMain;

	// A read cannot be done if a monitor pin is not defined !
	if (MonitorPin == UNDEFINED_PIN)
		return -1;

#ifdef DCCPP_DEBUG_MODE
	Serial.print(F("readCVraw : start reading cv "));
	Serial.println(cv+1);
#endif

	bRead[0] = 0x78 + (highByte(cv) & 0x03);   // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
	bRead[1] = lowByte(cv);

	bValue = 0;

	for (int i = 0; i<8; i++) {

		base = RegisterList::buildBaseAcknowlegde(MonitorPin);
        
    delay(10);

		bRead[2] = 0xE8 + i;

		loadPacket(0, resetPacket, 2, 3);          // NMRA recommends starting with 3 reset packets
		loadPacket(0, bRead, 3, 5);                // NMRA recommends 5 verify packets
//		loadPacket(0, resetPacket, 2, 1);          // forces code to wait until all repeats of bRead are completed (and decoder begins to respond)
		loadPacket(0, idlePacket, 2, 6);          // NMRA recommends 6 idle or reset packets for decoder recovery time
#ifdef DCCPP_DEBUG_MODE
    Serial.print(F("bit : "));
    Serial.print(i);
#endif
    delay(2);	        

		ret = RegisterList::checkAcknowlegde(MonitorPin, base);

		bitWrite(bValue, i, ret);
	}
  delay(10);

	base = RegisterList::buildBaseAcknowlegde(MonitorPin);

  delay(10);

	bRead[0] = 0x74 + (highByte(cv) & 0x03);   // set-up to re-verify entire byte
	bRead[2] = bValue;

	loadPacket(0, resetPacket, 2, 3);       // NMRA recommends starting with 3 reset packets
	loadPacket(0, bRead, 3, 5);             // NMRA recommends 5 verify packets
	//loadPacket(0, resetPacket, 2, 1);     // forces code to wait until all repeats of bRead are completed (and decoder begins to respond)
	loadPacket(0, idlePacket, 2, 6);				// NMRA recommends 6 idle or reset packets for decoder recovery time
#ifdef DCCPP_DEBUG_MODE
  Serial.print(F("verif : "));
#endif
  delay(2);

	ret = RegisterList::checkAcknowlegde(MonitorPin, base);

	if (ret == 0)    // verify unsuccessful
		bValue = -1;

#ifdef USE_THROTTLES
	if (TextCommand::pCurrentThrottle == NULL)
		return bValue;
#endif

#if defined(USE_TEXTCOMMAND)
	DCCPP_INTERFACE.print("<r");
	DCCPP_INTERFACE.print(callBack);
	DCCPP_INTERFACE.print("|");
	DCCPP_INTERFACE.print(callBackSub);
	DCCPP_INTERFACE.print("|");
	DCCPP_INTERFACE.print(cv + 1);
	DCCPP_INTERFACE.print(" ");
	DCCPP_INTERFACE.print(bValue);
	DCCPP_INTERFACE.print(">");
	DCCPP_INTERFACE.sendNewline();
#endif

#ifdef DCCPP_DEBUG_MODE
	Serial.println(F("end reading"));
#endif
	return bValue;
}

int RegisterList::readCV(int cv, int callBack, int callBackSub) volatile 
{
	Serial.println("RegisterList::readCv");
	return RegisterList::readCVraw(cv, callBack, callBackSub);
} // RegisterList::readCV(ints)

#ifdef USE_TEXTCOMMAND
int RegisterList::readCV(const char *s) volatile
{
	int cv, callBack, callBackSub;

	if (sscanf(s, "%d %d %d", &cv, &callBack, &callBackSub) != 3)          // cv = 1-1024
	{
#ifdef DCCPP_DEBUG_MODE
		Serial.println(F("R Syntax error"));
#endif
		return -1;
	}

	return this->readCV(cv, callBack, callBackSub);
} // RegisterList::readCV(string)
#endif

int RegisterList::readCVmain(int cv, int callBack, int callBackSub) volatile
{
	return RegisterList::readCVraw(cv, callBack, callBackSub);

} // RegisterList::readCV_Main()

#ifdef USE_TEXTCOMMAND
int RegisterList::readCVmain(const char *s) volatile
{
	int cv, callBack, callBackSub;

	if (sscanf(s, "%d %d %d", &cv, &callBack, &callBackSub) != 3)          // cv = 1-1024
	{
#ifdef DCCPP_DEBUG_MODE
		Serial.println(F("r Syntax error"));
#endif
		return -1;
	}
   
	return this->readCVmain(cv, callBack, callBackSub);
} // RegisterList::readCVmain(string)
#endif

///////////////////////////////////////////////////////////////////////////////

bool RegisterList::writeCVByte(int cv, int bValue, int callBack, int callBackSub) volatile 
{
	byte bWrite[4];
	bool ok = false;
	int ret, base;

	cv--;                              // actual CV addresses are cv-1 (0-1023)

	bWrite[0] = 0x7C + (highByte(cv) & 0x03);   // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
	bWrite[1] = lowByte(cv);
	bWrite[2] = bValue;

	loadPacket(0, resetPacket, 2, 3);        // NMRA recommends starting with 3 reset packets
	loadPacket(0, bWrite, 3, 5);             // NMRA recommends 5 verify packets
	loadPacket(0, bWrite, 3, 6);             // NMRA recommends 6 write or reset packets for decoder recovery time

	// If monitor pin undefined, write cv without any confirmation...
	if (DCCppConfig::CurrentMonitorProg != UNDEFINED_PIN)
	{
		base = RegisterList::buildBaseAcknowlegde(DCCppConfig::CurrentMonitorProg);

		bWrite[0] = 0x74 + (highByte(cv) & 0x03);   // set-up to re-verify entire byte

		loadPacket(0, resetPacket, 2, 3);          // NMRA recommends starting with 3 reset packets
		loadPacket(0, bWrite, 3, 5);               // NMRA recommends 5 verify packets
		loadPacket(0, bWrite, 3, 6);               // NMRA recommends 6 write or reset packets for decoder recovery time

		ret = RegisterList::checkAcknowlegde(DCCppConfig::CurrentMonitorProg, base);

		loadPacket(0, resetPacket, 2, 1);        // Final reset packet (and decoder begins to respond)

		if (ret != 0)    // verify successful
			ok = true;
	}

#if defined(USE_TEXTCOMMAND)
	DCCPP_INTERFACE.print("<r");
	DCCPP_INTERFACE.print(callBack);
	DCCPP_INTERFACE.print("|");
	DCCPP_INTERFACE.print(callBackSub);
	DCCPP_INTERFACE.print("|");
	DCCPP_INTERFACE.print(cv + 1);
	DCCPP_INTERFACE.print(" ");
	DCCPP_INTERFACE.print(bValue);
	DCCPP_INTERFACE.print(">");

	if (ok)
		DCCPP_INTERFACE.print("ok");
	else
		DCCPP_INTERFACE.print("fail");

	DCCPP_INTERFACE.sendNewline();
#endif

	return ok;
} // RegisterList::writeCVByte(ints)

#ifdef USE_TEXTCOMMAND
bool RegisterList::writeCVByte(const char *s) volatile
{
	int bValue, cv, callBack, callBackSub;

	if (sscanf(s, "%d %d %d %d", &cv, &bValue, &callBack, &callBackSub) != 4)          // cv = 1-1024
	{
#ifdef DCCPP_DEBUG_MODE
		Serial.println(F("W Syntax error"));
#endif
		return false;
	}

	return this->writeCVByte(cv, bValue, callBack, callBackSub);
} // RegisterList::writeCVByte(string)
#endif

  ///////////////////////////////////////////////////////////////////////////////

bool RegisterList::writeCVBit(int cv, int bNum, int bValue, int callBack, int callBackSub) volatile 
{
	byte bWrite[4];
	bool ok = false;
	int ret, base;

	cv--;                              // actual CV addresses are cv-1 (0-1023)
	bValue = bValue % 2;
	bNum = bNum % 8;

	bWrite[0] = 0x78 + (highByte(cv) & 0x03);   // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
	bWrite[1] = lowByte(cv);
	bWrite[2] = 0xF0 + bValue * 8 + bNum;

	loadPacket(0, resetPacket, 2, 3);        // NMRA recommends starting with 3 reset packets
	loadPacket(0, bWrite, 3, 5);             // NMRA recommends 5 verify packets
	loadPacket(0, bWrite, 3, 6);             // NMRA recommends 6 write or reset packets for decoder recovery time

	// If monitor pin undefined, write cv without any confirmation...
	if (DCCppConfig::CurrentMonitorProg != UNDEFINED_PIN)
	{
		base = RegisterList::buildBaseAcknowlegde(DCCppConfig::CurrentMonitorProg);

		bitClear(bWrite[2], 4);              // change instruction code from Write Bit to Verify Bit

		loadPacket(0, resetPacket, 2, 3);          // NMRA recommends starting with 3 reset packets
		loadPacket(0, bWrite, 3, 5);               // NMRA recommends 5 verfy packets
		loadPacket(0, bWrite, 3, 6);           // NMRA recommends 6 write or reset packets for decoder recovery time

		ret = RegisterList::checkAcknowlegde(DCCppConfig::CurrentMonitorProg, base);

		loadPacket(0, resetPacket, 2, 1);      // Final reset packetcompleted (and decoder begins to respond)

		if (ret != 0)    // verify successful
			ok = true;
	}

#if defined(USE_TEXTCOMMAND)
	DCCPP_INTERFACE.print("<r");
	DCCPP_INTERFACE.print(callBack);
	DCCPP_INTERFACE.print("|");
	DCCPP_INTERFACE.print(callBackSub);
	DCCPP_INTERFACE.print("|");
	DCCPP_INTERFACE.print(cv + 1);
	DCCPP_INTERFACE.print(" ");
	DCCPP_INTERFACE.print(bNum);
	DCCPP_INTERFACE.print(" ");
	DCCPP_INTERFACE.print(bValue);
	DCCPP_INTERFACE.print(">");

	if (ok)
		DCCPP_INTERFACE.print("ok");
	else
		DCCPP_INTERFACE.print("fail");

	DCCPP_INTERFACE.sendNewline();
#endif

	return ok;
} // RegisterList::writeCVBit(ints)

#ifdef USE_TEXTCOMMAND
bool RegisterList::writeCVBit(const char *s) volatile
{
  int bNum, bValue, cv, callBack, callBackSub;

  if(sscanf(s,"%d %d %d %d %d",&cv,&bNum,&bValue,&callBack,&callBackSub) != 5)          // cv = 1-1024
  {
#ifdef DCCPP_DEBUG_MODE
	  Serial.println(F("W Syntax error"));
#endif
	  return false;
  }

  return this->writeCVBit(cv, bNum, bValue, callBack, callBackSub);
} // RegisterList::writeCVBit(string)
#endif

///////////////////////////////////////////////////////////////////////////////

void RegisterList::writeCVByteMain(int cab, int cv, int bValue) volatile 
{
	byte b[6];                      // save space for checksum byte
	byte nB = 0;

	cv--;

	if (cab>127)
		b[nB++] = highByte(cab) | 0xC0;      // convert train number into a two-byte address

	b[nB++] = lowByte(cab);
	b[nB++] = 0xEC + (highByte(cv) & 0x03);   // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
	b[nB++] = lowByte(cv);
	b[nB++] = bValue;

	loadPacket(0, b, nB, 4);

} // RegisterList::writeCVByteMain(ints)

#ifdef USE_TEXTCOMMAND
void RegisterList::writeCVByteMain(const char *s) volatile
{
	int cab;
	int cv;
	int bValue;

	if (sscanf(s, "%d %d %d", &cab, &cv, &bValue) != 3)
	{
#ifdef DCCPP_DEBUG_MODE
		Serial.println(F("w Syntax error"));
#endif
		return;
	}

	this->writeCVByteMain(cab, cv, bValue);
} // RegisterList::writeCVByteMain(string)
#endif

  ///////////////////////////////////////////////////////////////////////////////

void RegisterList::writeCVBitMain(int cab, int cv, int bNum, int bValue) volatile 
{
	byte b[6];                      // save space for checksum byte
	byte nB = 0;

	cv--;

	bValue = bValue % 2;
	bNum = bNum % 8;

	if (cab>127)
		b[nB++] = highByte(cab) | 0xC0;      // convert train number into a two-byte address

	b[nB++] = lowByte(cab);
	b[nB++] = 0xE8 + (highByte(cv) & 0x03);   // any CV>1023 will become modulus(1024) due to bit-mask of 0x03
	b[nB++] = lowByte(cv);
	b[nB++] = 0xF0 + bValue * 8 + bNum;

	loadPacket(0, b, nB, 4);

} // RegisterList::writeCVBitMain(ints)

#ifdef USE_TEXTCOMMAND
void RegisterList::writeCVBitMain(const char *s) volatile
{
	int cab;
	int cv;
	int bNum;
	int bValue;

	if (sscanf(s, "%d %d %d %d", &cab, &cv, &bNum, &bValue) != 4)
	{
#ifdef DCCPP_DEBUG_MODE
		Serial.println(F("w Syntax error"));
#endif
		return;
	}

	this->writeCVBitMain(cab, cv, bNum, bValue);
} // RegisterList::writeCVBitMain(string)
#endif

///////////////////////////////////////////////////////////////////////////////

#ifdef DCCPP_DEBUG_MODE
void RegisterList::printPacket(int nReg, byte *b, int nBytes, int nRepeat) volatile 
{  
	DCCPP_INTERFACE.print("<*");
	DCCPP_INTERFACE.print(nReg);
	DCCPP_INTERFACE.print(":");
  for(int i=0;i<nBytes;i++){
	  DCCPP_INTERFACE.print(" ");
	  DCCPP_INTERFACE.print(b[i],HEX);
  }
  DCCPP_INTERFACE.print(" / ");
  DCCPP_INTERFACE.print(nRepeat);
  DCCPP_INTERFACE.print(">");
	DCCPP_INTERFACE.sendNewline();
} // RegisterList::printPacket()
#endif

///////////////////////////////////////////////////////////////////////////////

byte RegisterList::idlePacket[3]={0xFF,0x00,0};                 // always leave extra byte for checksum computation
byte RegisterList::resetPacket[3]={0x00,0x00,0};

byte RegisterList::bitMask[]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};         // masks used in interrupt routine to speed the query of a single bit in a Packet
/*
 modifs DB mises à la fin pour ne pas changer les numéros de ligne
 line 400  Serial.println(cv+1); // +1 car cv-- juste avant !
 line 372 if (max > ACK_SAMPLE_THRESHOLD / 10) // 10 au lieu de 2
 line 372 if (max < ACK_SAMPLE_THRESHOLD)
*/
