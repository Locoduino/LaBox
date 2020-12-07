/*
 * Truncated JMRI WiThrottle server implementation for DCC++ command station
 * Software version 1.03b
 * Copyright (c) 2016-2018, Valerie Valley RR https://sites.google.com/site/valerievalleyrr/
 * 
 * Change log:
 * 2017-09-10 - Fixed release responce for WiThrottle.app
 *              Fixed synchronize power status on clients
 * 2017-09-24 - Added mDNS responder
 *              Added start delay to fix connection problem with DCC++
 * 2018-12-11 - Fixed Engine Driver v2.19+ throttle numbers 
 * 
 * DCC++ https://github.com/DccPlusPlus
 * ESP8266 Core https://github.com/esp8266/Arduino
 * JMRI WiFi Throttle Communications Protocol http://jmri.sourceforge.net/help/en/package/jmri/jmrit/withrottle/Protocol.shtml
 * WiThrottle official site http://www.withrottle.com/WiThrottle/Home.html
 * Download WiThrottle on the AppStore https://itunes.apple.com/us/app/withrottle-lite/id344190130
 * Engine Driver official site https://enginedriver.mstevetodd.com/
 * Download Engine Driver on the GooglePlay https://play.google.com/store/apps/details?id=jmri.enginedriver
 * 
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * 
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Create a WiFi access point. */
#include <WiFi.h>
#include <WiFiMulti.h>
#include "Config.h"
#define maxCommandLenght 30
#define maxClient  4

/* Define turnout object structures */
typedef struct {
  int address;  
  int id;
  int tStatus;
} tData;
tData tt[512];

/* The interval of check connections between ESP & WiThrottle app */
const int heartbeatTimeout = 10;
boolean heartbeatEnable[maxClient];
unsigned long heartbeat[maxClient*2];

String LocoThrottle[]={"","","","","",""};
int LocoState[maxClient*2][31];
int fKey;
int Throttle =0;
int start;
int finish;
int last;
char commandString[maxCommandLenght+1];
boolean alreadyConnected[maxClient];
String powerStatus;
char msg[16];

const char* ssid = "VIDEOFUTUR_C56165_2.4G";
const char* password = "EenPghQD";

// the media access control (ethernet hardware) address for the shield:
uint8_t wifiMac[] = { 0xBE, 0xEF, 0xBE, 0xEF, 0xBE, 0x80 };
//the IP address for the shield:
uint8_t wifiIp[] = { 192, 168, 1, 100 };
uint8_t wifiMask[] = { 255, 255, 255, 0 };
#define PowerOnStart  0

WiFiMulti wifiMulti;

/* Define WiThrottle Server */
WiFiServer server(23);
WiFiClient client[maxClient];

void setup() {
  delay(1500);
  Serial.begin(115200);
  Serial.flush();
  wifiMulti.addAP(ssid, password);

  Serial.println("Connecting Wifi...");
  if(wifiMulti.run() == WL_CONNECTED) {
     Serial.println("");
     Serial.println("WiFi connected");
     Serial.println("IP address: ");
     Serial.println(WiFi.localIP());
  }

  server.begin();
  server.setNoDelay(true);
  
  /*if (PowerOnStart == 1)
    turnPowerOn();
  else
    turnPowerOff();*/
}

