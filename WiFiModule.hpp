/*
* Created by Damián M. González on 2017/05/18.
* Made for Arduino Wifi module ESP8266 ESP-01.
* ATTENTION: An improvement in memory consumption may be made trying to convert all String objects into char[], not
* worth for now.
*/

// guard declaration
#ifndef LC_61020_WIFIMODULE_HPP
#define LC_61020_WIFIMODULE_HPP

// include dependencies
#include <Arduino.h>
#include <HardwareSerial.h>
#include <HardwareSerial_private.h>
#include "DCPRequest.hpp"
#include "DCPResponse.hpp"

// Serial is defined elsewhere by Arduino (environment/IDE)
extern HardwareSerial Serial;

/*
* An instance of this class represents an Arduino Wifi module ESP8266 ESP-01. For now the idea is to call from the
* outside #joinNetwork(), #getIpAssigned() and #startTCPServer(), in that order. Hence the starting code that uses this
* library should be inside setup() and look like:
*
*   WiFiModule wifiModule = WiFiModule();
*   wifiModule.joinNetwork(WiFiModule::NETWORK_SSID, WiFiModule::NETWORK_PASSWORD);
*   String ipAssigned = wifiModule.getIpAssigned();
*   wifiModule.startTCPServer();
*
* The IP received (ipAssigned) MUST be shown in the robot screen.
* Then in the loop() part of the Arduino app you should call:
*
*   WiFiModule.getIncomingRequest();
*
* And later, if a request entered, once processed it, you should call:
*
*   WiFiModule.sendOutgoingResponse();
*
* That function will close the pipe with the client by itself.
*/
class WiFiModule {

  public:
    // constants
    static const int BAUD_RATE = 9600;
    // ATTENTION: In the future, a differente way to acquire SSID and pass may be supported
    static const String NETWORK_SSID = "**********";
    static const String NETWORK_PASSWORD = "**********";
    static const String REPONSE_FINALIZERS[] = {"OK\r\n", "SEND OK\r\n", "no change\r\n", "ready\r\n", "ERROR\r\n"};

  private:
    // fields
    String ipAssigned = nullptr;

  public:
    // constructor
    WiFiModule();

    // public functions
    bool joinNetwork(const String& ssid, const String& password);
    void startTCPServer() const;
    // could return nullptr, gets probably assigned in joinNetwork()
    inline String getIpAssigned() const noexcept {return ipAssigned;};
    DCPRequest getIncomingRequest() const;
    bool sendOutgoingResponse(const DCPResponse& dcpResponse) const;

  private:
    // private functions
    String consumeResponse(const int& timeout = 1000, const String& mustEndWith = "") const;
    byte parseResponse(const String& response) const;
    String getAcquiredIp();
    bool unlinkClient() const;
};

#endif //LC_61020_WIFIMODULE_HPP
