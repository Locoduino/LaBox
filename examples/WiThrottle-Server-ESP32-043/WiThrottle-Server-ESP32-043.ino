/*
 * WiThrottle server implementation for LaBox command station
 * Software version 0.4.0 du 22 Avril 2020
 * Copyright (c) 2018-2019, Dominique Bultez & Locoduino http://www.locoduino.org
 * 
 * Change log:
 * 2020-04-22 - Transposition from the ESP8266version 0.3.5
 * Fixed an exception "LoadProhibited" in the tests of Client in the beginning of the loop
 * 2020-04-24 - Renamed ESP32-041 (version 0.4.1)
 * 2020-04-25 - version 0.4.2 : revision complète des variables 
 * 
 * Inspired by Valerie Valley RR https://sites.google.com/site/valerievalleyrr/ but largely rewritten
 * JMRI WiFi Throttle Communications Protocol http://jmri.sourceforge.net/help/en/package/jmri/jmrit/withrottle/Protocol.shtml
 * WiThrottle official site http://www.withrottle.com/WiThrottle/Home.html
 * Download WiThrottle on the AppStore https://itunes.apple.com/us/app/withrottle-lite/id344190130
 * Engine Driver official site https://enginedriver.mstevetodd.com/
 * Download Engine Driver on the GooglePlay https://play.google.com/store/apps/details?id=jmri.enginedriver
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
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
 * 
 */
#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiAP.h>
#include "LaBox.h" //au lieu de "DCCpp.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSerif9pt7b.h>


//////// VERSION ////////////
const char  g_VERSION[20] = "Centrale WiFi 0.4.3";
//////////////////////

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);
int upState = 1;
int downState = 1;
int selState = 1;
boolean change = false;

#define LED_BUILTIN 2   // Set the GPIO pin where you connected your test LED or comment this line out if your dev board has a built-in LED

// Set these to your desired credentials.
const char *ssid = "withrottle";
const char *password = "";
//IPAddress WTServer_Ip(10, 10, 10, 10); // adresse donnée par le serveur (bonne ou mauvaise idée ?)
//IPAddress WTServer_NMask(255, 255, 255, 0);
int WTServer_Port = 44444;
IPAddress myIP;

#define maxCommandLenght 30
#define maxClient 3

#define SERIAL_BAUDRATE           115200
//#define dbg_output                Serial
//#define DBG_RX_PIN_ID             GPIO_NUM_15                 
//#define DBG_TX_PIN_ID             GPIO_NUM_2                  
//#define dccpp_serial              Serial2
//#define DCCPP_RX_PIN_ID           GPIO_NUM_16
//#define DCCPP_TX_PIN_ID           GPIO_NUM_17
// Serial = debug et commandes directes dccpp sans passer par le port série

boolean PowerOnStart = 1; // DCC on au démarrage (peut être conditionné à la connexion d'un smarphone)

/* Define turnout object structures */
/* Pas de turnout pour le moment */
/*
typedef struct {
  int address;
  int id;
  int tStatus;
} tData;
tData tt[512];
*/

struct Engine {
  boolean alreadyConnected = false;
  int cli; // numero de client wifi
  boolean isWT;  // True : is Withrottle, false :is Engine Driver
  String LocoThrottle = {""}; // pour stocker actionkey "S12" ou "L4550" 
  String LocoName = {""};
  String LocoAdress = {""}; // S24 ou L4561 ou *
  int LocoState[29];    // bits des fonctions F1-F28
  int dccAdress;
  int newSpeed = 0;
  int oldSpeed = 0;
  int dir = 1;
};
Engine Loco[maxClient*2]; // pourquoi 2 fois ?
/* The interval of check connections between ESP & WiThrottle app */

const int heartbeatTimeout = 10; // secondes
boolean heartbeatEnable[maxClient]; 
unsigned long heartbeat[maxClient * 2];

char commandString[maxCommandLenght + 1];
boolean alreadyConnected[maxClient];
String powerStatus;
int Throttle = 0;
int fKey;
int locoId;  // DCC id for this loco
int locoStepsNumber = 128;  // 14, 28 or 128
int locoSpeed;  // Current speed
bool locoDirectionForward;  // current direction.
String Data = "";
String clientData = "";
String LocoAdress = ""; // S24 ou L4561 ou *

