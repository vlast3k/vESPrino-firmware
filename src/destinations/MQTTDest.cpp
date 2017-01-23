#include "interfaces/Destination.hpp"
#include "interfaces/Plugin.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "MQTTDest.hpp"
#include "common.hpp"
#include "MenuHandler.hpp"
#include "plugins/PropertyList.hpp"
#include "plugins/AT_FW_Plugin.hpp"
#include <PubSubClient.h>
#include <ESP8266WiFi.h>
#include "plugins/WifiStuff.hpp"
extern WifiStuffClass WifiStuff;
extern MQTTDest mqttDest;
#define PROP_MQTT_SUBS_TOPIC F("mqtt.subs.topic")
#define PROP_MQTT_LISTEN F("mqtt.listen")

MQTTDest::MQTTDest() {
  registerDestination(this);
}

bool MQTTDest::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("mqtt_setup"), CMD_BEGIN, &MQTTDest::cmdMqttSetup, F("mqtt_setup \"idx\"value")));
  handler->registerCommand(new MenuEntry(F("mqtt_msg_add"), CMD_BEGIN, &MQTTDest::cmdMqttMsgAdd, F("mqtt_msg_add \"idx\"value")));
  handler->registerCommand(new MenuEntry(F("call_mqtt"), CMD_BEGIN, &MQTTDest::cmdCallMqtt, F("call_mqtt topic message")));
  handler->registerCommand(new MenuEntry(F("mqtt_msg_clean"), CMD_EXACT, &MQTTDest::cmdCleanCustomUrl, F("mqtt_msg_clean - clean all mqtt messages")));
  if (PropertyList.readBoolProperty(PROP_MQTT_LISTEN)) {
    mqttListen = true;
    if (!mqttStart()) {
      mqttEnd(false);
      return false;
    }
  }
  return false;
}

void MQTTDest::setupMqttListen() {
  if (isListening) return;
  if (!PropertyList.hasProperty(PROP_MQTT_LISTEN)) return;
  String s = PropertyList.readProperty(PROP_MQTT_SUBS_TOPIC);
  LOGGER << F("Setup Listen: ") << s << endl;
  if (!s.length()) return;
  s += "/cmd";
  client->subscribe(s.c_str());
  LOGGER << F("Accepting commands via MQTT on topic: ") << s << endl;
  isListening = true;
  for (int i=0; i < 10; i++) {
    LOGGER << F("Checking for Retained messages: ") << i << endl;
    loop();
    delay(200);
  }
}

void MQTTDest::cmdMqttSetup(const char *p) {
  char mqttServer[30], mqttPortS[6], mqttClient[20], mqttUser[45], mqttPass[45], mqttTopic[40];
  p = extractStringFromQuotes(p, mqttServer, sizeof(mqttServer));
  p = extractStringFromQuotes(p, mqttPortS,   sizeof(mqttPortS));
  p = extractStringFromQuotes(p, mqttClient, sizeof(mqttClient));
  p = extractStringFromQuotes(p, mqttUser,   sizeof(mqttUser));
  p = extractStringFromQuotes(p, mqttPass,   sizeof(mqttPass));
//  p = extractStringFromQuotes(p, mqttTopic,  sizeof(mqttTopic));
  //LOGGER << "Gere" << endl;
  delay(100);
  if (mqttClient[0] == 0) strcpy(mqttTopic, "vThing");
  if (mqttTopic[0] == 0) {
    #ifdef VTHING_H801_LED
      strcpy(mqttTopic, "vThingH801");
    #else
      strcpy(mqttTopic, "vThing/data");
    #endif
  }

  PropertyList.putProperty(EE_MQTT_SERVER, mqttServer);
  PropertyList.putProperty(EE_MQTT_PORT, mqttPortS);
  PropertyList.putProperty(EE_MQTT_CLIENT, mqttClient);
  PropertyList.putProperty(EE_MQTT_USER, mqttUser);
  PropertyList.putProperty(EE_MQTT_PASS, mqttPass);
//  PropertyList.putProperty(EE_MQTT_TOPIC, mqttTopic);

  LOGGER << F("MQTT Configuration Stored") << endl;
  LOGGER << mqttServer << "," << mqttPortS << "," << mqttClient << "," << mqttUser << "," << mqttPass << endl;
  LOGGER << F("DONE") << endl;
  LOGGER.flush();
#ifdef VTHING_H801_LED
  h801_mqtt_connect();
#endif
}

void MQTTDest::cmdMqttMsgAdd(const char *line) {
  //LOGGER << "menuAddCustomUrl" << endl;
  char sidx[10];//, url[200];
  line = extractStringFromQuotes(line, sidx, sizeof(sidx));
  LOGGER << F("Mqtt line:") << line << endl;
  LOGGER.flush();
  //delay(100);
  //line = extractStringFromQuotes(line, url, sizeof(url));
  if (sidx[0] == 0 || line[0] == 0) {
    LOGGER << F("Command not recognized");
    return;
  }
  int idx = atoi(sidx);
  PropertyList.putArrayProperty(F("mqtt_msg_arr"), idx, line);
}

void MQTTDest::cmdCleanCustomUrl(const char *line) {
  PropertyList.removeArrayProperty(F("mqtt_msg_arr"));
}

void MQTTDest::cmdCallMqtt(const char *line) {
  mqttDest.cmdCallMqttInst(line);
}

void MQTTDest::cmdCallMqttInst(const char *line) {
  char *c = strchr(line, ' ');
  char *x = strchr(c+1, ' ');
  bool retain = strstr(line, "call_mqttr") == line;
  *x = 0;
  String topic = c+1;
  String msg = x+1;
  LOGGER << F("Will send mqtt to:") << topic <<":[" << msg << "]" << endl;
  LOGGER.flush();
  if (WifiStuff.waitForWifi() != WL_CONNECTED) return;

  if (!mqttStart()) {
    mqttEnd(false);
    return;
  }
  bool res = client->publish(topic.c_str(), msg.c_str(), retain);
  mqttEnd(res);
}



