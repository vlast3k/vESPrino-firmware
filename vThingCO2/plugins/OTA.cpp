#include <Arduino.h>

#include <ESP8266httpUpdate.h>
#include "common.hpp"

void doHttpUpdate(int mode, const char *url) {
  if (!url) {
    #ifdef VTHING_CO2
      url = "http://anker-bg.com/vlast3k/vthing_co2/latest.bin";
    #elif defined(VTHING_STARTER)
      url = "http://anker-bg.com/vlast3k/vthing_starter/latest.bin";
    #elif defined(VTHING_H801_LED)
      url = "http://anker-bg.com/vlast3k/h801/latest.bin";
    #elif defined(VAIR)
      url = "http://anker-bg.com/vlast3k/vair/latest.bin";
    #endif
  }

  SERIAL << F("Will update from: ") << url << endl;

  t_httpUpdate_return ret;
  ret= ESPhttpUpdate.update(url);

  switch(ret) {
    case HTTP_UPDATE_FAILED:
      SERIAL.println(F("HTTP_UPDATE_FAILED"));
      break;

    case HTTP_UPDATE_NO_UPDATES:
      SERIAL.println(F("HTTP_UPDATE_NO_UPDATES"));
      break;

    case HTTP_UPDATE_OK:
      SERIAL.println(F("HTTP_UPDATE_OK"));
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
}