/* pour l'OLED */
unsigned long timeValue = 0;
boolean stateLed = true;
boolean line1 = false;
boolean line2 = false;
boolean line3 = false;
String Aline = "";
String Bline = "";
String Cline = "";
String Line0 = "";


WiFiServer server(WTServer_Port);
WiFiClient client[maxClient];

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(23, INPUT_PULLUP);//up
  pinMode(19, INPUT_PULLUP);//down
  pinMode(18, INPUT_PULLUP);//sel

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println("SSD1306 allocation failed");
    for(;;);
  }
  display.setRotation(2);
  digitalWrite(LED_BUILTIN, LOW);


  Serial.begin(SERIAL_BAUDRATE);
  //delay(2000);
  Serial.println(g_VERSION);
//  Serial.setDebugOutput(true); // Debug + complet
//  dccpp_serial.begin(SERIAL_BAUDRATE, SERIAL_8N1, DCCPP_RX_PIN_ID, DCCPP_TX_PIN_ID);
//  dccpp_serial.flush();
//  Serial.print("Configuring access point...");

  WiFi.mode(WIFI_STA);
  delay(1000);
  WiFi.disconnect(true);
  delay(1000);
  WiFi.softAPdisconnect(true);           // disconnects AP Mode 
  delay(1000); 
  
  WiFi.softAP(ssid, password);
  myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  server.begin();

  //Serial.println("Server started");
  ///if (PowerOnStart == 1)
  turnPowerOn();
  //  else
  //    turnPowerOff();
  //display.setFont(&FreeSerif9pt7b);
  display.clearDisplay();
  display.setRotation(2);
  display.setTextSize(1);             
  display.setTextColor(WHITE);        
  display.setCursor(0,0);  
 // Line0 = "AP : " + myIP;           
  display.println(myIP);
  //Serial.println(Line0);
  display.display();
  digitalWrite(LED_BUILTIN, LOW);
  
  DCCpp::begin();
  // configuration pour L6203 La Box
  DCCpp::beginMain(UNDEFINED_PIN, 33, 32, 36); // 33=DCC(Dir), 32=pwm/enable, 36-SVP= current sense
  digitalWrite(LED_BUILTIN, stateLed);
  DCCpp::powerOn();
  locoId = 10;
  locoStepsNumber = 128;
  locoSpeed = 0;
  locoDirectionForward = true;
  //locoFunctions.Clear();  // Already done by the constructor...
 
  timeValue = millis();

}

void affiche() {
  
  if (line1 || line2|| line3) {
    display.clearDisplay();
    display.setCursor(0,0);   
    display.println(myIP);
    
    display.setCursor(0,15);display.println(Aline);
    line1 = false;        
    display.setCursor(0,30);display.println(Bline);
    line2 = false;
    display.setCursor(0,45);display.println(Cline);
    line3 = false;
    display.display(); 
  }
}

void vuOled(int Throttle, int i) {
  DCCpp::setSpeedMain(Throttle, Loco[Throttle].dccAdress, locoStepsNumber, Loco[Throttle].newSpeed, Loco[Throttle].dir);
String Sens = Loco[Throttle].dir? ">>": "<<";
  switch (i) {
    case 0:
    line1 = true;                
    Aline = String(Loco[Throttle].dccAdress) + " " + Sens + " " + String(Loco[Throttle].newSpeed) + "     " ;
    break;
    case 1:
    line2 = true;                
    Bline = String(Loco[Throttle].dccAdress) + " " + Sens + " " + String(Loco[Throttle].newSpeed + "     " );
    break;
    case 2:
    line3 = true;                
    Cline = String(Loco[Throttle].dccAdress) + " " + Sens + " " + String(Loco[Throttle].newSpeed) + "     " ;
    break;
  }
}


////////////////////////// interfaces DCCpp ///////////////////
/*
 *   DCCpp::setSpeedMain(1, locoId, locoStepsNumber, locoSpeed, locoDirectionForward);
 *   DCCpp::activate(infunctionnumber)
 *   DCCpp::inactivate(infunctionnumber)
 *   DCCpp::setFunctionsMain(int nReg, int inLocoId, FunctionsState &inStates);
 *   à intégrer...
 * 
 */

