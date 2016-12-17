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

extern MQTTDest mqttDest;
#define PROP_MQTT_SUBS_TOPIC F("mqtt.subs.topic")
#define PROP_MQTT_LISTEN F("mqtt.listen")

MQTTDest::MQTTDest() {
  registerDestination(this);
}

void MQTTDest::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("mqtt_setup"), CMD_BEGIN, &MQTTDest::cmdMqttSetup, F("mqtt_setup \"idx\"value")));
  handler->registerCommand(new MenuEntry(F("mqtt_msg_add"), CMD_BEGIN, &MQTTDest::cmdMqttMsgAdd, F("mqtt_msg_add \"idx\"value")));
  handler->registerCommand(new MenuEntry(F("call_mqtt"), CMD_BEGIN, &MQTTDest::cmdCallMqtt, F("call_mqtt topic message")));
  handler->registerCommand(new MenuEntry(F("mqtt_msg_clean"), CMD_EXACT, &MQTTDest::cmdCleanCustomUrl, F("mqtt_msg_clean - clean all mqtt messages")));
  setupMqttListen();
}

void MQTTDest::setupMqttListen() {
  if (!PropertyList.hasProperty(PROP_MQTT_LISTEN)) return;
  String s = PropertyList.readProperty(PROP_MQTT_SUBS_TOPIC);
  if (!s.length()) return;
  if (waitForWifi() != WL_CONNECTED) return;
  if (!mqttStart()) {
    mqttEnd(false);
    return;
  }
  s += "/cmd";
  client->subscribe(s.c_str());
  Serial << F("Accepting commands via MQTT on topic: ") << s;
  isListening = true;
}

void MQTTDest::cmdMqttSetup(const char *p) {
  char mqttServer[30], mqttPortS[6], mqttClient[20], mqttUser[45], mqttPass[15], mqttTopic[40];
  p = extractStringFromQuotes(p, mqttServer, sizeof(mqttServer));
  p = extractStringFromQuotes(p, mqttPortS,   sizeof(mqttPortS));
  p = extractStringFromQuotes(p, mqttClient, sizeof(mqttClient));
  p = extractStringFromQuotes(p, mqttUser,   sizeof(mqttUser));
  p = extractStringFromQuotes(p, mqttPass,   sizeof(mqttPass));
//  p = extractStringFromQuotes(p, mqttTopic,  sizeof(mqttTopic));
  //Serial << "Gere" << endl;
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

  SERIAL_PORT << F("MQTT Configuration Stored") << endl;
  SERIAL_PORT << mqttServer << "," << mqttPortS << "," << mqttClient << "," << mqttUser << "," << mqttPass << endl;
  SERIAL_PORT << F("DONE") << endl;
  Serial.flush();
#ifdef VTHING_H801_LED
  h801_mqtt_connect();
#endif
}

