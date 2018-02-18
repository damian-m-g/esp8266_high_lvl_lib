/* Created by Damián M. González on 2017/05/18. */

#include "WiFiModule.hpp"


// Constructor.
WiFiModule::WiFiModule() {
  Serial.begin(BAUD_RATE);
  // startup
  Serial.println("AT");
  // not interested in the answer
  consumeResponse();
}


// Joins the network passed as *ssid* with due *password*. Returns true if everything went fine, false otherwise.
bool WiFiModule::joinNetwork(const String& ssid, const String& password) {
  // set module as a client of the network
  Serial.println("AT+CWMODE_CUR=1");
  // not interested in the answer
  consumeResponse();
  // try connection to the network, if ssid or pass contains " or , or \ must be escaped by a backward slash
  String cwjapDirective = "AT+CWJAP_CUR=\"" + NETWORK_SSID + "\",\"" + NETWORK_PASSWORD + "\"";
  Serial.println(cwjapDirective);
  String response = consumeResponse(12000);
  byte responseCode = parseResponse(response);
  // if everything went right, ask for our ip
  if(responseCode == 0) {
    ipAssigned = getAcquiredIp();
  }
}


// Setup a TCP server on the module. Port TCP 333 used. Some security improvements can be made in the future.
void WiFiModule::startTCPServer() const {
  // create TCP server, uses the default port (333)
  Serial.println("AT+CIPSERVER=1");
  consumeResponse();
  // set server timeout, in seconds
  Serial.println("AT+CIPSTO=10");
  consumeResponse();
}


// You'll want to call this method anytime you want after starting a TCP Server. Checks if there's an incoming message from a client, returns nullptr if there's none, otherwise returns a #DCPRequest. According to my observations, when a connection has success against the server, the server returns "Link". Then anything wrote from the client takes the form "\r\n+IPD,11:Hello world\r\nOK\r\n".
DCPRequest WiFiModule::getIncomingRequest() const {
  if(Serial.available()) {
    // give the network 1.5 sec to let enter the whole message to the module buffer
    String response = consumeResponse(1500, "\r\nOK\r\n");
    if(response != "") {
      // filter what's important
      int colonIndex = response.indexOf(':');
      int requestLength = response.substring(7, colonIndex).toInt();
      String message = response.substring((colonIndex + 1), (colonIndex + requestLength + 1));
      // build the object to return
      return DCPRequest(message);
    } else {
      // something went wrong while receiving data
      return nullptr;
    }
  } else {
    return nullptr;
  }
}


// Send a response to the current connected DCP client. The link gets closed if the message is successfully sent.
bool WiFiModule::sendOutgoingResponse(const DCPResponse& dcpResponse) const {
  // make the module aware of the amount of characters that are going to be sent
  Serial.println("AT+CIPSEND=" + String(dcpResponse.messageLength));
  // consume garbage
  consumeResponse(1000, "> ");
  // sent the actual response
  Serial.print(dcpResponse.message);
  // consume the response, see if was successfully sent
  String response = consumeResponse(1500);
  byte responseCode = parseResponse(response);
  if(responseCode == 0) {
    // everything went fine, close the connection with the client
    unlinkClient();
  } else {
    // ATTENTION: Re-send message? For now let's suppose everything run smooth.
  }
}

// *timeout* is milliseconds. Consume all chars written to the output of the serial. Returns all those chars as a
// #String. This function is not asynchronous. The response that the module does could vary a lot. Most of the time
// returns some data, then two CR (\r\n\r\n, hence a blank line), "OK" and a CR (\r\n). But sometimes may return
// something like "no change\r\n", "ready\r\n", maybe else.
String WiFiModule::consumeResponse(const int& timeout = 1000, const String& mustEndWith = "") const {
  unsigned long alpha_time = millis();
  unsigned long beta_time;
  String response = "";
  bool endFound = false;
  while(!endFound) {
    if(Serial.available() > 0) {
      char ch = Serial.read();
      response += ch;
      // check if this was the end of the message
      if(mustEndWith == "") {
        for(String finalizer : REPONSE_FINALIZERS) {
          if(response.endsWith(finalizer)) {
            endFound = true;
            break;
          }
        }
      } else {
        if(response.endsWith(mustEndWith)) {
          endFound = true;
          break;
        }
      }
    }
    // check if the loop got timed out
    beta_time = millis();
    int time_difference = beta_time - alpha_time;
    if(time_difference >= timeout) {
      break;
    }
  }
  // only if end went found, parse response to see if everything went fine
  if(endFound) {
    return response;
  } else {
    return "";
  }
}


/*
* Works well with next commands:
*   "AT"
*   "AT+CWMODE_CUR="
*   "AT+CWJAP_CUR="
*
* Returns:
*   GOOD:
*     0 => everything went fine
*   BAD:
*     1 => AT+CWJAP, connection timeout
*     2 => AT+CWJAP, wrong password
*     3 => AT+CWJAP, cannot find the target AP
*     4 => AT+CWJAP, connection failed
*     100 => something went wrong, hard to say what
*/
byte WiFiModule::parseResponse(const String& response) const {
  if(response.endsWith("OK\r\n") || response.endsWith("no change\r\n") || response.endsWith("SEND OK\r\n")) {
    return 0;
  } else if(response.endsWith("\r\n\r\nERROR\r\n")) {
    int index = response.indexOf("\r\n\r\nERROR\r\n");
    // expecting "1", "2", "3" or "4"
    if(index > 0) {
      char errorCode = response.charAt(index - 1);
      return ((int)errorCode - 48);
    } else {
      return 100;
    }
  } else {
    return 100;
  }
}


// It is supposed that this function isn't called before get a successfull conection against the network. Returns
// something like "192.168.0.2".
String WiFiModule::getAcquiredIp() {
  Serial.println("AT+CIFSR");
  String response = consumeResponse();
  int endOfSoftApIp = response.indexOf("\r\n");
  String restOfResponse = response.substring(endOfSoftApIp + 1);
  int endOfStationIp = restOfResponse.indexOf("\r\n");
  return restOfResponse.substring(0, endOfStationIp);
}


// Closes the connection with the current client.
bool WiFiModule::unlinkClient() const {
  Serial.println("AT+CIPCLOSE;");
  consumeResponse();
}