void loop() {

int fKey;
int start;
int finish;
int last;
//int dccAdress;

  for (int i = 0; i < maxClient; i++) {
    if (!client[i]) {
      client[i] = server.available(); // ecoute de clients entrants, i = n° de client
    }
    else {
      if (!client[i].connected()) {      // client deconnecte
        Serial.printf("Client %d déconnecté.", i);
        throttleStop(i);
      }
      else if (!alreadyConnected[i]) {
        Serial.printf("Nouveau client %d : %d.%d.%d.%d:%d\n", i, client[i].remoteIP()[0], client[i].remoteIP()[1], client[i].remoteIP()[2], client[i].remoteIP()[3], WTServer_Port);
        //loadTurnouts();
        throttleStart(i);
      } else { // alreadyconnected
        if (client[i].available()) {
          Data = "";
          while (client[i].available()) {
            char c = client[i].read();             // read byte
            Data += c;
          }
          

          start = -1;
          last = Data.lastIndexOf('\n');
          while (start < last) {  // traitement d'une commande à la fois
            finish = Data.indexOf('\n', start + 1);
            clientData = Data.substring(start + 1, finish);
            Serial.print(i); Serial.print("-> ");Serial.println(clientData);
            start = finish;
            if (clientData.startsWith("*")) {               // est-ce *+ ?
              if (clientData.startsWith("*+")) {
                heartbeatEnable[i] = true;
                heartbeat[i] = millis();
              } 
              else {
              heartbeat[i] = millis();
              }
            }
            else if (clientData.startsWith("N")) { // device Name, send heartbeat
            //Loco[i].LocoName = clientData.substring(1);     // on garde tout sauf le 'N' devant
            client[i].println("*" + String(heartbeatTimeout) + "\n\n");
            }
            else if (clientData.startsWith("H")) {            // device ID
              //Serial.println(clientData);                   // a commenter
            }
            else if (clientData.startsWith("PPA")) {          // est-ce PPA0 (track off) ou PPA1 (track on) ou PPA2 (unknown)
              powerStatus = clientData.substring(3);          // a priori not suported by Withrottle
              Serial.println("<" + powerStatus + ">");            // vers dcc++ <1> ou <0>
              client[i].print("PPA" + powerStatus + "\n\n");
            }
            else if (clientData.startsWith("M")) { // creation, suppression, commande/action sur Withrottle
            //else if (clientData.startsWith("MT") || clientData.startsWith("MS") || clientData.startsWith("M0") || clientData.startsWith("M1")) {
              String th = clientData.substring(1, 2); // "T/S" (WiThrottle) ou "0/1" (Engine Driver)
              int delimiter = clientData.indexOf(";");// MTAS123<;>V4 ou MTAS123<;>F01
              LocoAdress = clientData.substring(4,delimiter-1);

              if (th == "T" || th == "S") {
                Throttle = 0 + i * 2; //0,2,4
                Loco[Throttle].isWT =  true;
                //Serial.print("WT ");
              }
              else { // (th == "0" || th == "1")
                Throttle = 1 + i * 2;//1,3,5
                Loco[Throttle].isWT =  false;
                //Serial.println("ED ");
              }
              Loco[Throttle].cli = i; // sauvegarde numéro de cilent
              String action = clientData.substring(2, 3);  // "creation "+" ou action "A" ou suppression "-"
              String actionData = clientData.substring(3); // reste de la commande "S12<;>S12" ou "L455<;>L455" ou "adresse"<;>"actionkey"
              
              String ad = clientData.substring(3,4); // "S ou L" (WiThrottle) ou "*" (Engine Driver : adresse implicite) ou F pour les fonctions (mais ??), avant <;>
              String actionKey = clientData.substring(delimiter + 2); // "S12" ou "L455" apres délimiter contient adresse DCC (+) ou action V/F
              //String actionVal = actionData.substring(delimiter + 2); // S, L ou qV, qR, R, V F et la suite identique actionKey
              
              //Serial.print(" dcc ");Serial.print(LocoAdress);Serial.print(" action ");Serial.print(action);Serial.print(" actionData ");Serial.print(actionData);Serial.print(" ad ");Serial.print(ad);Serial.print(" actionKey ");Serial.println(actionKey);
              
              Loco[Throttle].LocoThrottle = actionKey;
              //  dccAdress = actionKey.substring(1).toInt(); // après le S ou L se trouve l'adresse DCC short ou long ou l'action (donc pas toujours l'adresse dcc)
      
              if (action == "+") {  // ajout d'une loco associée à un trottle
                Loco[Throttle].dccAdress = LocoAdress.toInt();   // enregistre l'adresse DCC 
                locoAdd(th, actionKey, i); // th = MT, MS, M0 ou M1, actionkey = S ou L + adresse dcc
              } // action +
              else if (action == "-") { // suppression d'une loco associée à un trottle
                locoRelease(th, actionKey, i);
              } // action -
              else if (action == "A") {  // "A" action : fonction et vitesse/direction
                if (actionKey.startsWith("V")) { // vitesse
                  fKey = actionKey.substring(1).toInt(); // valeur de vitesse après le V
                  Loco[Throttle].newSpeed = fKey; // vitesse
                } // if action starts with V
                else {
                  locoAction(ad, th, actionKey, i); // fonctions
                }
              } // if (action == "A")
              heartbeat[Throttle] = millis();
            } // else if (clientData.startsWith("M") ...)
          } // while (start < last)
      
          for (Throttle = 0 + i * 2; Throttle < 2 + i * 2; Throttle++) { // de 0 à 5
            if (Loco[Throttle].newSpeed != Loco[Throttle].oldSpeed) {
              Loco[Throttle].oldSpeed = Loco[Throttle].newSpeed;
              //if (Loco[Throttle].dir) Serial.println(" Avt"); else Serial.println(" Arr");
              Serial.println("<t " + String(Throttle + 1) + " " + String(Loco[Throttle].dccAdress) + "  " + String(Loco[Throttle].newSpeed) + " " + String(Loco[Throttle].dir) + ">");
              //Serial.println("dcc+ ");Serial.print(String(Loco[Throttle].dccAdress));Serial.print(" V ");Serial.print(String(Loco[Throttle].newSpeed));
              vuOled(Throttle, i);
              //loadResponse();
            } //if newSpeed != oldSpeed
          } // for Trottles...
          if (heartbeatEnable[i]) {
            checkHeartbeat(i);
          } // if heartbeetEnable
        } //Client[i].available()
      } // else already connected
    } // else Client exist
  } // end for Clients
  affiche();
} // end loop





