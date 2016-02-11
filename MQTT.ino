#include <PubSubClient.h>

String   mqttServer = "m20.cloudmqtt.com";
uint32_t mqttPort   = 19749;
String   mqttClient = "vAir_CO2_Monitor";
String   mqttUser   = "pndhubpk";
String   mqttPass   = "yfT7ax_KDrgG";
String   mqttTopic  = "co2Value";

void sendMQTT(String msg) {
  uint32_t st = millis();
  WiFiClient wclient;
  PubSubClient client(wclient, mqttServer, mqttPort);
  if (WiFi.status() == WL_CONNECTED) {
    MQTT::Connect conn = MQTT::Connect(mqttClient);
    if (mqttUser.length() > 0) conn.set_auth(mqttUser, mqttPass);
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

