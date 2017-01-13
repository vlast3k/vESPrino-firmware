#include "plugins/WebServer.hpp"
#include "MenuHandler.hpp"
#include <Timer.h>
#include <ESP8266WebServer.h>
#include <ESP8266NetBIOS.h>
#include <ESP8266mDNS.h>
#include "plugins/WifiStuff.hpp"
extern WifiStuffClass WifiStuff;

const char* apiKey = "Qw8rdb20aV";
//http://randomkeygen.com/


WebServerClass::WebServerClass() {
  registerPlugin(this);
}

bool WebServerClass::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("webserver_start"), CMD_BEGIN, WebServerClass::cmdStartWebServer, F("webserver_start")));
  if (!PowerManager.isWokeFromDeepSleep() && PropertyList.readBoolProperty(PROP_WEBSERVER_STARTONBOOT)) {
    menuHandler.scheduleCommand("webserver_start");
  }
  return false;

}

void WebServerClass::cmdStartWebServer(const char *ignore) {
  WebServer.cmdStartWebServerInst();
}
void WebServerClass::loop() {
  if (server != NULL) server->handleClient();
}

void WebServerClass::cmdStartWebServerInst() {
  if (server != NULL) delete server;
  if (WifiStuff.waitForWifi() != WL_CONNECTED) return;
  server = new ESP8266WebServer(80);
  //menuHandler.scheduleCommand("nop 0");
  server->on("/", WebServerClass::onCommand);
  server->begin();

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

void WebServerClass::onCommand() {
  WebServer.onCommandInst();
}

void WebServerClass::onCommandInst() {
  // if (!server.hasArg("key") || server.arg("key") != apiKey) {
  //   server.send(401, "text/plain", "missing key");
  //   return;
  // }

  if (server->hasArg("cmd")) {
    menuHandler.scheduleCommand(server->arg("cmd").c_str());
  }

  server->send(200, "text/plain", "Received");
}