int invert(int value) {
  if (value == 0)
    return 1;
  else
    return 0;
}

void turnPowerOn() {
  powerStatus = "1";
  Serial.println("<1>");
  //Serial.print("Power ON\n");
}

void turnPowerOff() {
  powerStatus = "0";
  Serial.println("<0>");
  //Serial.print("Power OFF\n");
}

void loadResponse() {
  /*
  char c;
  while (Serial.available() > 0) {
    c = Serial.read();
    Serial.print(c);
  }
  */
}


void throttleStart(int i) {
  client[i].flush();
  client[i].setTimeout(500);
  //Serial.println("\nNew client");
  client[i].print("VN2.0\n\n");//  client[i].println("VN2.0");
  client[i].print("RL0\n\n");//  client[i].println("RL0");
  client[i].print("PPA2\n\n");//  client[i].println("PPA" + powerStatus);
  client[i].print("RCL0\n\n");
  client[i].print("PW12080\n\n");

//  client[i].println("PTT]\\[Turnouts}|{Turnout]\\[Closed}|{2]\\[Thrown}|{4");
//  client[i].print("PTL");
/*  for (int t = 0 ; tt[t].address != 0; t++) {
    client[i].print("]\\[LT" + String(tt[t].address) + "}|{" + tt[t].id + "}|{" + tt[t].tStatus);
  }
*/
//  client[i].println("");
//  client[i].println("*" + String(heartbeatTimeout));
  alreadyConnected[i] = true;
  turnPowerOn();
}

void throttleStop(int i) {
  //locoRelease(i);
  client[i].stop();
  Serial.println("\nSTOP");
  alreadyConnected[i] = false;
  heartbeatEnable[i] = false;
  Loco[0 + i * 2].newSpeed = 0; //speed WT
  heartbeat[0 + i * 2] = 0;
  Loco[1 + i * 2].newSpeed = 0; //speed ED ?
  heartbeat[1 + i * 2] = 0;
}

