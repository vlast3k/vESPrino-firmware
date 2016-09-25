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

String getHTTPFile(String url) {
    HTTPClient http;
    String payload;
  //  Serial << "Retriving HTTP: " << url << endl;
    http.begin(url); //HTTP
    int httpCode = http.GET();
    if(httpCode > 0) {
        if (DEBUG) Serial << F("[HTTP] GET... code:") << httpCode << endl;
        if(httpCode == HTTP_CODE_OK) payload = http.getString();
    } else {
        if (DEBUG) Serial << F("[HTTP] GET... failed, error:") << http.errorToString(httpCode).c_str() << endl;
    }

    http.end();
    return payload;
}

void autoUpdateIfForced(const char *ignore) {
  //Serial << F("Waiting for Wifi connection\n");
  if (waitForWifi(10000) != WL_CONNECTED) return;
  String urlGen = String(F("http://anker-bg.com/vlast3k/vesprino/"));
  String chipid =  String(ESP.getChipId(), 16);
  chipid.toUpperCase();
  String urlChip = String(F("http://anker-bg.com/vlast3k/vesprino/")) + chipid + String(F("/"));
  String forcedUpdateGen  = getHTTPFile(urlGen + F("forced.txt"));
  String forcedUpdateChip = getHTTPFile(urlChip + F("forced.txt"));
  uint32_t fc = atol(forcedUpdateChip.c_str());
  uint32_t fg = atol(forcedUpdateGen.c_str());
  uint32_t bn = atol(BUILD_NUM);
  //Serial.printf("fg: %d, fc: %d, bn:%d\n", fg, fc, bn);
  if (bn > fc && bn > fg) return;
  String forcedUpdate = (fc >= fg) ? forcedUpdateChip : forcedUpdateGen;
  String urlUpdate    = (fc >= fg) ? urlChip : urlGen;
  if (DEBUG) {
    Serial << F("OTA: current Build: ") << atol(BUILD_NUM) << endl;
    Serial << F("OTA: forced  Build: ") << atol(forcedUpdate.c_str()) << endl;
  }
  //if (atol(BUILD_NUM) >= atol(forcedUpdateGen.c_str())) return;
  String url = urlUpdate + F("firmware") + forcedUpdate + F(".bin");
  doHttpUpdate(0, url.c_str());
}
