/**********************************************************************

PacketRegister.h
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#ifndef PacketRegister_h
#define PacketRegister_h

#include "Arduino.h"

// Define constants used for reading CVs from the Programming Track

#define  ACK_BASE_COUNT            100      /**< Number of analogRead samples to take before each CV verify to establish a baseline current.*/
#define  ACK_SAMPLE_COUNT          500      /**< Number of analogRead samples to take when monitoring current after a CV verify (bit or byte) has been sent.*/ 
#define  ACK_SAMPLE_SMOOTHING      0.2      /**< Exponential smoothing to use in processing the analogRead samples after a CV verify (bit or byte) has been sent.*/
#define  ACK_SAMPLE_THRESHOLD      DCCpp::ackThreshold		/**< The threshold that the exponentially-smoothed analogRead samples (after subtracting the baseline current) must cross to establish ACKNOWLEDGEMENT.*/

struct Packet{
  byte buf[10];
  byte nBits;
}; // Packet

struct Register{
  Packet packet[2];
  Packet *activePacket;
  Packet *updatePacket;
  void initPackets();
}; // Register
  
/** Define a series of registers that can be sequentially accessed over a loop to generate a repeating series of DCC Packets.
 */
struct RegisterList{
  int maxNumRegs;
  Register *reg;
  Register **regMap;
  Register *currentReg;
  Register *maxLoadedReg;
  Register *nextReg;
  Packet  *tempPacket;
  
#ifdef USE_ONLY1_INTERRUPT
  /* how many 58us periods needed for half-cycle (1 for "1", 2 for "0") */
  byte timerPeriods;
  /* how many 58us periods are left (at start, 2 for "1", 4 for "0"). */
  byte timerPeriodsLeft;
#endif

  byte currentBit;
  byte nRepeat;
  int *speedTable;
  static byte idlePacket[];
  static byte resetPacket[];
  static byte bitMask[];
  RegisterList(int);
  void loadPacket(int, byte *, int, int, int=0) volatile;

#ifdef USE_TEXTCOMMAND
  void setThrottle(const char *) volatile;
  void setFunction(const char *) volatile;
  void setAccessory(const char *) volatile;
  void writeTextPacket(const char *) volatile;
#endif

  int readCVraw(int cv, int callBack, int callBackSub) volatile;
	int buildBaseAcknowlegde(int inMonitorPin) volatile;
	int checkAcknowlegde(int inMonitorPin,  int inBase) volatile;

#ifdef USE_TEXTCOMMAND
  int readCV(const char *) volatile;
  bool writeCVByte(const char *) volatile;
  bool writeCVBit(const char *) volatile;

  int readCVmain(const char *) volatile;
  void writeCVByteMain(const char *) volatile;
  void writeCVBitMain(const char *s) volatile;
#endif

	byte setAckThreshold(byte inNewValue);

  void setThrottle(int nReg, int cab, int tSpeed, int tDirection) volatile;
  void setFunction(int nReg, int cab, int fByte, int eByte, bool returnMessages = true) volatile;
  void setAccessory(int aAdd, int aNum, int activate) volatile;
  void writeTextPacket(int nReg, byte *b, int nBytes) volatile;
  int readCV(int cv, int callBack, int callBackSub) volatile;
  int readCVmain(int cv, int callBack, int callBackSub) volatile;
  bool writeCVByte(int cv, int bValue, int callBack, int callBackSub) volatile;	// prog track
  bool writeCVBit(int cv, int bNum, int bValue, int callBack, int callBackSub) volatile;	// prog track
  void writeCVByteMain(int cab, int cv, int bvalue) volatile;
  void writeCVBitMain(int cab, int cv, int bNum, int bValue) volatile;
  bool isRegisterEmpty() volatile { return this->nextReg == NULL; }

#ifdef DCCPP_DEBUG_MODE
  void printPacket(int, byte *, int, int) volatile;
#endif
};

#endif