void loop() {
  for(int i=0; i<maxClient; i++){
    if (!client[i]) {
      client[i] = server.available();
    }
    else {
      /*if (client[i].connected()) {
        Serial.println("Client lost");
        throttleStop(i);
      }
      else*/ if(!alreadyConnected[i]) {
        Serial.println("New client");
        loadTurnouts();
        throttleStart(i);
      }
    }
    if (client[i].available()) {
      String Data = client[i].readString();
      Serial.println(Data);
      start = -1;
      last = Data.lastIndexOf('\n');
      String changeSpeed[]={"","","","","",""};
      while(start < last){
        finish = Data.indexOf('\n', start+1);
        String clientData = Data.substring(start+1, finish);
        start = finish;
        if (clientData.startsWith("*+")) {
          heartbeatEnable[i] = true;
        }
        else if (clientData.startsWith("PPA")){
          powerStatus = clientData.substring(3);
          Serial.println("<"+powerStatus+">");
          client[i].println("PPA"+powerStatus);
          for(int p=0; p<maxClient; p++){
            if (alreadyConnected[p]){
              client[p].println("PPA"+powerStatus);
            }
          }
        }
        else if (clientData.startsWith("PTA")){
          String aStatus = clientData.substring(3,4);
          int aAddr = clientData.substring(6).toInt();
          accessoryToggle(aAddr, aStatus);
        }
        else if (clientData.startsWith("N") || clientData.startsWith("*")){
          client[i].println("*" + String(heartbeatTimeout));
        }
        else if (clientData.startsWith("MT") || clientData.startsWith("MS") || clientData.startsWith("M0") || clientData.startsWith("M1")) {
          String th = clientData.substring(1,2);
          if (th == "T" || th == "0")
            Throttle = 0+i*2;
          else
            Throttle = 1+i*2;
          String action = clientData.substring(2,3);
          String actionData = clientData.substring(3);
          int delimiter = actionData.indexOf(";");
          String actionKey = actionData.substring(0, delimiter-1);
          String actionVal = actionData.substring(delimiter+2);
          if (action == "+") {
            locoAdd(th, actionKey, i);
          }
          else if (action == "-") {
            locoRelease(th, actionKey, i);
          }
          else if (action == "A") {
            if(actionVal.startsWith("V")){
              changeSpeed[Throttle]=th;
              fKey=actionVal.substring(1).toInt();
              LocoState[Throttle][29]=fKey;
            }
            else {
              locoAction(th, actionKey, actionVal, i);
            }
          }
          heartbeat[Throttle]=millis();
        }
      }
      for(Throttle=0+i*2;Throttle<2+i*2;Throttle++){
        if(changeSpeed[Throttle]!="" && LocoThrottle[Throttle]!=""){
          String locoAddress=LocoThrottle[Throttle].substring(1);
          fKey=LocoState[Throttle][29];
          Serial.print("<t "+String(Throttle+1)+" "+locoAddress+"  "+String(fKey)+" "+String(LocoState[Throttle][30])+">");
          String response = loadResponse();
        }
      }
      if (heartbeatEnable[i]) {
        checkHeartbeat(i);
      }
    }
  }
}

int invert(int value){
  if(value == 0)
    return 1;
  else
    return 0;
}

void turnPowerOn() {
  powerStatus = "1";
  while (Serial.available() <= 0) {
    Serial.println("<1>");
    delay(300);
  }
  char c;
  while(Serial.available()>0)
  {
    c=Serial.read();
  }
}

void turnPowerOff() {
  powerStatus = "0";
  while (Serial.available() <= 0) {
    Serial.println("<0>");
    delay(300);
  }
  char c;
  while(Serial.available()>0)
  {
    c=Serial.read();
  }
}

String loadResponse() {
  while (Serial.available() <= 0) {
    delay(300);
  }
  char c;
  while(Serial.available()>0) {
    c=Serial.read();
    if(c=='<')
      sprintf(commandString,"");
    else if(c=='>')
      return (char*)commandString;
    else if(strlen(commandString)<maxCommandLenght)
      sprintf(commandString,"%s%c",commandString,c);
  }
}

void loadTurnouts() {
  Serial.write("<T>");
  char c;
  while (Serial.available() <= 0)
  {
    delay(300);
  }
  int t=0;
  while(Serial.available()>0)
  {
    c=Serial.read();
    if(c=='<')
      sprintf(commandString,"");
    else if(c=='>')
    {
      String s;
      char *str=(char*)commandString;
      char * pch;
      pch = strtok(str, " ");
      s = (char*)pch;
      int id = s.substring(1).toInt();
      pch = strtok (NULL, " ");
      s = (char*)pch;
      int x = s.toInt();
      pch = strtok (NULL, " ");
      s = (char*)pch;
      int y = s.toInt();
      pch = strtok (NULL, " ");
      s = (char*)pch;
      int z=2;
      if(s=="1") z=4;
      int a = (x-1)*4+y+1;
      tt[t]={a,id,z};
      t++;
    }
    else if(strlen(commandString)<maxCommandLenght)
      sprintf(commandString,"%s%c",commandString,c);
  }
}

