#ifndef MQTTDest_h
#define MQTTDest_h

#include "interfaces/Destination.hpp"
#include "interfaces/Plugin.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "MenuHandler.hpp"
#include "Arduino.h"
#include <PubSubClient.h>
#include <ESP8266WiFi.h>

//#define PROP_CUSTOM_URL_ARR F("custom_url_arr")
class MQTTDest : public Destination {
public:
  MQTTDest();
  bool setup(MenuHandler *handler);
  void loop();
  static void cmdMqttSetup(const char *line);
  static void cmdMqttMsgAdd(const char *line);
  static void cmdCleanCustomUrl(const char *line);
  static void cmdCallMqtt(const char *line);
  static void onReceive(char* topic1, byte* payload1, unsigned int length);
  void cmdCallMqttInst(const char *line);
  bool process(LinkedList<Pair*> &data);
  void replaceValuesInURL(LinkedList<Pair *> &data, String &s);
  bool mqttStart();
  bool reconnect();
  void mqttEnd(bool res);
//  void invokeURL(String &url);
const char* getName() {
  return "MQTT";
}

private:
  PubSubClient *client;
  WiFiClient *wclient;
  bool isListening = false;
  void setupMqttListen();
  bool mqttListen = false;
  //static const __FlashStringHelper* PROP_CUSTOM_URL_ARR;// = F("custom_url_arr");
};

#endif
