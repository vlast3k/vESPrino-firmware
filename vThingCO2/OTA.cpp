#include <Arduino.h>

#include <ESP8266httpUpdate.h>
#include <Streaming.h>
#define SERIAL Serial

void doHttpUpdate(int mode, char *url) {
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
