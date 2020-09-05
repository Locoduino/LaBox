#include "hmi.h"
#include "hmiConfig.h"
#include "HmiInterface.hpp"
#include "Labox.h"

void hmi::HmiInterfaceLoop()
{
  _HMIDEBUG_FCT_PRINTLN("hmi::HmiInterfaceLoop().. Begin");

  int core = xPortGetCoreID();
  if (core != this->executionCore || this->pHmiInterfaceEventBuffer == NULL)
    return;

  HmiInterfaceMessage msg;
  char mess[LineCarNbMax];

  if (!this->pHmiInterfaceEventBuffer->GetBytes((byte*)&msg, sizeof(msg)))
    return;

  switch (msg.event)
  {
  case HmiInterfaceEvent_WifiStartConnection:
    Serial.println("HmiInterface :: WifiStartConnection");
    this->addNotification(0, HMI_WifiWaiting, 0);
    break;

  case HmiInterfaceEvent_WifiConnected:
    Serial.println("HmiInterface :: WifiConnected");

    addNotification(HMI_WifiOk);
    sprintf(mess, "%d.%d.%d.%d", msg.data.ip[0], msg.data.ip[1], msg.data.ip[2], msg.data.ip[3]);
    addNotification(mess);
    addNotification(TXT_BoxAddr);
    break;

  case HmiInterfaceEvent_WifiEndConnection:
    Serial.println("HmiInterface :: WifiEndConnection");
    this->addNotification(HMI_noWifi);
    break;

  case HmiInterfaceEvent_NewClient:
    Serial.println("HmiInterface :: NewClient");

    sprintf(mess, "#%d: %d.%d.%d.%d", msg.data.client.clientId, msg.data.client.ip[0], msg.data.client.ip[1], msg.data.client.ip[2], msg.data.client.ip[3]);
    this->addNotification(mess);
    break;

  case HmiInterfaceEvent_CloseClient:
    Serial.println("HmiInterface :: CloseClient");

    sprintf(mess, "#%d closed", msg.data.client.clientId);
    this->addNotification(mess);
    break;

  case HmiInterfaceEvent_ChangeSpeed:
    Serial.println("HmiInterface :: ChangeSpeed");
    addNotification(msg.data.dcc.addr, Locomotives::get(msg.data.dcc.addr)->isDirectionForward() ? HMI_OrderForward : HMI_OrderBack, msg.data.dcc.speed);
    break;

  case HmiInterfaceEvent_ChangeDirection:
    Serial.println("HmiInterface :: ChangeDirection");
    addNotification(msg.data.dcc.addr, msg.data.dcc.forward ? HMI_OrderForward : HMI_OrderBack, Locomotives::get(msg.data.dcc.addr)->getSpeed());
    break;

  case HmiInterfaceEvent_ChangeFunction:
    Serial.println("HmiInterface :: ChangeFunction");
    addNotification(msg.data.dcc.addr, HMI_OrderFunction, msg.data.dcc.function);
    break;

  case HmiInterfaceEvent_DccOn:
    Serial.println("HmiInterface :: DccOn");
    addNotification(HMI_StartDCC);
    break;

  case HmiInterfaceEvent_DccOff:
    Serial.println("HmiInterface :: DccOff");
    addNotification(HMI_OrderStopAll);
    break;

  case HmiInterfaceEvent_DccShort:
    Serial.println("HmiInterface :: Short");
    addNotification(HMI_ShortCurcuit);
    break;

  case HmiInterfaceEvent_DccEmergencyStop:
    Serial.println("HmiInterface :: EmergencyStop");
    // Todo : missing order emergency stop !
    addNotification(HMI_OrderStopAll);
    break;

  case HmiInterfaceEvent_DccGeneralStop:
    Serial.println("HmiInterface :: GeneralStop");
    addNotification(HMI_OrderStopAll);
    break;

  case HmiInterfaceEvent_LocoAdd:
    Serial.println("HmiInterface :: LocoAdd");
    addNotification(msg.data.dcc.addr, 
            Locomotives::get(msg.data.dcc.addr)->isDirectionForward() ? HMI_OrderForward : HMI_OrderBack, 
            Locomotives::get(msg.data.dcc.addr)->getSpeed());
    break;

  case HmiInterfaceEvent_LocoRemove:
    Serial.println("HmiInterface :: LocoRemove");
    addNotification(msg.data.dcc.addr, HMI_OrderStop, 0);
    break;
  }
  _HMIDEBUG_FCT_PRINTLN("hmi::HmiInterfaceLoop().. End");
}

void hmi::HmiInterfaceUpdateDrawing()
{
  this->update();
}

