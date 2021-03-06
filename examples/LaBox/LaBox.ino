/*************************************************************
project: <LaBox>
author: <Thierry PARIS>
description: <LaBox Wifi Controller sample>
*************************************************************/

#include "LaBox.h"

#if !defined(USE_TEXTCOMMAND) || !defined(USE_WIFI)
#error To be able to compile this sample,the lines #define USE_TEXTCOMMAND and #define USE_WIFI_EXTERNSSID or USE_WIFI_LOCALSSID must be uncommented in DCCpp.h
#endif

// WIFI

#ifdef USE_WIFI_EXTERNSSID
const char* ssid = "VIDEOFUTUR_ED5939_2.4G";
const char* password = "*******";

IPAddress ip      (192,   168,  0,  200);   // fix IP of the server in client mode
IPAddress gateway (192,   168,  0,  1);     // WiFi router's IP. If you are in AP mode, it's the ESP IP. If you are in client, it's your own gateway
IPAddress subnet  (255,   255,  255,  0);    // Subnet mask
IPAddress dns     (192,   168,  0,  1);     // DNS server, generally, the WiFi access point is a DNS server, but we can define another one.

#endif
#ifdef USE_WIFI_LOCALSSID
const char* ssid = "Locoduino LaBox";
const char* password = "";
#endif

//--------------------------- HMI client -------------------------------------
hmi boxHMI(&Wire);
//----------------------------------------------------------------------------

int dccPPPort = 2560;
int wifiPort = 44444;

// SERIAL
 
SERIAL_INTERFACE(Serial, Normal);
ThrottleSerial throttleSerial("Serial", SERIAL_INTERFACE_INSTANCE(Normal));

ThrottleWifiJMRI throttleWifiJMRI;

ThrottleWifi throttleWifi1("Z21 - 1", Z21_UDPPORT);
MessageConverterZ21 converter1;
ThrottleWifi throttleWifi2("Z21 - 2", Z21_UDPPORT);
MessageConverterZ21 converter2;
ThrottleWifi throttleWifi3("Z21 - 3", Z21_UDPPORT);
MessageConverterZ21 converter3;

ThrottleWifi throttleWifi4("WiThrottle - 1", wifiPort);
MessageConverterWiThrottle converterWT1;
ThrottleWifi throttleWifi5("WiThrottle - 2", wifiPort);
MessageConverterWiThrottle converterWT2;
ThrottleWifi throttleWifi6("WiThrottle - 3", wifiPort);
MessageConverterWiThrottle converterWT3;

void setup()
{
  Serial.begin(115200);

  // setup led !
  pinMode(PIN_LEDBUILTIN, OUTPUT);
  digitalWrite(PIN_LEDBUILTIN, HIGH);

  Serial.print("LaBox ");
  Serial.println(LABOX_LIBRARY_VERSION);

  //----------- Start HMI -------------
  boxHMI.begin();
  
  //Locomotives::debugLocomotivesSet();

  //----------- Start Wifi
#ifdef USE_WIFI_LOCALSSID
  ThrottleWifi::connectWifi(ssid, password);
#else
  ThrottleWifi::connectWifi(ssid, password, ip, gateway, subnet, dns);
#endif

  throttleSerial.begin();

  // For JMRI connection
  throttleWifiJMRI.begin();

  // For DCCpp syntax applications like DCCpp cab or RTDrive+
  //throttleWifi0.begin(TCP);

  // Z21 applications
  throttleWifi1.begin(&converter1);
  throttleWifi2.begin(&converter2);
  throttleWifi3.begin(&converter3);

  // WiThrottle protocol (WiThrottle and Engine Driver apps)
  throttleWifi4.begin(&converterWT1);
  throttleWifi5.begin(&converterWT2);
  throttleWifi6.begin(&converterWT3);

  DCCpp::begin();

  // configuration pour L6203 La Box
  DCCpp::beginMain(UNDEFINED_PIN, 33, 32, 36);
  DCCpp::setResendFunctions(false);

  digitalWrite(PIN_LEDBUILTIN, LOW);
  //DCCpp::setCurrentSampleMaxMain(4000);
}

void loop()
{
  boxHMI.update();
  DCCpp::loop();
}
