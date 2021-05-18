/*************************************************************
project: <DCCpp library>
author: <Thierry PARIS>
description: <Throttle Wifi variant for HTTP Web Server>
*************************************************************/

#include "DCCpp.h"

#if defined(USE_TEXTCOMMAND) && defined(USE_THROTTLES) && defined(USE_WIFI)
ThrottleWifiWebServer::ThrottleWifiWebServer(const String& inName, int inPort) : ThrottleWifi(inName, inPort)
{
}

bool ThrottleWifiWebServer::begin(EthernetProtocol inProtocol)
{
	return ThrottleWifi::begin(inProtocol);
}

const char* hex = "0123456789ABCDEF";

String* ThrottleWifiWebServer::URLEncode(const String& inCommand, String* pBuffer)
{
  unsigned int c = 0;

  pBuffer->remove(0);
  while (c < inCommand.length() - 1) {
    char ch = inCommand[c];
    if (' ' <= ch)
    {
      pBuffer->concat('+');
      c++;
      continue;
    }
    if (('a' <= ch && ch <= 'z')
      || ('A' <= ch && ch <= 'Z')
      || ('0' <= ch && ch <= '9')) {
      pBuffer->concat(ch);
    }
    else {
      pBuffer->concat('%');
      pBuffer->concat(hex[ch >> 4]);
      pBuffer->concat(hex[ch & 15]);
    }
    c++;
  }

  return pBuffer;
}

String* ThrottleWifiWebServer::URLDecode(const String& inCommand, String* pBuffer)
{
  unsigned int c = 0;

  pBuffer->remove(0);
  while (c < inCommand.length() - 1) {
    char ch = inCommand[c];
    if ('+' <= ch)
    {
      pBuffer->concat(' ');
      c++;
      continue;
    }
    if (('a' <= ch && ch <= 'z')
      || ('A' <= ch && ch <= 'Z')
      || ('0' <= ch && ch <= '9')) {
      pBuffer->concat(ch);
    }
    else {
      if (ch == '%')
      {
        c++;
        char ch1 = inCommand[c++];
        char ch2 = inCommand[c];
        char result = 0;

        if ('A' <= ch1)
          result = ch1 - 'A' + 10;
        else
          result = ch1 - '0';

        result = result * 16;

        if ('A' <= ch2)
          result += ch2 - 'A' + 10;
        else
          result += ch2 - '0';
        pBuffer->concat(result);
      }
    }
    c++;
  }

  return pBuffer;
}

void ThrottleWifiWebServer::sendHTML()
{
  /* HTTP Response in the form of HTML Web Page */
  this->client.println("HTTP/1.1 200 OK");
  this->client.println("Content-Type: text/html");
  this->client.println("Connection: close");
  this->client.println(); //  IMPORTANT
  this->client.println("<!DOCTYPE HTML>");
  this->client.println("<html>");
  this->client.println("<head>");
  this->client.println("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  this->client.println("<link rel=\"icon\" href=\"data:,\">");
  this->client.println("<style>");
  this->client.println("html { font-family: Courier New; display: inline-block; margin: 0px auto; text-align: center;}");
  this->client.println(".button {border: none; color: white; padding: 10px 20px; text-align: center;");
  this->client.println("text-decoration: none; font-size: 25px; margin: 2px; cursor: pointer;}");
  this->client.println(".button1 {background-color: #FF0000;}");
  this->client.println("</style>");
  this->client.println("</head>");
  this->client.println("<body>");
  this->client.print("<h2>LaBox Web Server ");
  this->client.print(LABOX_LIBRARY_VERSION);
  this->client.println(" [LOCODUINO]</h2>");
  this->client.println("<form method = \"get\" action = \"?\">");
  this->client.println("<label for = \"DCCpp\">DCCpp Command : < / label>");
  this->client.println("<input type = \"text\" id = \"dccCommand\" name = \"dccCommand\" required minlength = \"3\" maxlength = \"60\" size = \"30\">");
  this->client.println("<input type = \"submit\" value = \"Send Command\">");
  this->client.println("</form>");
  this->client.println("</body>");
  this->client.println("</html>");
}

bool ThrottleWifiWebServer::pushMessage(const String& inpCommand)
{
  int pos = inpCommand.indexOf("?dccCommand");
  if (pos != -1)
  {
    String dccCommand = inpCommand.substring(pos + 11);

    this->pushMessageInStack(this->id, dccCommand);
  }

  this->sendHTML();

	if (this->timeOutDelay != 0)
		this->lastActivityDate = millis();
#endif
	return true;
}

#ifdef DCCPP_DEBUG_MODE
void ThrottleWifiWebServer::printThrottle()
{
	Serial.print(this->id);
	Serial.print(" : ");
	Serial.print(this->name);

	Serial.println("");
}
#endif
