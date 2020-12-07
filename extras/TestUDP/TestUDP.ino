/*
 *  This sketch sends random data over UDP on a ESP32 device
 *
 */
#include <WiFi.h>
#include <WiFiAP.h>
#include <WiFiUdp.h>

// WiFi network name and password:
const char * networkName = "Z21Server";
const char * networkPswd = "";

//IP address to send UDP data to:
// either use the ip address of the server or 
// a network broadcast address
const char * udpAddress = "192.168.1.11";
const int udpPort = 44444;

//Are we currently connected?
boolean connected = false;

//The udp library class
WiFiUDP udp;

void setup()
{
  // Initilize hardware serial:
  Serial.begin(115200);
  
  //Connect to the WiFi network
  connectToWiFi(networkName, networkPswd);
}

void loop(){
  //data will be sent to server
  uint8_t buffer[50];
/*  buffer[0] = '\x04';
  buffer[1] = '\x00';
  buffer[2] = '\x10';
  buffer[3] = '\x00';
  //send hello world to server
  udp.beginPacket(udpAddress, udpPort);
  udp.write(buffer, 4);
  udp.endPacket();
  */

  IPAddress ip;
  int port;
  
  memset(buffer, 0, 50);
  //processing incoming packet, must be called before reading the buffer
  udp.parsePacket();
  //receive response from server, it will be HELLO WORLD
  int len = udp.read(buffer, 50);
  if (len > 0)
  {
    ip = udp.remoteIP();
    port = udp.remotePort();
    Serial.print(ip);
    Serial.print(":");
    Serial.print(port);
    Serial.print(" ");
    for (int i = 0; i < len; i++)
    {
      Serial.print((int)buffer[i], HEX);
      Serial.print(" ");
    }
    Serial.println("");
    
    //resend answer
    udp.beginPacket(ip, port);
    udp.write((uint8_t*)"Renvoyé ", 8);
    udp.write(buffer, len); // double string to confirm...
    udp.endPacket();
  }
  
  //Wait for 1 second
//  delay(500);
}

void connectToWiFi(const char * ssid, const char * pwd){
  Serial.println("Connecting to WiFi network: " + String(ssid));

  //DO NOT TOUCH
  //  This is here to force the ESP32 to reset the WiFi and initialise correctly.
  WiFi.mode(WIFI_STA);
  delay(1000);
  WiFi.disconnect(true);
  delay(1000);
  WiFi.softAPdisconnect(true);           // disconnects AP Mode 
  delay(1000); 
  // End silly stuff !!!
  
  //register event handler
  //WiFi.onEvent(WiFiEvent);
  
  WiFi.softAP(networkName, networkPswd);
  
  Serial.print("WiFi connected! IP address: ");
  Serial.print(WiFi.softAPIP());  
  Serial.print(":");
  Serial.println(udpPort);
  
  udp.begin(WiFi.softAPIP(),udpPort);
}

//wifi event handler
void WiFiEvent(WiFiEvent_t event){
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          //When connected set 
          Serial.print("WiFi connected! IP address: ");
          Serial.print(WiFi.softAPIP());  
          Serial.print(":");
          Serial.println(udpPort);
          //initializes the UDP state
          //This initializes the transfer buffer
          connected = true;
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          connected = false;
          break;
      default: break;
    }
}
