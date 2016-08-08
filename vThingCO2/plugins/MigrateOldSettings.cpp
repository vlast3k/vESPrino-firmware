#include "PropertyList.hpp"
#include "EEPROM.h"
#include "Arduino.h"
#include "common.hpp"
#define EE_WIFI_SSID_30B 0
#define EE_WIFI_P1_30B 30
#define EE_WIFI_P2_30B 60
#define EE_IOT_HOST_60B 90
#define EE_IOT_PATH_140B 150
#define EE_IOT_TOKN_40B 290
#define EE_GENIOT_PATH_140B 330
#define EE_MQTT_SERVER_30B  470
#define EE_MQTT_PORT_4B     500
#define EE_MQTT_CLIENT_20B  504
#define EE_MQTT_USER_45B    524
#define EE_MQTT_PASS_15B    569
#define EE_MQTT_TOPIC_40B   584
#define EE_MQTT_VALUE_70B   624
//#define EE_RESET_CO2_1B     694


//#define PROP_H801_API_KEY F("h801.api_keyddddddddddddddddddddd")

#define H801_API_KEY F("h801key")
#define XX_SND_INT   F("xxSndInt")
#define XX_SND_THR   F("xxSndThr")

void _migrateEESetting(int eeAddr, const __FlashStringHelper *key) {
  char buf[200];
  EEPROM.get(eeAddr, buf);
  PropertyList.putProperty(key, buf);
}

void _migrateJsonSetting(const __FlashStringHelper *keyJson, const __FlashStringHelper *keyProp) {
  char buf[200];
  getJSONConfig(String(keyJson).c_str(), buf);
  PropertyList.putProperty(keyProp, buf);
}

void MigrateSettingsIfNeeded() {
  if (PropertyList.readBoolProperty("isMigrated")) return;

  EEPROM.begin(3000);
  _migrateEESetting(EE_WIFI_SSID_30B, EE_WIFI_SSID);
  _migrateEESetting(EE_WIFI_P1_30B, EE_WIFI_P1);
  _migrateEESetting(EE_WIFI_P2_30B, EE_WIFI_P2);
  _migrateEESetting(EE_IOT_HOST_60B, EE_IOT_HOST);
  _migrateEESetting(EE_IOT_PATH_140B, EE_IOT_PATH);
  _migrateEESetting(EE_IOT_TOKN_40B, EE_IOT_TOKN);
  _migrateEESetting(EE_GENIOT_PATH_140B, EE_GENIOT_PATH);
  _migrateEESetting(EE_MQTT_SERVER_30B, EE_MQTT_SERVER);
  _migrateEESetting(EE_MQTT_PORT_4B, EE_MQTT_PORT);
  _migrateEESetting(EE_MQTT_CLIENT_20B, EE_MQTT_CLIENT);
  _migrateEESetting(EE_MQTT_USER_45B, EE_MQTT_USER);
  _migrateEESetting(EE_MQTT_PASS_15B, EE_MQTT_PASS);
  _migrateEESetting(EE_MQTT_TOPIC_40B, EE_MQTT_TOPIC);
  _migrateEESetting(EE_MQTT_VALUE_70B, EE_MQTT_VALUE);
  
  _migrateJsonSetting(XX_SND_INT, PROP_SND_INT);
  _migrateJsonSetting(XX_SND_THR, PROP_SND_THR);
  _migrateJsonSetting(H801_API_KEY, PROP_H801_API_KEY);

  EEPROM.end();
}