void locoAdd(String th, String actionKey, int i) { // th = T, S, 0 ou 1, actionkey = "S12" ou "L4550"  i= n° de client dans loop
  Loco[Throttle].LocoThrottle = actionKey;         // sauvegarde String adresse "S12" ou "L4550"
  if (Loco[Throttle].isWT) {
  client[i].print("MT+"+actionKey+"<;>\n\n");
  client[i].print("MTL"+actionKey+"<;>]\[Feux]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[\n\n");
  } else {
  client[i].print("M0+"+actionKey+"<;>\n\n");
  client[i].print("M0L"+actionKey+"<;>]\[Feux]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[]\[\n\n");
  }
  //client[i].println("M" + th + "A" + actionKey + "<;>"); // a revoir !
  for (fKey = 0; fKey < 29; fKey++) { 
    Loco[Throttle].LocoState[fKey] = 0; // raz des fonctions
    client[i].println("M" + th + "A" + actionKey + "<;>F0" + String(fKey) +"\n\n");
  }
  Loco[Throttle].newSpeed =0;
  Loco[Throttle].oldSpeed =0;
  Loco[Throttle].dir =1;  // avant
  Loco[Throttle].newSpeed = 0; // vitesse à remplacer par Loco[Throttle].newspeed
  Loco[Throttle].dir = 1; // direction à remplacer par Loco[Throttle].dir
  client[i].println("M" + th + "A" + actionKey + "<;>V0\n\n");
  client[i].println("M" + th + "A" + actionKey + "<;>R1\n\n");
  client[i].println("M" + th + "A" + actionKey + "<;>s1\n\n");
  //Serial.print("locoAdd "); Serial.print(actionKey); Serial.print(" adresse dcc ");Serial.println(Loco[Throttle].dccAdress);
  vuOled(Throttle, i);

}

void locoRelease(String th, String actionKey, int i) {
 
  String locoAddress = Loco[Throttle].LocoThrottle.substring(1);
  heartbeat[Throttle] = 0;
  //Loco[Throttle].LocoThrottle[Throttle] = "";
  //Serial.print("locoRelease ");Serial.println(Loco[Throttle].dccAdress);
  Serial.print("<t " + String(Throttle + 1) + " " + locoAddress + " 0 0 " + ">");
  //loadResponse();
  client[i].println("M" + th + "-" + actionKey + "<;>\n\n");
  Loco[Throttle].newSpeed =0;
  vuOled(Throttle, i);
}

