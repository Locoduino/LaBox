/*
 * La Box Project
 * Parameters
 * @Author : Cedric Bellec
 * @Organization : Locoduino.org
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <arduino.h>

//************************ Global variables ****************************
//***********************************************************************


enum enumHMIState :byte {StateDashboard, StateParametersMenu, StateDashboardTrainView, StateBrowseEventLst, StateExitMenu, StateWifiWaiting, StateNoWifi } ;
enum enumEvent    :byte {noEvent, eventUp, eventDown, eventSel};
enum enumWifiMode :byte {wifiAp, wifiClient}; 

// variables must be global due to static methods
//enumEvent     _HMIEvent      = noEvent;

//************************** DEBUG LOG **********************************
//***********************************************************************
// Comment or uncomment #define if you want to see debugging information.
// Take care, to much informations is not good for the ESP32.
#define _DEBUG                    // Global debugging traces. <<< Log levels only work if _DEBUG is active >>>
#define _DEBUG_CRITICAL           // Critical error level
#define _DEBUG_ERROR              // Error level
//#define _DEBUG_FUNCTION           // Log begin and exit function
//#define _DEBUG_PARAMS             // Log parameters at function beginning 
//#define _DEBUG_LEVEL1             // Level 1 Log
//#define _DEBUG_LEVEL2             // Level 2 Log
//#define _DEBUG_LEVEL3             // Level 3 Log
//#define _DEBUG_SIMUL              // simulates physical measurements
//#define _DEBUG_STATE_MACHINE      // State Mahcine Log
//***********************************************************************

//*********************************************************************************
//------------------ Traces functions only if _DEBUG is active --------------------
//*********************************************************************************
#ifdef _DEBUG_FUNCTION
  #define _DEBUG_FCT_PRINTLN(x) Serial.println(x)
  #define _DEBUG_FCT_PRINTLN2(x,y) Serial.println(x,y)
  #define _DEBUG_FCT_PRINT(x) Serial.print(x)
  #define _DEBUG_FCT_PRINT2(x,y) Serial.print(x,y)
#else
  #define _DEBUG_FCT_PRINTLN(x)
  #define _DEBUG_FCT_PRINTLN2(x,y)
  #define _DEBUG_FCT_PRINT(x)
  #define _DEBUG_FCT_PRINT2(x,y)
#endif 

#ifdef _DEBUG_PARAMS
  #define _DEBUG_PARAMS_PRINTLN(x) Serial.println(x)
  #define _DEBUG_PARAMS_PRINTLN2(x,y) Serial.println(x,y)
  #define _DEBUG_PARAMS_PRINT(x) Serial.print(x)
  #define _DEBUG_PARAMS_PRINT2(x,y) Serial.print(x,y)
#else
  #define _DEBUG_PARAMS_PRINTLN(x)
  #define _DEBUG_PARAMS_PRINTLN2(x,y)
  #define _DEBUG_PARAMS_PRINT(x)
  #define _DEBUG_PARAMS_PRINT2(x,y)
#endif
  
#ifdef _DEBUG_LEVEL1
  #define _DEBUG_LEVEL1_PRINTLN(x) Serial.println(x)
  #define _DEBUG_LEVEL1_PRINTLN2(x,y) Serial.println(x,y)
  #define _DEBUG_LEVEL1_PRINT(x) Serial.print(x)
  #define _DEBUG_LEVEL1_PRINT2(x,y) Serial.print(x,y)
#else
  #define _DEBUG_LEVEL1_PRINTLN(x)
  #define _DEBUG_LEVEL1_PRINTLN2(x,y)
  #define _DEBUG_LEVEL1_PRINT(x)
  #define _DEBUG_LEVEL1_PRINT2(x,y)
#endif 

#ifdef _DEBUG_LEVEL2
  #define _DEBUG_LEVEL2_PRINTLN(x) Serial.println(x)
  #define _DEBUG_LEVEL2_PRINTLN2(x,y) Serial.println(x,y)
  #define _DEBUG_LEVEL2_PRINT(x) Serial.print(x)
  #define _DEBUG_LEVEL2_PRINT2(x,y) Serial.print(x,y)
#else
  #define _DEBUG_LEVEL2_PRINTLN(x)
  #define _DEBUG_LEVEL2_PRINTLN2(x,y)
  #define _DEBUG_LEVEL2_PRINT(x)
  #define _DEBUG_LEVEL2_PRINT2(x,y)
#endif 

#ifdef _DEBUG_LEVEL3
  #define _DEBUG_LEVEL3_PRINTLN(x) Serial.println(x)
  #define _DEBUG_LEVEL3_PRINTLN2(x,y) Serial.println(x,y)
  #define _DEBUG_LEVEL3_PRINT(x) Serial.print(x)
  #define _DEBUG_LEVEL3_PRINT2(x,y) Serial.print(x,y)
#else
  #define _DEBUG_LEVEL3_PRINTLN(x)
  #define _DEBUG_LEVEL3_PRINTLN2(x,y)
  #define _DEBUG_LEVEL3_PRINT(x)
  #define _DEBUG_LEVEL3_PRINT2(x,y)
#endif
// simulates physical measurements
#ifdef _DEBUG_SIMUL
  #define _DEBUG_SIMUL_PRINTLN(x) Serial.println(x)
  #define _DEBUG_SIMUL_PRINTLN2(x,y) Serial.println(x,y)
  #define _DEBUG_SIMUL_PRINT(x) Serial.print(x)
  #define _DEBUG_SIMUL_PRINT2(x,y) Serial.print(x,y)
#else
  #define _DEBUG_SIMUL_PRINTLN(x)
  #define _DEBUG_SIMUL_PRINTLN2(x,y)
  #define _DEBUG_SIMUL_PRINT(x)
  #define _DEBUG_SIMUL_PRINT2(x,y)
#endif
// simulates physical measurements
#ifdef _DEBUG_STATE_MACHINE
  #define _DEBUG_SM_PRINTLN(x) Serial.println(x)
  #define _DEBUG_SM_PRINTLN2(x,y) Serial.println(x,y)
  #define _DEBUG_SM_PRINT(x) Serial.print(x)
  #define _DEBUG_SM_PRINT2(x,y) Serial.print(x,y)
#else
  #define _DEBUG_SM_PRINTLN(x)
  #define _DEBUG_SM_PRINTLN2(x,y)
  #define _DEBUG_SM_PRINT(x)
  #define _DEBUG_SM_PRINT2(x,y)
#endif

#ifdef _DEBUG_CRITICAL
  #define _DEBUG_CRITICAL_PRINTLN(x) Serial.println(x)
  #define _DEBUG_CRITICAL_PRINTLN2(x,y) Serial.println(x,y)
  #define _DEBUG_CRITICAL_PRINT(x) Serial.print(x)
  #define _DEBUG_CRITICAL_PRINT2(x,y) Serial.print(x,y)
#else
  #define _DEBUG_CRITICAL_PRINTLN(x)
  #define _DEBUG_CRITICAL_PRINTLN2(x,y)
  #define _DEBUG_CRITICAL_PRINT(x)
  #define _DEBUG_CRITICAL_PRINT2(x,y)
#endif

#ifdef _DEBUG_ERROR
  #define _DEBUG_ERROR_PRINTLN(x) Serial.println(x)
  #define _DEBUG_ERROR_PRINTLN2(x,y) Serial.println(x,y)
  #define _DEBUG_ERROR_PRINT(x) Serial.print(x)
  #define _DEBUG_ERROR_PRINT2(x,y) Serial.print(x,y)
#else
  #define _DEBUG_ERROR_PRINTLN(x)
  #define _DEBUG_ERROR_PRINTLN2(x,y)
  #define _DEBUG_ERROR_PRINT(x)
  #define _DEBUG_ERROR_PRINT2(x,y)
#endif
#endif
