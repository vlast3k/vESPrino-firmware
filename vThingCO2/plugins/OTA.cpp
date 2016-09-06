#include <Arduino.h>

#include <ESP8266httpUpdate.h>
#include "common.hpp"


void autoUpdateIfForced(const char *ignore);
String getForceUpdateBuild();

void doHttpUpdate(int mode, const char *url) {
  if (!url) {
    #ifdef VTHING_CO2
      url = "http://anker-bg.com/vlast3k/vthing_co2/latest.bin";
    #elif defined(VTHING_STARTER)
      url = "http://anker-bg.com/vlast3k/vthing_starter/latest.bin";
    #elif defined(VTHING_VESPRINO)
      url = "http://anker-bg.com/vlast3k/vesprino/latest.bin";
    #elif defined(VTHING_H801_LED)
      url = "http://anker-bg.com/vlast3k/h801/latest.bin";
    #elif defined(VAIR)
      url = "http://anker-bg.com/vlast3k/vair/latest.bin";
    #endif
  }

  SERIAL_PORT << F("Will update from: ") << url << endl;

  menuHandler.handleCommand("ledcolor red");
  menuHandler.handleCommand("ledbrg 80");
  t_httpUpdate_return ret;
  ret= ESPhttpUpdate.update(url);

  switch(ret) {
    case HTTP_UPDATE_FAILED:
      SERIAL_PORT.println(ESPhttpUpdate.getLastErrorString());
      SERIAL_PORT.println(F("HTTP_UPDATE_FAILED"));
      break;

    case HTTP_UPDATE_NO_UPDATES:
      SERIAL_PORT.println(F("HTTP_UPDATE_NO_UPDATES"));
      break;

    case HTTP_UPDATE_OK:
      SERIAL_PORT.println(F("HTTP_UPDATE_OK"));
      break;
  }
}

void ota_otahtest(const char* line) {
  doHttpUpdate(2, NULL);
}

void ota_otah(const char* line) {
  doHttpUpdate(1, NULL);
}

void ota_otau(const char* line) {
  doHttpUpdate(0, &line[5]);
}

void OTA_registerCommands(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("otahtest"), CMD_BEGIN, ota_otahtest, F("HCP Cfg 1")));
  handler->registerCommand(new MenuEntry(F("otah"), CMD_EXACT, ota_otah, F("HCP Cfg 2")));
  handler->registerCommand(new MenuEntry(F("otau"), CMD_BEGIN, ota_otau, F("HCP Cfg 2")));
  handler->registerCommand(new MenuEntry(F("fupd"), CMD_EXACT, autoUpdateIfForced, F("HCP Cfg 2")));
}

String getForceUpdateBuild() {

    HTTPClient http;
    String payload;
    Serial.print("[HTTP] begin...\n");
    // configure traged server and url
    //http.begin("https://192.168.1.12/test.html", "7a 9c f4 db 40 d3 62 5a 6e 21 bc 5c cc 66 c8 3e a1 45 59 38"); //HTTPS
    http.begin("http://anker-bg.com/vlast3k/vesprino/forced.txt"); //HTTP

    Serial.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if(httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);

        // file found at server
        if(httpCode == HTTP_CODE_OK) {
            payload = http.getString();
            Serial.println(payload);
        }
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    http.end();
    return payload;
}

void autoUpdateIfForced(const char *ignore) {
  if (waitForWifi(10000) != WL_CONNECTED) return;
  String forcedUpdate = getForceUpdateBuild();
  if (!forcedUpdate.length()) return;
  Serial << "OTA: current Build: " << atol(BUILD_NUM) << endl;
  Serial << "OTA: forced  Build: " << atol(forcedUpdate.c_str()) << endl;
  if (atol(BUILD_NUM) >= atol(forcedUpdate.c_str())) return;
  String url = "http://anker-bg.com/vlast3k/vesprino/firmware";
  url += forcedUpdate + ".bin";
  doHttpUpdate(0, url.c_str());
}
