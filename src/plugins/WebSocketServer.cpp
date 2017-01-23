#include "plugins/WebSocketServer.hpp"
#include "MenuHandler.hpp"
#include <Timer.h>
#include <ESP8266NetBIOS.h>
#include <ESP8266mDNS.h>
#include "plugins/WifiStuff.hpp"
extern WifiStuffClass WifiStuff;

//const char* apiKey = "Qw8rdb20aV";
//http://randomkeygen.com/


WebSocketServerClass::WebSocketServerClass() {
  registerPlugin(this);
}

bool WebSocketServerClass::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("wss_start"), CMD_BEGIN, WebSocketServerClass::cmdStartServer, F("wss_start")));
  handler->registerCommand(new MenuEntry(F("wss_stop"), CMD_EXACT, WebSocketServerClass::cmdStopServer, F("wss_stop")));
  if (!PowerManager.isWokeFromDeepSleep() && PropertyList.readBoolProperty(PROP_WSSERVER_STARTONBOOT)) {
    menuHandler.scheduleCommand("wss_start");
  }
  return false;

}

void WebSocketServerClass::cmdStartServer(const char *ignore) {
  myWSS.cmdStartServerInst();
}

void WebSocketServerClass::cmdStopServer(const char *ignore) {
  myWSS.cmdStopServerInst();
}

void WebSocketServerClass::cmdStopServerInst() {
  if (server!= NULL) {
    delete server;
    server = NULL;
  }
}


void WebSocketServerClass::loop() {
  if (server != NULL) server->loop();
}

void WebSocketServerClass::cmdStartServerInst() {
  if (server != NULL) delete server;
  if (WifiStuff.waitForWifi() != WL_CONNECTED) return;
  server = new WebSocketsServer(8266);
  //menuHandler.scheduleCommand("nop 0");
  //server->on("/", WebSocketServerClass::onCommand);
  server->begin();
  server->onEvent(WebSocketServerClass::onWebSocketEvent);

  LOGGER << F("\n\nWebSockets Server started on wss://") << WiFi.localIP() << F(":") << 8266 << endl;
  LOGGER.flush();
  LOGGER << F("Open http://config.vair-monitor.com to configure device online") << endl;
  LOGGER.flush();
  // LOGGER.print(WiFi.localIP());
  // LOGGER.println(F("/?cmd=..."));
  //
  // String hostname = PropertyList.readProperty(PROP_ESP_HOSTNAME);
  // if (hostname.length() == 0) hostname = "vthing";
  // LOGGER << F("Web Server accessible on :") << endl;
  // LOGGER << F("   http://") << WiFi.localIP() << endl;
  // if (MDNS.begin(hostname.c_str())) {
  //   LOGGER << F("   http://") << hostname << F(".local/") << endl;
  //   MDNS.addService("http", "tcp", 80);
  // }
  // NBNS.begin(hostname.c_str());
  // LOGGER << F("   http://") << hostname << F("/") << endl;
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
              LOGGER.printf(String(F("[%u] Disconnected!\n")).c_str(), num);
              menuHandler.scheduleCommand("nop 300");
              break;
          case WStype_CONNECTED:
              {
                  IPAddress ip = server->remoteIP(num);
                  LOGGER.setLogToWSS(true);
                  server->sendTXT(num, "Connected");
                  LOGGER.printf(String(F("[%u] Connected from %d.%d.%d.%d url: %s\n")).c_str(), num, ip[0], ip[1], ip[2], ip[3], payload);
                  LOGGER.flush();

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
              LOGGER.printf(String(F("[%u] get binary lenght: %u\n")).c_str(), num, length);
              //hexdump(payload, length);

              // send message to client
              // webSocket.sendBIN(num, payload, lenght);
              break;
      }

}

void WebSocketServerClass::onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  myWSS.onWebSocketEventInst(num, type, payload, length);
}

void WebSocketsServer::handleNonWebsocketConnection(WSclient_t * client) {
    DEBUG_WEBSOCKETS("[WS-Server][%d][handleHeader] no Websocket connection close.\n", client->num);
    client->tcp->write(String(F("HTTP/1.1 400 Bad Request\r\n"
            "Server: arduino-WebSocket-Server\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 32\r\n"
            "Connection: close\r\n"
            "Sec-WebSocket-Version: 13\r\n"
            "\r\n"
            "This is a Websocket server only!")).c_str());
    clientDisconnect(client);
}

/**
 * called if a non Authorization connection is coming in.
 * Note: can be override
 * @param client WSclient_t *  ptr to the client struct
 */
void WebSocketsServer::handleAuthorizationFailed(WSclient_t *client) {

    client->tcp->write(String(F("HTTP/1.1 401 Unauthorized\r\n"
            "Server: arduino-WebSocket-Server\r\n"
            "Content-Type: text/plain\r\n"
            "Content-Length: 45\r\n"
            "Connection: close\r\n"
            "Sec-WebSocket-Version: 13\r\n"
            "WWW-Authenticate: Basic realm=\"WebSocket Server\""
            "\r\n"
            "This Websocket server requires Authorization!")).c_str());
    clientDisconnect(client);
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