void MQTTDest::cmdMqttMsgAdd(const char *line) {
  //Serial << "menuAddCustomUrl" << endl;
  char sidx[10];//, url[200];
  line = extractStringFromQuotes(line, sidx, sizeof(sidx));
  Serial << "Mqtt line:" << line << endl;
  Serial.flush();
  //delay(100);
  //line = extractStringFromQuotes(line, url, sizeof(url));
  if (sidx[0] == 0 || line[0] == 0) {
    Serial << F("Command not recognized");
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
  *x = 0;
  String topic = c+1;
  String msg = x+1;
  Serial << F("Will send mqtt to:") << topic <<":" << msg << endl;
  Serial.flush();
  if (waitForWifi() != WL_CONNECTED) return;

  if (!mqttStart()) {
    mqttEnd(false);
    return;
  }
  bool res = client->publish(topic.c_str(), msg.c_str());
  mqttEnd(res);
}



void MQTTDest::process(LinkedList<Pair *> &data) {
  Serial << F("MQTTDest::process") << endl;
  Serial.flush();
//  heap("");
  String s = PropertyList.getArrayProperty(F("mqtt_msg_arr"), 0);
  if (!s.length()) return;
  if (waitForWifi() != WL_CONNECTED) return;

  if (!mqttStart()) {
    mqttEnd(false);
    return;
  }
  //int i=0;
  String mqttTopic = "vair";//  = PropertyList.readProperty(EE_MQTT_TOPIC);
  for (int i=0; ; i++) {
    s = PropertyList.getArrayProperty(F("mqtt_msg_arr"), i);
    if (!s.length()) break;
    replaceValuesInURL(data, s);
    if (hasPlaceholders(s)) return;    
    if (s.indexOf(':') > -1) {
      mqttTopic = s.substring(0,s.indexOf(':'));
      s = s.substring(s.indexOf(':') + 1);
    }
    Serial << F("Mqtt Dest: sending: to topic:") << mqttTopic << ", msg: " << s << endl;
    Serial.flush();
    if(!client->publish(mqttTopic.c_str(), s.c_str())) {
      mqttEnd(false);
      return;
    }
  }
  mqttEnd(true);
  //heap("");
}

bool MQTTDest::mqttStart() {
  if (client) return true;
  if (WiFi.status() != WL_CONNECTED) {
    if (DEBUG) Serial << F("MQTT Dest: Cannot send while wifi offline\n");
    return false;
  }
  String mqttServer, mqttClient, mqttUser, mqttPass;
  long mqttPort;
  mqttServer = PropertyList.readProperty(EE_MQTT_SERVER);
  mqttPort   = PropertyList.readLongProperty(EE_MQTT_PORT);
  mqttClient = PropertyList.readProperty(EE_MQTT_CLIENT);
  mqttUser   = PropertyList.readProperty(EE_MQTT_USER);
  mqttPass   = PropertyList.readProperty(EE_MQTT_PASS);
  if (!mqttServer.length()) return false;
  if (!mqttClient.length()) mqttClient = "vESPrino";
   SERIAL_PORT << "Sending via MQTT: ";
   Serial.flush();
   SERIAL_PORT << mqttServer << "," << mqttPort << "," << mqttClient << "," ;
   Serial.flush();
   SERIAL_PORT << mqttUser;
   Serial.flush();
   SERIAL_PORT << "," << mqttPass << endl;
   Serial.flush();
  uint32_t st = millis();
  wclient = new WiFiClient();
//  PubSubClient client(wclient, mqttServer, mqttPort);
  client = new PubSubClient(mqttServer.c_str(), mqttPort, MQTTDest::onReceive, *wclient);
  bool res;
  if (mqttUser.length() > 0) res = client->connect(mqttClient.c_str(), mqttUser.c_str(), mqttPass.c_str());
  else res = client->connect(mqttClient.c_str());
  if (!res) SERIAL_PORT.println(F("Could not connect to MQTT server"));
  return res;
}

void MQTTDest::mqttEnd(bool res) {
  if (DEBUG) SERIAL_PORT.println(res ? F("CLOSED") : F("Failed!"));
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
  // Serial << "MQTTDest::replaceUrl = " << s << endl;
  // for (int i=0; i < data.size(); i++) {
  //   Serial << data.get(i)->key << " = " << data.get(i)->value << "."<<endl;
  // }
  for (int i=0; i < data.size(); i++) {
    Pair *p = data.get(i);
    String skey = String("%") + p->key + String("%");
    s.replace(skey, String(p->value));
    //Serial << "after replace: key << " << skey << "." << p->value<< "." << String(p->value) << " " << s << endl;
  }
}

// bool MQTTDest::invokeURL(String &url) {
//   SERIAL_PORT << F("Connected. Will publish: ") << url << endl;
//   return client.publish(mqttTopic.c_str(), url.c_str());
// }

void MQTTDest::onReceive(char* topic1, byte* payload1, unsigned int length) {
  char bPayload[360];
  strncpy(bPayload, (char*)payload1, _min(length, sizeof(bPayload)-1));
  bPayload[length] = 0;
  String payload = String((char*)bPayload), topic = String(topic1);
  SERIAL_PORT << topic << " => " << payload << endl;
  if (topic.indexOf("/cmd")) menuHandler.scheduleCommand(payload.c_str());
  // else if (topic.indexOf("/Color") > -1) processTriplet(payload);
  // else if (topic.indexOf("/W1")   > -1) analogWriteColor(w1PIN, payload.toInt());
  // else if (topic.indexOf("/W2")   > -1) analogWriteColor(w2PIN, payload.toInt());
  // else if (topic.indexOf("/Save") > -1) onH801LEDStoreData();
  //publishMQTTStatus();
}
