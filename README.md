# How to use it?
It is suggested to do a manual installation, please follow the "Manual Installation" steps in this document: https://www.arduino.cc/en/Guide/Libraries.

# How does it works?

An instance of WiFiModule represents an Arduino Wifi module ESP8266 ESP-01. For now the idea is to call from the outside #joinNetwork(), #getIpAssigned() and #startTCPServer(), in that order. Hence the starting code that uses this library should be inside Arduino's setup() and look like:

```
   WiFiModule wifiModule = WiFiModule();
   wifiModule.joinNetwork(WiFiModule::NETWORK_SSID, WiFiModule::NETWORK_PASSWORD);
   String ipAssigned = wifiModule.getIpAssigned();
   wifiModule.startTCPServer();
```

The IP received (ipAssigned) MUST be shown in the robot screen. Then in the loop() part of the Arduino app you should call:

```
   WiFiModule.getIncomingRequest();
```

And later, if a request was entered, once processed it, you should call:

```
   WiFiModule.sendOutgoingResponse();
```

That function will close the pipe with the client by itself.

P.D.: WifiModule::NETWORK_SSID and WifiModule::NETWORK_PASSWORD are currently harcoded into the library, for now, hardcode the user and password of your wifi network there. In a next release, it will be provided a way to pass this data externally.

# About the DCP (Digital Communication Protocol)

* Custom _downstream_ TCP/IP protocol layer.
* Implemented in methods #getIncomingRequest() and #sendOutgoingResponse().
* Maximum package length: 2048 bytes (including message, headers and body).
* There's no such thing as a body.
* The structure of a request is divided in 2 parts:
  * The subject of the message in caps, in the first line.
  * Arguments, aka header in HTTP. Keys should be downcased and must not contain ":" as part of its name, for values it doesn't matter. It isn't mandatory to contain arguments.
* A request message looks like:
```
LOAD MAP
map:kitchen
id:3
```
* The structure of a response is divided in 2 parts:
  * The first line contains:
    * The length of the whole message.
    * After a comma you'll find "OK" or "FAIL".
  * The rest of the lines are arguments, aka header in HTTP. Keys should be downcased and must not contain ":" as part of its name, values doesn't matter. It isn't mandatory to contain arguments.
* A response message looks like:
```
31,OK
loaded map:kitchen
id:3
```
* Line breaks must be \r\n

