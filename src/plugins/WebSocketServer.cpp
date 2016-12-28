#include "plugins/WebSocketServer.hpp"
#include "MenuHandler.hpp"
#include <Timer.h>
#include <ESP8266NetBIOS.h>
#include <ESP8266mDNS.h>

//const char* apiKey = "Qw8rdb20aV";
//http://randomkeygen.com/


WebSocketServerClass::WebSocketServerClass() {
  registerPlugin(this);
}

void WebSocketServerClass::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("wss_start"), CMD_BEGIN, WebSocketServerClass::cmdStartWebSocketServer, F("wss_start")));
  if (!PowerManager.isWokeFromDeepSleep() && PropertyList.readBoolProperty(PROP_WSSERVER_STARTONBOOT)) {
    menuHandler.scheduleCommand("wss_start");
  }
}

void WebSocketServerClass::cmdStartWebSocketServer(const char *ignore) {
  myWSS.cmdStartWebSocketServerInst();
}
void WebSocketServerClass::loop() {
  if (server != NULL) server->loop();
}

void WebSocketServerClass::cmdStartWebSocketServerInst() {
  if (server != NULL) delete server;
  if (waitForWifi() != WL_CONNECTED) return;
  server = new WebSocketsServer(81);
  //menuHandler.scheduleCommand("nop 0");
  //server->on("/", WebSocketServerClass::onCommand);
  server->begin();
  server->onEvent(WebSocketServerClass::onWebSocketEvent);

  LOGGER.print(F("\n\nSend commands to http://"));
  LOGGER.print(WiFi.localIP());
  LOGGER.println(F("/?cmd=..."));

  String hostname = PropertyList.readProperty(PROP_ESP_HOSTNAME);
  if (hostname.length() == 0) hostname = "vthing";
  LOGGER << F("Web Server accessible on :") << endl;
  LOGGER << F("   http://") << WiFi.localIP() << endl;
  if (MDNS.begin(hostname.c_str())) {
    LOGGER << F("   http://") << hostname << F(".local/") << endl;
    MDNS.addService("http", "tcp", 80);
  }
  NBNS.begin(hostname.c_str());
  LOGGER << F("   http://") << hostname << F("/") << endl;
}

void WebSocketServerClass::sendData(uint8_t * payload, size_t length) {
  if (server) server->sendTXT(0, payload, length);
}

void WebSocketServerClass::sendData(uint8_t ch) {
  char aa[2] = {ch, 0};
  if (server) server->sendTXT(0, aa);

}

void WebSocketServerClass::onWebSocketEventInst(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {

      switch(type) {
          case WStype_DISCONNECTED:
              LOGGER.printf("[%u] Disconnected!\n", num);
              menuHandler.scheduleCommand("nop 300");
              break;
          case WStype_CONNECTED:
              {
                  IPAddress ip = server->remoteIP(num);
                  LOGGER.setLogToWSS(true);
                  server->sendTXT(num, "Connected");
                  LOGGER.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

  				// send message to client
              }
              break;
          case WStype_TEXT:
              //LOGGER.printf("[%u] get Text: %s\n", num, payload);
              menuHandler.scheduleCommand((char *)payload);

              // send message to client
              // webSocket.sendTXT(num, "message here");

              // send data to all connected clients
              // webSocket.broadcastTXT("message here");
              break;
          case WStype_BIN:
              LOGGER.printf("[%u] get binary lenght: %u\n", num, length);
              hexdump(payload, length);

              // send message to client
              // webSocket.sendBIN(num, payload, lenght);
              break;
      }

}

void WebSocketServerClass::onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  myWSS.onWebSocketEventInst(num, type, payload, length);
}

// void WebSocketServerClass::onCommand() {
//   WebSocketServer.onCommandInst();
// }
//
// void WebSocketServerClass::onCommandInst() {
//   // if (!server.hasArg("key") || server.arg("key") != apiKey) {
//   //   server.send(401, "text/plain", "missing key");
//   //   return;
//   // }
//
//   if (server->hasArg("cmd")) {
//     menuHandler.scheduleCommand(server->arg("cmd").c_str());
//   }
//
//   server->send(200, "text/plain", "Received");
// }
