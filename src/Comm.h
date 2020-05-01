/**********************************************************************

Comm.h
COPYRIGHT (c) 2013-2016 Gregg E. Berman

Part of DCC++ BASE STATION for the Arduino

**********************************************************************/

#include "Config.h"

#if defined(USE_ETHERNET) || defined(USE_WIFI)
enum EthernetProtocol
{
	HTTP,
	TCP,
	UDP
};
#endif

// define DCCPP_INTERFACE as serial in all cases to be able at least to print diagnostic messages on console
//#define DCCPP_INTERFACE Serial
