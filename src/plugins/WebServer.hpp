#ifndef WebServerH
#define WebServerH
#include "common.hpp"
#include "PropertyList.hpp"
#include "interfaces/Plugin.hpp"
#include <Timer.h>
#include <ESP8266WebServer.h>


#define PROP_WEBSERVER_STARTONBOOT F("ws.autostart")

class WebServerClass : public Plugin {
public:
  WebServerClass() ;
  bool setup(MenuHandler *handler);
  const char* getName() {
    return "WebServer";
  }
  void loop();
  static void onCommand();
  void onCommandInst();
  static void cmdStartWebServer(const char *ignore);
  void cmdStartWebServerInst();
  void onProperty(String &key, String &value);
  ESP8266WebServer *server;

  bool autoStart = false;

};

extern WebServerClass WebServer;
#endif
