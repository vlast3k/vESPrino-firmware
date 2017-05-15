#ifndef WebSocketServerH
#define WebSocketServerH
#include "common.hpp"
#include "PropertyList.hpp"
#include "interfaces/Plugin.hpp"
#include <Timer.h>
#include <WebSocketsServer.h>
#include <Hash.h>


#define PROP_ESP_HOSTNAME F("hostname")


class WebSocketServerClass : public Plugin {
public:
  WebSocketServerClass() ;
  bool setup(MenuHandler *handler);
  const char* getName() {
    return "WebSocketServer";
  }
  void loop();
  void onProperty(String &key, String &value);
  // static void onCommand();
  // void onCommandInst();
  static void cmdStartServer(const char *ignore);
  static void cmdStopServer(const char *ignore);
  void cmdStopServerInst();
  void cmdStartServerInst();
  static void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
  void onWebSocketEventInst(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
  void sendData(uint8_t * payload, size_t length);
  void sendData(uint8_t ch);
  WebSocketsServer *server;
  uint32_t lastActivity = 0;
  uint8_t stopAfterMin = 5;
  bool autoStart = false;
};

extern WebSocketServerClass myWSS;
#endif