void locoAction(String ad, String th, String actionKey, int i) { // fonctions F0 à F28 et autres fonctions S, L ou qV, qR, R, V F ..
  // ad si "F" ignorer (mais pourquoi ?)
  // th :  "MT" "MS" (WiThrottle) ou "M0" "M1" (Engine Driver)
  // actionkey = "S12" ou "L455" ou "F028" apres délimiter contient adresse DCC (<;>) actionval
  // i est le numero de client
  String locoAddress = String(Loco[Throttle].dccAdress);
  if (actionKey == "*") { // heartbeat
    actionKey = Loco[Throttle].LocoThrottle; // restaurer actionKey "S12" ou "L455"
  } //*
  //if (ad != "F" && actionVal.startsWith("F")) { // inversion de l'état de la fonction F028 ou F128 en éliminant MTAF...
  if (actionKey.startsWith("F")) { // inversion de l'état de la fonction F028 ou F128
    //Loco[Throttle].LocoState[fKey] = actionVal.substring(1,2).toInt(); // etat de la fonction 0 ou 1
    fKey = actionKey.substring(2).toInt(); // numero de fonction dans la commande
    Loco[Throttle].LocoState[fKey] = invert(Loco[Throttle].LocoState[fKey]); // inversion
    //Serial.println("->M" + th + "A" + Loco[Throttle].LocoThrottle + "<;>" + "F" + String(Loco[Throttle].LocoState[fKey]) + String(fKey));
    //client[i].println("M" + th + "A" + Loco[Throttle].LocoThrottle + "<;>" + "F1" + String(fKey));
    //client[i].println("M" + th + "A" + Loco[Throttle].LocoThrottle + "<;>" + "F0" + String(fKey));
    client[i].println("M" + th + "A" + Loco[Throttle].LocoThrottle + "<;>" + "F" + String(Loco[Throttle].LocoState[fKey]) + String(fKey));
    byte func;
    switch (fKey) {
      case 0:
      case 1:
      case 2:
      case 3:
      case 4:
        func = 128 + Loco[Throttle].LocoState[1] * 1 + Loco[Throttle].LocoState[2] * 2 + Loco[Throttle].LocoState[3] * 4 + Loco[Throttle].LocoState[4] * 8 + Loco[Throttle].LocoState[0] * 16;
        Serial.println("<f " + locoAddress + " " + String(func) + ">");
        break;
      case 5:
      case 6:
      case 7:
      case 8:
        func = 176 + Loco[Throttle].LocoState[5] * 1 + Loco[Throttle].LocoState[6] * 2 + Loco[Throttle].LocoState[7] * 4 + Loco[Throttle].LocoState[8] * 8;
        Serial.println("<f " + locoAddress + " " + String(func) + ">");
        break;
      case 9:
      case 10:
      case 11:
      case 12:
        func = 160 + Loco[Throttle].LocoState[9] * 1 + Loco[Throttle].LocoState[10] * 2 + Loco[Throttle].LocoState[11] * 4 + Loco[Throttle].LocoState[12] * 8;
        Serial.println("<f " + locoAddress + " " + String(func) + ">");
        break;
      case 13:
      case 14:
      case 15:
      case 16:
      case 17:
      case 18:
      case 19:
      case 20:
        func = Loco[Throttle].LocoState[13] * 1 + Loco[Throttle].LocoState[14] * 2 + Loco[Throttle].LocoState[15] * 4 + Loco[Throttle].LocoState[16] * 8 + Loco[Throttle].LocoState[17] * 16 + Loco[Throttle].LocoState[18] * 32 + Loco[Throttle].LocoState[19] * 64 + Loco[Throttle].LocoState[20] * 128;
        Serial.println("<f " + locoAddress + " " + String(222) + " " + String(func) + ">");
        break;
      case 21:
      case 22:
      case 23:
      case 24:
      case 25:
      case 26:
      case 27:
      case 28:
        func = Loco[Throttle].LocoState[21] * 1 + Loco[Throttle].LocoState[22] * 2 + Loco[Throttle].LocoState[23] * 4 + Loco[Throttle].LocoState[24] * 8 + Loco[Throttle].LocoState[25] * 16 + Loco[Throttle].LocoState[26] * 32 + Loco[Throttle].LocoState[27] * 64 + Loco[Throttle].LocoState[28] * 128;
        Serial.println("<f " + locoAddress + " " + String(223) + " " + String(func) + ">");
        break;
    } // switch
  } // start with F
  else if (actionKey.startsWith("qV")) { // ask for current speed  // actionKey remplace LocoThrottle[Throttle]
    client[i].println("M" + th + "A" + actionKey + "<;>" + "V" + String(Loco[Throttle].newSpeed) + "\n\n");
  } //qV
  else if (actionKey.startsWith("V")) {  // sets the speed (velocity)
    fKey = actionKey.substring(1).toInt(); //  vitesse
    Loco[Throttle].newSpeed = fKey;
    //Serial.print("dcc ");Serial.print(String(Loco[Throttle].dccAdress));Serial.print(" V ");Serial.print(String(Loco[Throttle].newSpeed));
    //if (Loco[Throttle].dir) Serial.println(" Avt"); else Serial.println(" Arr");
    Serial.println("<t " + String(Throttle + 1) + " " + String(Loco[Throttle].dccAdress) + "  " + String(fKey) + " " + String(Loco[Throttle].dir) + ">");
    vuOled(Throttle, i);
    //loadResponse();
  } //V
  else if (actionKey.startsWith("qR")) { // ask for current direction
    client[i].println("M" + th + "A" + actionKey + "<;>" + "R" + String(Loco[Throttle].dir) + "\n\n");
  } //qR
  else if (actionKey.startsWith("R")) {  // set direction.
    fKey = actionKey.substring(1).toInt();
    Loco[Throttle].dir = fKey;
    Loco[Throttle].newSpeed = 0; // arret loco obligatoire (mais le curseur de WT ne revient pas à 0 !!)
    //Serial.print("dcc ");Serial.print(String(Loco[Throttle].dccAdress));Serial.print(" V: ");Serial.print(String(Loco[Throttle].newSpeed));
    if (Loco[Throttle].dir) Serial.println(" Avt"); else Serial.println(" Arr");
    Serial.println("<t " + String(Throttle + 1) + " " + String(Loco[Throttle].dccAdress) + " 0 " + String(fKey) + ">");
    vuOled(Throttle, i);
    //loadResponse();
    //client[i].println("M" + th + "A" + actionKey + "<;>" + "V" + String(Loco[Throttle].newSpeed) + "\n\n"); // ca ne suffit pas pour positionner le curseur

  } //R
  else if (actionKey.startsWith("X")) {  // emergency STOP : set speed to -1
    Loco[Throttle].newSpeed = 0;
    //Serial.print("dcc ");Serial.print(String(Loco[Throttle].dccAdress));Serial.println(" Emergency STOP");
    Serial.println("<t " + String(Throttle + 1) + " " + String(Loco[Throttle].dccAdress) + " -1 " + String(Loco[Throttle].dir) + ">");
    vuOled(Throttle, i);
    //loadResponse();
  } //X
  else if (actionKey.startsWith("I")) {  // IDLE : set speed to 0
    Loco[Throttle].newSpeed = 0;
    //Serial.print("dcc ");Serial.print(String(Loco[Throttle].dccAdress));Serial.println(" IDLE");
    Serial.println("<t " + String(Throttle + 1) + " " + String(Loco[Throttle].dccAdress) + " 0 " + String(Loco[Throttle].dir) + ">");
    vuOled(Throttle, i);//loadResponse();
  } //I
  else if (actionKey.startsWith("Q")) {  // the loco QUIT, set speed to 0
    Loco[Throttle].newSpeed = 0;
    //Serial.print("dcc ");Serial.print(String(Loco[Throttle].dccAdress));Serial.println(" QUIT");
    Serial.println("<t " + String(Throttle + 1) + " " + String(Loco[Throttle].dccAdress) + " 0 " + String(Loco[Throttle].dir) + ">");
    vuOled(Throttle, i);
    //loadResponse();
  } //Q
} // locoAction


