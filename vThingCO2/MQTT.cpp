#include "common.hpp"

#include <PubSubClient.h>
#include "plugins/PropertyList.hpp"


String   mqttServer; //= "m20.cloudmqtt.com";
uint32_t mqttPort   ;//= 19749;
String   mqttClient ;//= "vAir_CO2_Monitor";
String   mqttUser   ;//= "pndhubpk";
String   mqttPass   ;//= "yfT7ax_KDrgG";
String   mqttTopic  ;//= "co2Value";

//String   mqttServer = "m20.cloudmqtt.com";
//uint32_t mqttPort   = 19749;
//String   mqttClient = "vAir_CO2_Monitor";
//String   mqttUser   = "pndhubpk";
//String   mqttPass   = "yfT7ax_KDrgG";
//String   mqttTopic  = "co2Value";
//cfg_mqtt "m20.cloudmqtt.com", "19749", "vAir_CO2_Monitor", "pndhubpk", "yfT7ax_KDrgG", "co2Value"
//cfg_mqtt "mqtt.beebotte.com", "1883", "vAir_CO2_Monitor", "token:1455209083269_09xjatPXToF8bP63", "", "vThingTest/co2"
//cfg_mqval {"data": %s, "write": true, "ispublic": true}
// wifi "SAP-Guest","63597688","w6QQckVn"
// wifi "vladiHome", "0888414447"
//atest_mqtt

void configMQTT(const char *p) {
  char mqttServer[30], mqttPortS[6], mqttClient[20], mqttUser[45], mqttPass[15], mqttTopic[40];
  p = extractStringFromQuotes(p, mqttServer, sizeof(mqttServer));
  p = extractStringFromQuotes(p, mqttPortS,   sizeof(mqttPortS));
  p = extractStringFromQuotes(p, mqttClient, sizeof(mqttClient));
  p = extractStringFromQuotes(p, mqttUser,   sizeof(mqttUser));
  p = extractStringFromQuotes(p, mqttPass,   sizeof(mqttPass));
  p = extractStringFromQuotes(p, mqttTopic,  sizeof(mqttTopic));
  if (mqttClient[0] == 0) strcpy(mqttTopic, "vThing");
  if (mqttTopic[0] == 0) {
    #ifdef VTHING_H801_LED
      strcpy(mqttTopic, "vThingH801");
    #else
      strcpy(mqttTopic, "vThing/data");
    #endif
  }

  PropertyList.putProperty(EE_MQTT_SERVER, mqttServer);
  PropertyList.putProperty(EE_MQTT_SERVER, mqttServer);
  PropertyList.putProperty(EE_MQTT_PORT, mqttPortS);
  PropertyList.putProperty(EE_MQTT_CLIENT, mqttClient);
  PropertyList.putProperty(EE_MQTT_USER, mqttUser);
  PropertyList.putProperty(EE_MQTT_PASS, mqttPass);
  PropertyList.putProperty(EE_MQTT_TOPIC, mqttTopic);

  EEPROM.commit();
  SERIAL << F("MQTT Configuration Stored") << endl;
  SERIAL << mqttServer << "," << mqttPortS << "," << mqttClient << "," << mqttUser << "," << mqttPass << "," << mqttTopic << endl;
  SERIAL << F("DONE") << endl;
#ifdef VTHING_H801_LED
  h801_mqtt_connect();
#endif

}

void sendMQTT(String msg) {
  String mqttServer, mqttClient, mqttUser, mqttPass, mqttTopic, mqttValue;
  long mqttPort;
  mqttServer = PropertyList.readProperty(EE_MQTT_SERVER);
  mqttPort   = PropertyList.readLongProperty(EE_MQTT_PORT);
  mqttClient = PropertyList.readProperty(EE_MQTT_CLIENT);
  mqttUser   = PropertyList.readProperty(EE_MQTT_USER);
  mqttPass   = PropertyList.readProperty(EE_MQTT_PASS);
  mqttTopic  = PropertyList.readProperty(EE_MQTT_TOPIC);
  mqttValue  = PropertyList.readProperty(EE_MQTT_VALUE);
  SERIAL << "Sending via MQTT: ";
  delay(150);
  SERIAL << mqttServer << "," << mqttPort << "," << mqttClient << "," ;
  delay(150);
  SERIAL << mqttUser;
  delay(150);
  SERIAL << "," << mqttPass << ",";
  delay(150);
  SERIAL << mqttTopic;
  delay(150);
  SERIAL << ",";
  delay(150);
  SERIAL << mqttValue << endl;
  delay(150);
  uint32_t st = millis();
  WiFiClient wclient;
//  PubSubClient client(wclient, mqttServer, mqttPort);
  PubSubClient client(mqttServer.c_str(), mqttPort, wclient);
  if (WiFi.status() == WL_CONNECTED) {
    //MQTT::Connect conn = MQTT::Connect(mqttClient);
//    if (strlen(mqttUser) > 0) conn.set_auth(mqttUser, mqttPass);
//    if (client.connect(conn)) {
    boolean res;
    if (mqttUser.length() > 0) res = client.connect(mqttClient.c_str(), mqttUser.c_str(), mqttPass.c_str());
    else res = client.connect(mqttClient.c_str());
    if (res) {
      //char ddd[50];
      //sprintf(ddd, "{\"data\": %s, \"write\": true,  \"ispublic\": true}", msg.c_str());
      char mqttValue2[80];
      sprintf(mqttValue2, mqttValue.c_str(), msg.c_str());
      SERIAL << F("Connected. Will publish: ") << mqttValue2 << endl;
      boolean res = client.publish(mqttTopic.c_str(), mqttValue2);
      SERIAL.println(res ? F("CLOSED") : F("Failed!"));
    } else {
        SERIAL.println(F("Could not connect to MQTT server"));
    }
  }
  client.disconnect();
  SERIAL << "sent in:" << (millis() - st) << endl;
}

void configMQTTVal(const char *line) {
  PropertyList.putProperty(EE_MQTT_VALUE, &line[10]);
  SERIAL << "DONE" << endl;
}

void configMQTTTest(const char *ignore) {
  sendMQTT("556");
}

void MQTT_RegisterCommands(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("cfg_mqtt"),  CMD_BEGIN, configMQTT, F("")));
  handler->registerCommand(new MenuEntry(F("cfg_mqval"), CMD_BEGIN, configMQTTVal, F("")));
  handler->registerCommand(new MenuEntry(F("atest_mqtt"), CMD_EXACT, configMQTTTest, F("")));
}
