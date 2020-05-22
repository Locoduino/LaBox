//-------------------------------------------------------------------
#ifndef __MessageConverterZ21_hpp__
#define __MessageConverterZ21_hpp__
//-------------------------------------------------------------------

#include "DCCpp.h"

#if defined(USE_THROTTLES)

#define Z21_UDPPORT		21105
#define Z21_TIMEOUT		60000		// if no activity during 1 minute, disconnect the throttle...

#define HEADER_LAN_GET_SERIAL_NUMBER 0x10
#define HEADER_LAN_LOGOFF 0x30
#define HEADER_LAN_XPRESS_NET 0x40
#define HEADER_LAN_SET_BROADCASTFLAGS 0x50
#define HEADER_LAN_GET_BROADCASTFLAGS 0x51
#define HEADER_LAN_SYSTEMSTATE_GETDATA 0x85    //0x141 0x21 0x24 0x05
#define HEADER_LAN_GET_HWINFO 0x1A
#define HEADER_LAN_GET_LOCOMODE 0x60
#define HEADER_LAN_SET_LOCOMODE 0x61
#define HEADER_LAN_GET_TURNOUTMODE 0x70
#define HEADER_LAN_SET_TURNOUTMODE 0x71
#define HEADER_LAN_RMBUS_DATACHANGED 0x80
#define HEADER_LAN_RMBUS_GETDATA 0x81
#define HEADER_LAN_RMBUS_PROGRAMMODULE 0x82
#define HEADER_LAN_RAILCOM_DATACHANGED 0x88
#define HEADER_LAN_RAILCOM_GETDATA 0x89
#define HEADER_LAN_LOCONET_DISPATCH_ADDR 0xA3
#define HEADER_LAN_LOCONET_DETECTOR 0xA4

#define LAN_GET_CONFIG 0x12

#define LAN_X_HEADER_GENERAL 0x21
#define LAN_X_HEADER_SET_STOP 0x80
#define LAN_X_HEADER_GET_FIRMWARE_VERSION 0xF1  //0x141 0x21 0x21 0x00 
#define LAN_X_HEADER_GET_LOCO_INFO 0xE3
#define LAN_X_HEADER_SET_LOCO 0xE4
#define LAN_X_HEADER_GET_TURNOUT_INFO 0x43
#define LAN_X_HEADER_SET_TURNOUT 0x53
#define LAN_X_HEADER_CV_READ 0x23
#define LAN_X_HEADER_CV_WRITE 0x24
#define LAN_X_HEADER_CV_POM 0xE6

#define LAN_X_DB0_GET_VERSION 0x21
#define LAN_X_DB0_GET_STATUS 0x24
#define LAN_X_DB0_SET_TRACK_POWER_OFF 0x80
#define LAN_X_DB0_SET_TRACK_POWER_ON 0x81
#define LAN_X_DB0_LOCO_DCC14	0x10
#define LAN_X_DB0_LOCO_DCC28	0x12
#define LAN_X_DB0_LOCO_DCC128	0x13
#define LAN_X_DB0_SET_LOCO_FUNCTION 0xF8
#define LAN_X_DB0_CV_POM_WRITE 0x30
#define LAN_X_DB0_CV_POM_ACCESSORY_WRITE 0x31

#define LAN_X_OPTION_CV_POM_WRITE_BYTE 0xEC
#define LAN_X_OPTION_CV_POM_WRITE_BIT 0xE8
#define LAN_X_OPTION_CV_POM_READ_BYTE 0xE4

// Replies to the controllers
#define HEADER_LAN_SYSTEMSTATE	0x84

#define LAN_X_HEADER_LOCO_INFO	0xEF
#define LAN_X_HEADER_TURNOUT_INFO 0x43
#define LAN_X_HEADER_FIRMWARE_VERSION 0xF3
#define LAN_X_HEADER_CV_NACK 0x61
#define LAN_X_HEADER_CV_RESULT 0x64

#define LAN_X_DB0_CV_NACK_SC 0x12
#define LAN_X_DB0_CV_NACK 0x13

// Broadcast flags

#define BROADCAST_BASE							0x00000001
#define BROADCAST_RBUS							0x00000002
#define BROADCAST_RAILCOM						0x00000004
#define BROADCAST_SYSTEM						0x00000100
#define BROADCAST_BASE_LOCOINFO			0x00010000
#define BROADCAST_LOCONET						0x01000000
#define BROADCAST_LOCONET_LOCO			0x02000000
#define BROADCAST_LOCONET_SWITCH		0x04000000
#define BROADCAST_LOCONET_DETECTOR	0x08000000
#define BROADCAST_RAILCOM_AUTO			0x00040000
#define BROADCAST_CAN								0x00080000

/** This is a class to handle decoder functions.
An instance of this class handle the status of the functions of one decoder.
A function can be active or not.
*/
class MessageConverterZ21 : public MessageConverter
{
private:
	int32_t broadcastFlags;

public:
	/** Creates a new instance for only one register.
	@param inName	throttle new name.
	*/
	MessageConverterZ21();

	void clientStart(Throttle* inpThrottle);
	void clientStop(Throttle* inpThrottle);
	bool processBuffer(Throttle* inpThrottle);
	void stayAlive(Throttle* inpThrottle);
	EthernetProtocol getProtocol() { return UDP; }

	void setSpeed(Throttle* inpThrottle, byte inNbSteps, byte inDB1, byte inDB2, byte inDB3);
	void setFunction(Throttle* inpThrottle, byte inDB1, byte inDB2, byte inDB3);
	void cvReadProg(Throttle* inpThrottle, byte inDB1, byte inDB2);
	void cvWriteProg(Throttle* inpThrottle, byte inDB1, byte inDB2, byte inDB3);

#ifdef DCCPP_DEBUG_MODE
	/** Print the status of the Converter.
	@remark Only available if DCCPP_DEBUG_MODE is defined.
	*/
	void printConverter();
#endif
};

#endif
#endif