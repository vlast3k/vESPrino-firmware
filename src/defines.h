#define PROP_ITERATION_DURATION F("pwr.iterdur")
#define PROP_TIMEOUT_INTERVAL F("pwr.timeoutint")
#define BUILD_NUM "20170221" 
//#define HARDCODED_SENSORS "CM110x,SI7021,"
//#define HARDCODED_SENSORS "none,"


//#define SERIAL_PORT LOGGER
#define EE_WIFI_SSID F("wifi.ssid")
#define EE_WIFI_P1   F("wifi.p1")
#define EE_WIFI_P2   F("wifi.p2")
#define EE_IOT_HOST  F("sap.hcp.host")
#define EE_IOT_PATH  F("sap.hcp.path")
#define EE_IOT_TOKN  F("sap.hcp.token")
#define EE_GENIOT_PATH  F("custom_url.path")
#define EE_MQTT_SERVER  F("mqtt.server")
#define EE_MQTT_PORT    F("mqtt.port")
#define EE_MQTT_CLIENT  F("mqtt.client")
#define EE_MQTT_USER    F("mqtt.user")
#define EE_MQTT_PASS    F("mqtt.pass")
#define EE_MQTT_TOPIC   F("mqtt.topic")
#define EE_MQTT_VALUE   F("mqtt.value")
#define EE_RESET_CO2_1B 1



#define PROP_SND_INT      F("send.interval")
//#define PROP_SND_ITER     F("send.iterations")
#define PROP_SND_THR      F("cubic.co2.threshold")
#define PROP_H801_API_KEY F("h801.api_key")
#define PROP_DEBUG        F("debug")
#define PROP_SERVO_PORT F("servo.port")
#define PROP_I2C_DISABLED_PORTS F("i2c.disabled.ports")
#define PROP_TEMP_ADJ F("temp.adjustment")


#define LED_START         F("1lnlnd")
#define LED_START_DS      F("1lnlnlnd")
#define LED_NO_I2C        F("rlrlrld")
#define LED_LOST_I2C      F("ryryryd")
#define LED_WIFI_SEARCH   F("cb")
#define LED_WIFI_FOUND    F("cncncnd")
#define LED_WIFI_FAILED   F("rcrcrcnd")
#define LED_SEND_OK       F("ngngngd")
#define LED_SEND_FAILED   F("nrnrnrd")
#define LED_GO_DEEPSLEEP
// #define EE_WIFI_SSID_30B 0
// #define EE_WIFI_P1_30B 30
// #define EE_WIFI_P2_30B 60
#define EE_IOT_HOST_60B 90
#define EE_IOT_PATH_140B 150
#define EE_IOT_TOKN_40B 290
// #define EE_GENIOT_PATH_140B 330
// #define EE_MQTT_SERVER_30B  470
// #define EE_MQTT_PORT_4B     500
// #define EE_MQTT_CLIENT_20B  504
// #define EE_MQTT_USER_45B    524
// #define EE_MQTT_PASS_15B    569
// #define EE_MQTT_TOPIC_40B   584
// #define EE_MQTT_VALUE_70B   624
// #define EE_1B_RESET_CO2     694
// //#define EE_LAST 695
#define EE_JSON_CFG_1000B   1000

#define SAP_IOT_HOST "spHst"
#define SAP_IOT_DEVID "spDvId"
#define SAP_IOT_TOKEN "spTok"
#define SAP_IOT_BTN_MSGID "spBtMID"

//#define PERF(str) { LOGGER << str << ":" << millis() << endl; LOGGER.flush();delay(100);}
#define PERF(str)
//#define H801_API_KEY "h801key"
//#define XX_SND_INT  "xxSndInt"
//#define XX_SND_THR  "xxSndThr"

#define HTTP_STR  "http://"
#define HTTPS_STR "https://"