void throttleStart(int i) {
  client[i].flush();
  client[i].setTimeout(500);
  Serial.println("\nNew client");
  client[i].println("VN2.0");
  client[i].println("RL0");
  client[i].println("PPA"+powerStatus);
  client[i].println("PTT]\\[Turnouts}|{Turnout]\\[Closed}|{2]\\[Thrown}|{4");
  client[i].print("PTL");
  for (int t = 0 ; tt[t].address != 0; t++) {
    client[i].print("]\\[LT"+String(tt[t].address)+"}|{"+tt[t].id+"}|{"+tt[t].tStatus);
  }
  client[i].println("");
  client[i].println("*"+String(heartbeatTimeout));
  alreadyConnected[i] = true;
}

void throttleStop(int i) {
  client[i].stop();
  Serial.println("\n");
  alreadyConnected[i] = false;
  heartbeatEnable[i] = false;
  LocoState[0+i*2][29] = 0;
  heartbeat[0+i*2] = 0;
  LocoState[1+i*2][29] = 0;
  heartbeat[1+i*2] = 0;
}

void locoAdd(String th, String actionKey, int i) {
  LocoThrottle[Throttle] = actionKey;
  client[i].println("M"+th+"+"+actionKey+"<;>");
  for(fKey=0; fKey<29; fKey++){
    LocoState[Throttle][fKey] =0;
    client[i].println("M"+th+"A"+actionKey+"<;>F0"+String(fKey));
  }
  LocoState[Throttle][29] =0;
  LocoState[Throttle][30] =1;
  client[i].println("M"+th+"+"+actionKey+"<;>V0");
  client[i].println("M"+th+"+"+actionKey+"<;>R1");
  client[i].println("M"+th+"+"+actionKey+"<;>s1");
}

void locoRelease(String th, String actionKey, int i) {
  String locoAddress = LocoThrottle[Throttle].substring(1);
  heartbeat[Throttle] =0;
  LocoThrottle[Throttle] = "";
  Serial.print("<t "+String(Throttle+1)+" "+locoAddress+" 0 "+String(LocoState[Throttle][30])+">");
  String response = loadResponse();
  client[i].println("M"+th+"-"+actionKey+"<;>");

}

void locoAction(String th, String actionKey, String actionVal, int i){
  String locoAddress = LocoThrottle[Throttle].substring(1);
  String response;
  if(actionKey == "*"){
    actionKey = LocoThrottle[Throttle];
  }
  if(actionVal.startsWith("F1")){
    fKey = actionVal.substring(2).toInt();
    LocoState[Throttle][fKey] = invert(LocoState[Throttle][fKey]);
    client[i].println("M"+th+"A"+LocoThrottle[Throttle]+"<;>" + "F"+String(LocoState[Throttle][fKey])+String(fKey));
    byte func;
    switch(fKey){
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
        func = 128+LocoState[Throttle][1]*1+LocoState[Throttle][2]*2+LocoState[Throttle][3]*4+LocoState[Throttle][4]*8+LocoState[Throttle][0]*16;
        Serial.println("<f "+locoAddress+" "+String(func)+">");
      break;
      case 5:
      case 6:
      case 7:
      case 8:
        func = 176+LocoState[Throttle][5]*1+LocoState[Throttle][6]*2+LocoState[Throttle][7]*4+LocoState[Throttle][8]*8;
        Serial.println("<f "+locoAddress+" "+String(func)+">");
      break;
      case 9:
      case 10:
      case 11:
      case 12:
        func = 160+LocoState[Throttle][9]*1+LocoState[Throttle][10]*2+LocoState[Throttle][11]*4+LocoState[Throttle][12]*8;
        Serial.println("<f "+locoAddress+" "+String(func)+">");
      break;
      case 13:
      case 14:
      case 15:
      case 16:
      case 17:
      case 18:
      case 19:
      case 20:
        func = LocoState[Throttle][13]*1+LocoState[Throttle][14]*2+LocoState[Throttle][15]*4+LocoState[Throttle][16]*8+LocoState[Throttle][17]*16+LocoState[Throttle][18]*32+LocoState[Throttle][19]*64+LocoState[Throttle][20]*128;
        Serial.println("<f "+locoAddress+" "+String(222)+" "+String(func)+">");
      break;
      case 21:
      case 22:
      case 23:
      case 24:
      case 25:
      case 26:
      case 27:
      case 28:
        func = LocoState[Throttle][21]*1+LocoState[Throttle][22]*2+LocoState[Throttle][23]*4+LocoState[Throttle][24]*8+LocoState[Throttle][25]*16+LocoState[Throttle][26]*32+LocoState[Throttle][27]*64+LocoState[Throttle][28]*128;
        Serial.println("<f "+locoAddress+" "+String(223)+" "+String(func)+">");
      break;
    }
  }
  else if(actionVal.startsWith("qV")){
    client[i].println("M"+th+"A"+LocoThrottle[Throttle]+"<;>" + "V"+String(LocoState[Throttle][29]));              
  }
  else if(actionVal.startsWith("V")){
    fKey = actionVal.substring(1).toInt();
    LocoState[Throttle][29] = fKey;
    Serial.print("<t "+String(Throttle+1)+" "+locoAddress+"  "+String(fKey)+" "+String(LocoState[Throttle][30])+">");
    response = loadResponse();
  }
  else if(actionVal.startsWith("qR")){
    client[i].println("M"+th+"A"+LocoThrottle[Throttle]+"<;>" + "R"+String(LocoState[Throttle][30]));              
  }
  else if(actionVal.startsWith("R")){
    fKey = actionVal.substring(1).toInt();
    LocoState[Throttle][30] = fKey;
    Serial.print("<t "+String(Throttle+1)+" "+locoAddress+" "+String(LocoState[Throttle][29])+"  "+String(fKey)+">");
    response = loadResponse();
  }
  else if(actionVal.startsWith("X")){
    LocoState[Throttle][29] = 0;
    Serial.print("<t "+String(Throttle+1)+" "+locoAddress+" -1 "+String(LocoState[Throttle][30])+">");
    response = loadResponse();
  }
  else if(actionVal.startsWith("I")){
    LocoState[Throttle][29] = 0;
    Serial.print("<t "+String(Throttle+1)+" "+locoAddress+" 0 "+String(LocoState[Throttle][30])+">");
    response = loadResponse();
  }
  else if(actionVal.startsWith("Q")){
    LocoState[Throttle][29] = 0;
    Serial.print("<t "+String(Throttle+1)+" "+locoAddress+" 0 "+String(LocoState[Throttle][30])+">");
    response = loadResponse();
  }
}