void checkHeartbeat(int i) {
  if (heartbeat[0 + i * 2] > 0 && heartbeat[0 + i * 2] + heartbeatTimeout *5000 < millis()) {
    Loco[0 + i * 2].newSpeed = 0;
    heartbeat[0 + i * 2] = 0;
    client[i].println("MTA" + Loco[0 + i * 2].LocoThrottle + "<;>" + "V0");
  }
  if (heartbeat[1 + i * 2] > 0 && heartbeat[1 + i * 2] + heartbeatTimeout * 5000 < millis()) {
    Loco[1 + i * 2].newSpeed = 0;
    heartbeat[1 + i * 2] = 0;
    client[i].println("MSA" + Loco[1 + i * 2].LocoThrottle + "<;>" + "V0");
  }
}

/*
void accessoryToggle(int aAddr, String aStatus) {
  int newStat;
  if (aStatus == "T")
    newStat = 1;
  else if (aStatus == "C")
    newStat = 0;
  else
    newStat = -1;
  int t = 0;
  for (t = 0 ; tt[t].address != 0 && tt[t].address != aAddr; t++);
  if (tt[t].address == 0 && newStat > -1) {
    int addr = ((aAddr - 1) / 4) + 1;
    int sub = aAddr - addr * 4 + 3;
    for (int i = 0; i < maxClient; i++) {
      client[i].println("PTA2LT" + String(aAddr));
    }
    Serial.print("<a " + String(addr) + " " + String(sub) + " " + String(newStat) + ">");
  }
  else {
    if (newStat == -1) {
      switch (tt[t].tStatus) {
        case 2:
          tt[t].tStatus = 4;
          newStat = 0;
          break;
        case 4:
          tt[t].tStatus = 2;
          newStat = 1;
          break;
      }
    }
    else {
      switch (newStat) {
        case 0:
          tt[t].tStatus = 2;
          break;
        case 1:
          tt[t].tStatus = 4;
          break;
      }
    }
    for (int i = 0; i < maxClient; i++) {
      client[i].println("PTA" + String(tt[t].tStatus) + "LT" + String(tt[t].address));
    }
    Serial.print("<T " + String(tt[t].id) + " " + String(newStat) + ">");
    //loadResponse();
  }
}
*/
/*
void loadTurnouts() {
  Serial.write("<T>");
  char c;
  while (Serial.available() <= 0)
  {
    delay(300);
  }
  int t = 0;
  while (Serial.available() > 0)
  {
    c = Serial.read();
    if (c == '<')
      sprintf(commandString, " ");
    else if (c == '>')
    {
      String s;
      char *str = (char*)commandString;
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
      int z = 2;
      if (s == "1") z = 4;
      int a = (x - 1) * 4 + y + 1;
      tt[t] = {a, id, z};
      t++;
    }
    else if (strlen(commandString) < maxCommandLenght)
      sprintf(commandString, "%s%c", commandString, c);
  }
}*/