bool MQTTDest::process(LinkedList<Pair *> &data) {
  LOGGER << F("MQTTDest::process") << endl;
  LOGGER.flush();
//  heap("");
  if (mqttListen) {
    if (!client ) {
      if (!mqttStart()) {
        mqttEnd(false);
      }
    }
  }
  String s = PropertyList.getArrayProperty(F("mqtt_msg_arr"), 0);
  if (!s.length()) return true;
  if (WifiStuff.waitForWifi() != WL_CONNECTED) return false;

  if (!mqttStart()) {
    mqttEnd(false);
    return false;
  }
  //int i=0;
  String mqttTopic = F("vair");//  = PropertyList.readProperty(EE_MQTT_TOPIC);
  bool status = true;
  for (int i=0; ; i++) {
    s = PropertyList.getArrayProperty(F("mqtt_msg_arr"), i);
    if (!s.length()) break;
    replaceValuesInURL(data, s);
    if (hasPlaceholders(s)) {
      status = false;
      //mqttEnd(false);
      continue;
    }
    if (s.indexOf(':') > -1) {
      mqttTopic = s.substring(0,s.indexOf(':'));
      s = s.substring(s.indexOf(':') + 1);
    }
    LOGGER << F("Mqtt Dest: sending: to topic:") << mqttTopic << F(", msg: ") << s << endl;
    LOGGER.flush();
    if(!client->publish(mqttTopic.c_str(), s.c_str())) {
      mqttEnd(false);
      return false;
    }
  }
  mqttEnd(true);
  return status;
  //heap("");
}

bool MQTTDest::reconnect() {
  if (!client->connected()) {
    isListening = false;
    String mqttServer, mqttClient, mqttUser, mqttPass;
    mqttClient = PropertyList.readProperty(EE_MQTT_CLIENT);
    mqttUser   = PropertyList.readProperty(EE_MQTT_USER);
    mqttPass   = PropertyList.readProperty(EE_MQTT_PASS);
    if (!mqttClient.length()) mqttClient = F("vESPrino");

    LOGGER << F("Connecting to server :") << mqttClient << "," << mqttUser << "," << mqttPass << endl;
    LOGGER.flush();
    if (mqttUser.length() > 0) client->connect(mqttClient.c_str(), mqttUser.c_str(), mqttPass.c_str());
    else client->connect(mqttClient.c_str());
  }
  if (client->connected()) {
    LOGGER << F("MQTT Connected") << endl;
    setupMqttListen();
    return true;
  } else {
    LOGGER.println(F("Could not connect to MQTT server. Will retry on next iteration"));
    return false;
  }
}

bool MQTTDest::mqttStart() {
  if (WifiStuff.waitForWifi() != WL_CONNECTED) {
    if (DEBUG) LOGGER << F("MQTT Dest: Cannot send while wifi offline\n");
    return false;
  }
  if (client) return reconnect();
  String mqttServer;
  long mqttPort;
  mqttServer = PropertyList.readProperty(EE_MQTT_SERVER);
  mqttPort   = PropertyList.readLongProperty(EE_MQTT_PORT);
  if (!mqttServer.length()) return false;
  LOGGER << F("Sending via MQTT: ");
  LOGGER.flush();
  LOGGER << mqttServer << "," << mqttPort << endl;
  LOGGER.flush();
  uint32_t st = millis();
  wclient = new WiFiClient();
  client = new PubSubClient(mqttServer.c_str(), mqttPort, MQTTDest::onReceive, *wclient);
  isListening = false;
  return reconnect();
}

void MQTTDest::mqttEnd(bool res) {
  if (DEBUG) LOGGER.println(res ? F("CLOSED") : F("Failed!"));
  if (isListening) return;
  if (client) {
    client->disconnect();
    delete client;
  }
  if (wclient) {
    delete wclient;
  }

  client = NULL;
  wclient = NULL;
}

void MQTTDest::replaceValuesInURL(LinkedList<Pair *> &data, String &s) {
  // LOGGER << "MQTTDest::replaceUrl = " << s << endl;
  // for (int i=0; i < data.size(); i++) {
  //   LOGGER << data.get(i)->key << " = " << data.get(i)->value << "."<<endl;
  // }
  for (int i=0; i < data.size(); i++) {
    Pair *p = data.get(i);
    String skey = String("%") + p->key + String("%");
    s.replace(skey, String(p->value));
    //LOGGER << "after replace: key << " << skey << "." << p->value<< "." << String(p->value) << " " << s << endl;
  }
}

void MQTTDest::onReceive(char* topic1, byte* payload1, unsigned int length) {
  char bPayload[360];
  strncpy(bPayload, (char*)payload1, _min(length, sizeof(bPayload)-1));
  bPayload[length] = 0;
  String payload = String((char*)bPayload), topic = String(topic1);
  LOGGER << F("MQTT Inbound") << topic << " => " << payload.length() << "[" << payload << "]" << endl;
  if (topic.indexOf("/cmd") && payload.length() > 0) {
    String cmd = String(F("call_mqttr ")) + topic + " ";
    menuHandler.scheduleCommand(payload.c_str());
    menuHandler.scheduleCommand(cmd.c_str());
  }
}


void MQTTDest::loop() {
  //Serial << ".";
  if (client != NULL) {
    if (client->connected()) client->loop();
    else {
      isListening = false;
      mqttEnd(false);
    }
  }
}