void checkHeartbeat(int i) {
  if(heartbeat[0+i*2] > 0 && heartbeat[0+i*2] + heartbeatTimeout * 1000 < millis()) {
    LocoState[0+i*2][29] = 0;
    heartbeat[0+i*2] = 0;
    client[i].println("MTA"+LocoThrottle[0+i*2]+"<;>" + "V0");
  }
  if(heartbeat[1+i*2] > 0 && heartbeat[1+i*2] + heartbeatTimeout * 1000 < millis()) {
    LocoState[1+i*2][29] = 0;
    heartbeat[1+i*2] = 0;
    client[i].println("MSA"+LocoThrottle[1+i*2]+"<;>" + "V0");
  }
}

void accessoryToggle(int aAddr, String aStatus){
  int newStat;
  if(aStatus=="T") 
    newStat=1;
  else if(aStatus=="C")
    newStat=0;
  else
    newStat=-1;
  int t=0;
  for (t = 0 ; tt[t].address!=0 && tt[t].address!=aAddr; t++);
  if(tt[t].address==0 && newStat>-1){
    int addr=((aAddr-1)/4)+1;
    int sub=aAddr-addr*4+3;
    for(int i=0; i<maxClient; i++){
      client[i].println("PTA2LT"+String(aAddr));
    }
    Serial.print("<a "+String(addr)+" "+String(sub)+" "+String(newStat)+">");
  }
  else {
    if(newStat==-1){
      switch(tt[t].tStatus){
        case 2:
          tt[t].tStatus=4;
          newStat=0;
        break;
        case 4:
          tt[t].tStatus=2;
          newStat=1;
        break;
      }
    }
    else {
      switch(newStat){
        case 0:
          tt[t].tStatus=2;
        break;
        case 1:
          tt[t].tStatus=4;
        break;
      }
    }
    for(int i=0; i<maxClient; i++){
      client[i].println("PTA"+String(tt[t].tStatus)+"LT"+String(tt[t].address));
    }
    Serial.print("<T "+String(tt[t].id)+" "+String(newStat)+">");
    String response = loadResponse();
  }
}
