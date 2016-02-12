#include <PubSubClient.h>

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

void configMQTTVal(const char *p) {
   
}
void configMQTT(const char *p) {
  char mqttServer[30], mqttPortS[6], mqttClient[20], mqttUser[45], mqttPass[15], mqttTopic[40];
  p = extractStringFromQuotes(p, mqttServer, sizeof(mqttServer)); 
  p = extractStringFromQuotes(p, mqttPortS,   sizeof(mqttPortS)); 
  p = extractStringFromQuotes(p, mqttClient, sizeof(mqttClient)); 
  p = extractStringFromQuotes(p, mqttUser,   sizeof(mqttUser)); 
  p = extractStringFromQuotes(p, mqttPass,   sizeof(mqttPass)); 
  p = extractStringFromQuotes(p, mqttTopic,  sizeof(mqttTopic)); 
  Serial << "port: " << atol(mqttPortS) << endl;
  EEPROM.put(EE_MQTT_SERVER_30B, mqttServer);
  EEPROM.put(EE_MQTT_PORT_4B, atol(mqttPortS));
  EEPROM.put(EE_MQTT_CLIENT_20B, mqttClient);
  EEPROM.put(EE_MQTT_USER_45B, mqttUser);
  EEPROM.put(EE_MQTT_PASS_15B, mqttPass);
  EEPROM.put(EE_MQTT_TOPIC_40B, mqttTopic);
  EEPROM.commit();
  Serial << "MQTT Configuration Stored" << endl;
  Serial << mqttServer << "," << mqttPortS << "," << mqttClient << "," << mqttUser << "," << mqttPass << "," << mqttTopic << endl;
  Serial << "DONE" << endl;
  
}

void sendMQTT(String msg) {
  char mqttServer[30], mqttClient[20], mqttUser[45], mqttPass[15], mqttTopic[40], mqttValue[70];
  long mqttPort;
  EEPROM.get(EE_MQTT_SERVER_30B, mqttServer);
  EEPROM.get(EE_MQTT_PORT_4B,    mqttPort);
  EEPROM.get(EE_MQTT_CLIENT_20B, mqttClient);
  EEPROM.get(EE_MQTT_USER_45B,   mqttUser);
  EEPROM.get(EE_MQTT_PASS_15B,   mqttPass);
  EEPROM.get(EE_MQTT_TOPIC_40B,  mqttTopic);
  EEPROM.get(EE_MQTT_VALUE_70B,  mqttValue);
  Serial << "MQTT Configuration: " << endl;
  Serial << mqttServer << "," << mqttPort << "," << mqttClient << "," << mqttUser << "," << mqttPass << "," << mqttTopic << "," << mqttValue << endl;
  uint32_t st = millis();
  WiFiClient wclient;
  PubSubClient client(wclient, mqttServer, mqttPort);
  if (WiFi.status() == WL_CONNECTED) {
    MQTT::Connect conn = MQTT::Connect(mqttClient);
    if (strlen(mqttUser) > 0) conn.set_auth(mqttUser, mqttPass);
    if (client.connect(conn)) {
      //char ddd[50];
      //sprintf(ddd, "{\"data\": %s, \"write\": true,  \"ispublic\": true}", msg.c_str());
      char mqttValue2[80];
      sprintf(mqttValue2, mqttValue, msg.c_str());
      Serial << mqttValue2 << endl;
      client.publish(mqttTopic, mqttValue2);
      Serial.println("sent");
    } else {
        Serial.println("Could not connect to MQTT server");   
    }     
  }
  client.disconnect();
  Serial << "sent in:" << (millis() - st) << endl;
}

