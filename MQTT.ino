#include <PubSubClient.h>

//String   mqttServer = "m20.cloudmqtt.com";
//uint32_t mqttPort   = 19749;
//String   mqttClient = "vAir_CO2_Monitor";
//String   mqttUser   = "pndhubpk";
//String   mqttPass   = "yfT7ax_KDrgG";
//String   mqttTopic  = "co2Value";
//cfg_mqtt "m20.cloudmqtt.com", "19749", "vAir_CO2_Monitor", "pndhubpk", "yfT7ax_KDrgG", "co2Value"


void configMQTT(const char *p) {
  char mqttServer[30], mqttPortS[6], mqttClient[20], mqttUser[15], mqttPass[15], mqttTopic[40];
  p = extractStringFromQuotes(p, mqttServer, sizeof(mqttServer)); 
  p = extractStringFromQuotes(p, mqttPortS,   sizeof(mqttPortS)); 
  p = extractStringFromQuotes(p, mqttClient, sizeof(mqttClient)); 
  p = extractStringFromQuotes(p, mqttUser,   sizeof(mqttUser)); 
  p = extractStringFromQuotes(p, mqttPass,   sizeof(mqttPass)); 
  p = extractStringFromQuotes(p, mqttTopic,  sizeof(mqttTopic)); 

  EEPROM.put(EE_MQTT_SERVER_30B, mqttServer);
  EEPROM.put(EE_MQTT_PORT_4B, long(mqttPortS));
  EEPROM.put(EE_MQTT_CLIENT_20B, mqttClient);
  EEPROM.put(EE_MQTT_USER_15B, mqttUser);
  EEPROM.put(EE_MQTT_PASS_15B, mqttPass);
  EEPROM.put(EE_MQTT_TOPIC_40B, mqttTopic);
  EEPROM.commit();
  Serial << "MQTT Configuration Stored" << endl;
  Serial << mqttServer << "," << mqttPortS << "," << mqttClient << "," << mqttUser << "," << mqttPass << "," << mqttTopic << endl;
  
}

void sendMQTT(String msg) {
  char mqttServer[30], mqttClient[20], mqttUser[15], mqttPass[15], mqttTopic[40];
  uint32_t mqttPort;
  EEPROM.get(EE_MQTT_SERVER_30B, mqttServer);
  EEPROM.get(EE_MQTT_PORT_4B,    mqttPort);
  EEPROM.get(EE_MQTT_CLIENT_20B, mqttClient);
  EEPROM.get(EE_MQTT_USER_15B,   mqttUser);
  EEPROM.get(EE_MQTT_PASS_15B,   mqttPass);
  EEPROM.get(EE_MQTT_TOPIC_40B,  mqttTopic);
  Serial << "MQTT Configuration: " << endl;
  Serial << mqttServer << "," << mqttPort << "," << mqttClient << "," << mqttUser << "," << mqttPass << "," << mqttTopic << endl;
  
  uint32_t st = millis();
  WiFiClient wclient;
  PubSubClient client(wclient, mqttServer, mqttPort);
  if (WiFi.status() == WL_CONNECTED) {
    MQTT::Connect conn = MQTT::Connect(mqttClient);
    if (strlen(mqttUser) > 0) conn.set_auth(mqttUser, mqttPass);
    if (client.connect(conn)) {
      client.publish(mqttTopic, msg);
      Serial.println("sent");
    } else {
        Serial.println("Could not connect to MQTT server");   
    }     
  }
  client.disconnect();
  Serial << "sent in:" << (millis() - st) << endl;
}

