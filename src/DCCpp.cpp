/*************************************************************
  project: <DCCpp library>
  author: <Thierry PARIS>
  description: <DCCpp class>
*************************************************************/

#include "Arduino.h"
#include "DCCpp.h"

// NEXT DECLARE GLOBAL OBJECTS TO PROCESS AND STORE DCC PACKETS AND MONITOR TRACK CURRENTS.
// NOTE REGISTER LISTS MUST BE DECLARED WITH "VOLATILE" QUALIFIER TO ENSURE THEY ARE PROPERLY UPDATED BY INTERRUPT ROUTINES

volatile RegisterList DCCpp::mainRegs(MAX_MAIN_REGISTERS);    // create list of registers for MAX_MAIN_REGISTER Main Track Packets
volatile RegisterList DCCpp::progRegs(3);                     // create a shorter list of only two registers for Program Track Packets

CurrentMonitor DCCpp::mainMonitor;  // create monitor for current on Main Track
CurrentMonitor DCCpp::progMonitor;  // create monitor for current on Program Track

bool DCCpp::programMode;
bool DCCpp::panicStopped;
byte DCCpp::ackThreshold = 0;
bool DCCpp::IsPowerOnMain = false;
bool DCCpp::IsPowerOnProg = false;
bool DCCpp::powerOnAtFirstClient = true;

#if defined(ARDUINO_ARCH_ESP32) && defined(USE_TEXTCOMMAND)
TaskHandle_t Task1;
#endif

// *********************************************************** DCCpp class

static bool first = true;

///////////////////////////////////////////////////////////////////////////////
// MAIN ARDUINO LOOP
///////////////////////////////////////////////////////////////////////////////

void DCCpp::loop()
{
#ifdef USE_THROTTLES
  Throttle* pCurr = Throttles::getFirst();
  bool done = false;

  while (pCurr != NULL)
  {
    TextCommand::pCurrentThrottle = pCurr;
    done = pCurr->processBuffer();
    pCurr = pCurr->pNextThrottle;
  }

  TextCommand::pCurrentThrottle = NULL;
#endif

#ifdef USE_TEXTCOMMAND
  TextCommand::process();              // check for, and process, and new serial commands
#endif

  if (first)
  {
    first = false;
#if defined(DCCPP_DEBUG_MODE) && defined(DCCPP_PRINT_DCCPP)
    showConfiguration();
#endif
  }

  if (CurrentMonitor::checkTime())
  { // if sufficient time has elapsed since last update, check current draw on Main and Program Tracks
    mainMonitor.check();
    progMonitor.check();
  }

#ifdef USE_SENSOR
  Sensor::check();    // check sensors for activated or not
#endif
}

#ifndef USE_ONLY1_INTERRUPT
void DCCpp::beginMain(uint8_t inOptionalDirectionMotor, uint8_t inSignalPin, uint8_t inSignalEnable, uint8_t inCurrentMonitor)
{
  DCCppConfig::DirectionMotorA = inOptionalDirectionMotor;
#else
void DCCpp::beginMain(uint8_t inSignalPin, uint8_t inSignalEnable, uint8_t inCurrentMonitor)
{
#endif

#ifdef DCCPP_DEBUG_MODE
  //	CheckPowerConnectionsWithLeds(inSignalPin, 1000);
#endif

  DCCppConfig::SignalEnablePinMain = inSignalEnable;	// PWM
  DCCppConfig::CurrentMonitorMain = inCurrentMonitor;

  // If no main line, exit.
  if (inSignalPin == UNDEFINED_PIN)
  {
#ifdef DCCPP_DEBUG_MODE
    Serial.println("No main track");
#endif
    return;
  }

  mainMonitor.begin(DCCppConfig::CurrentMonitorMain, DCCppConfig::SignalEnablePinMain, (char *) "<p2>");

  DCCpp::beginMainDccSignal(inSignalPin);

  if (DCCppConfig::SignalEnablePinMain != UNDEFINED_PIN)
    digitalWrite(DCCppConfig::SignalEnablePinMain, LOW);

#ifdef DCCPP_DEBUG_MODE
  Serial.print(F("beginMain achivied with pin "));
  Serial.println(inSignalPin);
#endif
}

#ifndef USE_ONLY1_INTERRUPT
void DCCpp::beginProg(uint8_t inOptionalDirectionMotor, uint8_t inSignalPin, uint8_t inSignalEnable, uint8_t inCurrentMonitor)
{
  DCCppConfig::DirectionMotorB = inOptionalDirectionMotor;
#else
void DCCpp::beginProg(uint8_t inSignalPin, uint8_t inSignalEnable, uint8_t inCurrentMonitor)
{
#endif
  DCCppConfig::SignalEnablePinProg = inSignalEnable;
  DCCppConfig::CurrentMonitorProg = inCurrentMonitor;

  // If no programming line, exit.
  if (inSignalPin == UNDEFINED_PIN)
  {
#ifdef DCCPP_DEBUG_MODE
    Serial.println("No prog track");
#endif
    return;
  }

  progMonitor.begin(DCCppConfig::CurrentMonitorProg, DCCppConfig::SignalEnablePinProg, (char *) "<p3>");

  DCCpp::beginProgDccSignal(inSignalPin);

#ifdef DCCPP_DEBUG_MODE
  Serial.print(F("beginProg achivied with pin "));
  Serial.println(inSignalPin);
#endif
}

#if defined(ARDUINO_ARCH_ESP32) && defined(USE_TEXTCOMMAND)
//Task1code
void Task1code(void* pvParameters)
{
  while (true)
  {
     TextCommand::receiveCommands();
  }
}
#endif

void DCCpp::begin()
{
  DCCpp::programMode = false;
  DCCpp::panicStopped = false;
  DCCpp::ackThreshold = 30;

  DCCppConfig::SignalEnablePinMain = UNDEFINED_PIN;
  DCCppConfig::CurrentMonitorMain = UNDEFINED_PIN;

  DCCppConfig::SignalEnablePinProg = UNDEFINED_PIN;
  DCCppConfig::CurrentMonitorProg = UNDEFINED_PIN;

#ifndef USE_ONLY1_INTERRUPT
  DCCppConfig::DirectionMotorA = UNDEFINED_PIN;
  DCCppConfig::DirectionMotorB = UNDEFINED_PIN;
#else
  DCCppConfig::SignalPortMaskMain = 0;
  DCCppConfig::SignalPortMaskProg = 0;
#endif

  mainMonitor.begin(UNDEFINED_PIN, UNDEFINED_PIN, "");
  progMonitor.begin(UNDEFINED_PIN, UNDEFINED_PIN, "");

#ifdef SDCARD_CS
  pinMode(SDCARD_CS, OUTPUT);
  digitalWrite(SDCARD_CS, HIGH);     // De-select the SD card
#endif

#ifdef USE_EEPROM
  EEStore::init();                                          // initialize and load Turnout and Sensor definitions stored in EEPROM
  if (EEStore::needsRefreshing())
    EEStore::store();
#endif

#if defined(ARDUINO_ARCH_ESP32)
  MessageStack::MessagesStack.begin(true);
#else
  MessageStack::MessagesStack.begin(false);
#endif

/*#ifdef USE_THROTTLES
  Throttle* pCurr = Throttles::getFirst();

  while (pCurr != NULL)
  {
    pCurr->begin();
    pCurr = pCurr->pNextThrottle;
  }
#endif*/

#if defined(ARDUINO_ARCH_ESP32) && defined(USE_TEXTCOMMAND)
  
  int core = xPortGetCoreID();

  // Remove WatchDog on Core 0 to avoid continual reboot...
  if (core == 0)
  {
      Throttles::executionCore = 1;
      disableCore1WDT();
  }
  else
  {
      Throttles::executionCore = 0;
      disableCore0WDT();
  }

#ifdef DCCPP_DEBUG_MODE
  Serial.print("Ino executing on core ");
  Serial.println(core);
  Serial.print("Throttles command receivers executing on core ");
  Serial.println(Throttles::executionCore);
#endif

  //create a task that will be executed in the Task1code() function, with priority 1 and executed on core 0
  xTaskCreatePinnedToCore(
    Task1code,   // Task function.
    "ReceiveCommands",     // name of task.
    10000,       // Stack size of task
    NULL,        // parameter of the task
    1,           // priority of the task
    &Task1,      // Task handle to keep track of created task
    Throttles::executionCore);    // pin task to the other available core.
#endif

#ifdef DCCPP_DEBUG_MODE
  //pinMode(LED_BUILTIN, OUTPUT);
  Serial.println(F("begin achieved"));
#endif
} // begin

#ifdef DCCPP_PRINT_DCCPP
///////////////////////////////////////////////////////////////////////////////
// PRINT CONFIGURATION INFO TO SERIAL PORT REGARDLESS OF INTERFACE TYPE
// - ACTIVATED ON STARTUP IF SHOW_CONFIG_PIN IS TIED HIGH

void DCCpp::showConfiguration()
{
  Serial.println(F("*** DCCpp LIBRARY ***"));

  Serial.print(F("VERSION DCC++:      "));
  Serial.println(VERSION);
  Serial.println(F(DCCPP_LIBRARY_VERSION));
  Serial.print(F("COMPILED:     "));
  Serial.print(__DATE__);
  Serial.print(F(" "));
  Serial.println(__TIME__);

  //Serial.print(F("\nARDUINO:      "));
  //Serial.print(ARDUINO_TYPE);

  //Serial.print(F("\n\nMOTOR SHIELD: "));
  //Serial.print(MOTOR_SHIELD_NAME);

#ifdef ARDUINO_ARCH_AVR
  Serial.print(F("\n\nDCC SIG MAIN(DIR): "));
  Serial.println(DCC_SIGNAL_PIN_MAIN);
#endif

#ifndef USE_ONLY1_INTERRUPT
  if (DCCppConfig::DirectionMotorA != UNDEFINED_PIN)
  {
    Serial.print(F("   DIRECTION: "));
    Serial.println(DCCppConfig::DirectionMotorA);
  }
#endif

  if (DCCppConfig::SignalEnablePinMain != UNDEFINED_PIN)
  {
    Serial.print(F("   ENABLE(PWM): "));
    Serial.println(DCCppConfig::SignalEnablePinMain);
  }

  if (DCCppConfig::CurrentMonitorMain != UNDEFINED_PIN)
  {
    Serial.print(F("   CURRENT: "));
    Serial.println(DCCppConfig::CurrentMonitorMain);
  }

#ifdef ARDUINO_ARCH_AVR
  Serial.print(F("\n\nDCC SIG PROG(DIR): "));
  Serial.println(DCC_SIGNAL_PIN_PROG);
#endif

#ifndef USE_ONLY1_INTERRUPT
  if (DCCppConfig::DirectionMotorB != UNDEFINED_PIN)
  {
    Serial.print(F("   DIRECTION: "));
    Serial.println(DCCppConfig::DirectionMotorB);
  }
#endif
  if (DCCppConfig::SignalEnablePinProg != UNDEFINED_PIN)
  {
    Serial.print(F("   ENABLE(PWM): "));
    Serial.println(DCCppConfig::SignalEnablePinProg);
  }
  if (DCCppConfig::CurrentMonitorProg != UNDEFINED_PIN)
  {
    Serial.print(F("   CURRENT: "));
    Serial.println(DCCppConfig::CurrentMonitorProg);
  }

#if defined(USE_EEPROM)
#if defined(USE_TURNOUT)
  Serial.print(F("\n\nNUM TURNOUTS: "));
  Serial.println(EEStore::data.nTurnouts);
#endif
#if defined(USE_SENSOR)
  Serial.print(F("     SENSORS: "));
  Serial.println(EEStore::data.nSensors);
#endif
#if defined(USE_OUTPUT)
  Serial.print(F("     OUTPUTS: "));
  Serial.println(EEStore::data.nOutputs);
#endif
#endif


#ifdef USE_TEXTCOMMAND
#ifdef USE_THROTTLES
  Throttles::printThrottles();
#endif
#endif
  //	Serial.print(F("\n\nPROGRAM HALTED - PLEASE RESTART ARDUINO"));

  //	while (true);
  //		Serial.println("");
}
#endif

void DCCpp::panicStop(bool inStop)
{
  panicStopped = inStop;

#ifdef DCCPP_DEBUG_MODE
  Serial.print(F("DCCpp PanicStop "));
  Serial.println(inStop ? F("pressed") : F("canceled"));
#endif

  /* activate or not the power on rails */

  if (inStop)
    powerOff();
  else
    powerOn();
  if (hmi::CurrentInterface != NULL)
    hmi::CurrentInterface->DCCEmergencyStop();
}

void DCCpp::powerOn(bool inMain, bool inProg)
{
#ifdef DCCPP_DEBUG_MODE
  Serial.println(F("DCCpp PowerOn"));
#endif

  bool done = false;
  if (inProg && DCCppConfig::SignalEnablePinProg != UNDEFINED_PIN)
  {
    digitalWrite(DCCppConfig::SignalEnablePinProg, HIGH);
    done = true;
    IsPowerOnMain = true;
  }

  if (inMain && DCCppConfig::SignalEnablePinMain != UNDEFINED_PIN)
  {
    digitalWrite(DCCppConfig::SignalEnablePinMain, HIGH);
    done = true;
    IsPowerOnProg = true;
  }

  if (done)
  {
    if (hmi::CurrentInterface != NULL)
    {
      hmi::CurrentInterface->DCCOn();
    }
    DCCPP_INTERFACE.print("<p1>");
#ifdef USE_THROTTLES
    if (DCCPP_INTERFACE.sendNewline())
#endif
      DCCPP_INTERFACE.println("");
  }

  DCCpp::powerOnAtFirstClient = false;
}

void DCCpp::powerOff(bool inMain, bool inProg)
{
#ifdef DCCPP_DEBUG_MODE
  Serial.println(F("DCCpp PowerOff"));
#endif

  bool done = false;
  if (inProg && DCCppConfig::SignalEnablePinProg != UNDEFINED_PIN)
  {
    digitalWrite(DCCppConfig::SignalEnablePinProg, LOW);
    done = true;
    IsPowerOnMain = false;
  }
  if (inMain && DCCppConfig::SignalEnablePinMain != UNDEFINED_PIN)
  {
    digitalWrite(DCCppConfig::SignalEnablePinMain, LOW);
    done = true;
    IsPowerOnProg = false;
  }

  if (done)
  {
    if (hmi::CurrentInterface != NULL)
      hmi::CurrentInterface->DCCOff();
    DCCPP_INTERFACE.print("<p0>");
#ifdef USE_THROTTLES
    if (DCCPP_INTERFACE.sendNewline())
#endif
      DCCPP_INTERFACE.println("");
  }
}

byte DCCpp::setAckThreshold(byte inNewValue)
{
  byte old = DCCpp::ackThreshold;
  DCCpp::ackThreshold = inNewValue;
  return old;
}

/***************************** Driving functions */

bool DCCpp::setThrottle(volatile RegisterList *inpRegs, int nReg,  int inLocoId, int inStepsNumber, int inNewSpeed, bool inForward)
{
  int val = 0;

  if (panicStopped)
    val = 1;
  else if (inNewSpeed > 1)
    val = map(inNewSpeed, 2, inStepsNumber, 2, 126);

#ifdef DCCPP_DEBUG_MODE
  Serial.print(F("DCCpp SetSpeed for loco "));
  Serial.print(inLocoId);
  Serial.print(F(" : "));
  Serial.print(inForward ? inNewSpeed : -inNewSpeed);
  Serial.print(F("/"));
  Serial.print(inStepsNumber);
//  Serial.print(F(" (in Dcc "));
//  Serial.print(val);
  Serial.println(F(" )"));
#endif

  if (hmi::CurrentInterface != NULL)
  {
    hmi::CurrentInterface->ChangeDirection(inLocoId, inForward);
    hmi::CurrentInterface->ChangeSpeed(inLocoId, inNewSpeed);
  }
  inpRegs->setThrottle(nReg, inLocoId, val, inForward);

  return true;
}

#ifdef USE_LOCOMOTIVES
void DCCpp::setFunctions(volatile RegisterList *inpRegs, int nReg, int inLocoId, FunctionsState &inStates)
{
#ifdef DCCPP_DEBUG_MODE
  if (inpRegs == &mainRegs)
  {
    if (nReg > MAX_MAIN_REGISTERS)
      Serial.println(F("Invalid register number on main track."));
  }
  else
  {
    if (nReg > MAX_PROG_REGISTERS)
      Serial.println(F("Invalid register number on programming track."));
  }
#endif
  byte flags = 0;

  byte oneByte1 = 128;	// Group one functions F0-F4
  byte twoByte1 = 176;	// Group two F5-F8
  byte threeByte1 = 160;	// Group three F9-F12
  byte fourByte2 = 0;		// Group four F13-F20
  byte fiveByte2 = 0;		// Group five F21-F28

  for (byte func = 0; func <= 28; func++)
  {
    if (inStates.isActivationChanged(func))
    {
      if (hmi::CurrentInterface != NULL)
      {
        hmi::CurrentInterface->ChangeFunction(inLocoId, func, inStates.isActivated(func));
      }
    }

    if (func <= 4)
    {
      /*
      	To set functions F0 - F4 on(= 1) or off(= 0) :

           BYTE1 : 128 + F1 * 1 + F2 * 2 + F3 * 4 + F4 * 8 + F0 * 16
        BYTE2 : omitted
      */

      if (inStates.isActivationChanged(func))
        flags |= 1;
      if (inStates.isActivated(func))
      {
        if (func == 0)
          oneByte1 += 16;
        else
          oneByte1 += (1 << (func - 1));
      }
    }
    else if (func <= 8)
    {
      /*
      	To set functions F5 - F8 on(= 1) or off(= 0) :

           BYTE1 : 176 + F5 * 1 + F6 * 2 + F7 * 4 + F8 * 8
        BYTE2 : omitted
      */

      if (inStates.isActivationChanged(func))
        flags |= 2;
      if (inStates.isActivated(func))
        twoByte1 += (1 << (func - 5));
    }
    else if (func <= 12)
    {
      /*
           To set functions F9 - F12 on(= 1) or off(= 0) :

           BYTE1 : 160 + F9 * 1 + F10 * 2 + F11 * 4 + F12 * 8
        BYTE2 : omitted
      */

      if (inStates.isActivationChanged(func))
        flags |= 4;
      if (inStates.isActivated(func))
        threeByte1 += (1 << (func - 9));
    }
    else if (func <= 20)
    {
      /*
           To set functions F13 - F20 on(= 1) or off(= 0) :

           BYTE1 : 222
        BYTE2 : F13 * 1 + F14 * 2 + F15 * 4 + F16 * 8 + F17 * 16 + F18 * 32 + F19 * 64 + F20 * 128
      */

      if (inStates.isActivationChanged(func))
        flags |= 8;
      if (inStates.isActivated(func))
        fourByte2 += (1 << (func - 13));
    }
    else if (func <= 28)
    {
      /*
           To set functions F21 - F28 on(= 1) of off(= 0) :

           BYTE1 : 223
        BYTE2 : F21 * 1 + F22 * 2 + F23 * 4 + F24 * 8 + F25 * 16 + F26 * 32 + F27 * 64 + F28 * 128
      */

      if (inStates.isActivationChanged(func))
        flags |= 16;
      if (inStates.isActivated(func))
        fiveByte2 += (1 << (func - 21));
    }
  }

  int b1, b2 = -1;

  if (flags & 1)
    b1 = oneByte1;
  if (flags & 2)
    b1 = twoByte1;
  if (flags & 4)
    b1 = threeByte1;
  if (flags & 8)
  {
    b1 = 222;
    b2 = fourByte2;
  }
  if (flags & 16)
  {
    b1 = 223;
    b2 = fiveByte2;
  }

  inpRegs->setFunction(nReg, inLocoId, b1, b2);

  inStates.statesSent();

#ifdef DCCPP_DEBUG_MODE
  Serial.print(F("DCCpp SetFunctions for loco "));
  Serial.print(inLocoId);
  Serial.print(" / Activated : ");
  inStates.printActivated();
#endif
}
#endif

int DCCpp::identifyLocoId(volatile RegisterList *inReg)
{
  int  id = -1;
  int temp;
  temp = inReg->readCVraw(29, 100, 200);
  if ((temp != -1) && (bitRead(temp, 5))) {
    // long address : get CV#17 and CV#18
    id = inReg->readCVraw(18, 100, 200);
    if (id != -1) {
      temp = inReg->readCVraw(17, 100, 200);
      if (temp != -1) {
        id = id + ((temp - 192) << 8);
      }
    }
  }
  else {
    // short address: read only CV#1
    id = inReg->readCVraw(1, 100, 200);
  }

  return (id);
}

int DCCpp::readCvMain(int inCvId, int callBack, int callBackSub)
{
  Serial.println("main");
  int cvValue;
  cvValue = mainRegs.readCVmain(inCvId, callBack, callBackSub);
  if (hmi::CurrentInterface != NULL)
    hmi::CurrentInterface->ReadCV(true, inCvId, cvValue, cvValue != -1);

  return cvValue;
}

bool DCCpp::writeCvMain(int inCvId, byte cvValue, int callBack, int callBackSub)
{ 
  bool ret = writeCv(&(mainRegs), inCvId, cvValue, callBack, callBackSub);
  if (hmi::CurrentInterface != NULL)
    hmi::CurrentInterface->WriteCV(true, inCvId, cvValue, ret);
  return ret;
}

int DCCpp::readCvProg(int inCvId, int callBack, int callBackSub) 
{ 
  Serial.println("prog");
  int cvValue;
  cvValue = progRegs.readCV(inCvId, callBack, callBackSub);
  if (hmi::CurrentInterface != NULL)
    hmi::CurrentInterface->ReadCV(false, inCvId, cvValue, cvValue != -1);

  return cvValue;
}

bool DCCpp::writeCvProg(int inCvId, byte cvValue, int callBack, int callBackSub) 
{ 
  bool ret = writeCv(&(progRegs), inCvId, cvValue, callBack, callBackSub);
  if (hmi::CurrentInterface != NULL)
    hmi::CurrentInterface->WriteCV(false, inCvId, cvValue, ret);
  return ret;
}

bool DCCpp::writeCv(volatile RegisterList *inReg, int inCv, byte inValue, int callBack, int callBackSub)
{
#ifdef DCCPP_DEBUG_MODE
  Serial.print(F("DCCpp WriteCv "));
  Serial.print(inCv);
  Serial.print(F(" : "));
  Serial.println(inValue);
#endif

  return inReg->writeCVByte(inCv, inValue, callBack, callBackSub);
}

void DCCpp::setAccessory(int inAddress, byte inSubAddress, byte inActivate)
{
#ifdef DCCPP_DEBUG_MODE
  Serial.print(F("DCCpp AccessoryOperation "));
  Serial.print(inAddress);
  Serial.print(F(" / "));
  Serial.print(inSubAddress);
  Serial.print(F(" : "));
  Serial.println(inActivate);
#endif

  mainRegs.setAccessory(inAddress, inSubAddress, inActivate);
}

#ifdef DCCPP_DEBUG_MODE
void DCCpp::CheckPowerConnectionsWithLeds(uint8_t aDirPin, unsigned int inDelay)
{
  if (DCCppConfig::SignalEnablePinMain != 255 || DCCppConfig::SignalEnablePinProg != 255)
  {
    Serial.print(F("DCC signal is started, this function cannot operates."));
    return;
  }

  pinMode(aDirPin, OUTPUT);
  digitalWrite(aDirPin, HIGH);
  delay(inDelay);

  digitalWrite(aDirPin, LOW);
  delay(inDelay);
}
#endif
