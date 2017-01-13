#include "plugins/AT_FW_Plugin.hpp"
#include "common.hpp"
#include "plugins/WifiStuff.hpp"
extern WifiStuffClass WifiStuff;

extern char atCIPSTART_IP[20];
void AT_FW_Plugin::registerCommands(MenuHandler *handler) {
  // else if (line[0] == 'A') mockATCommand(line);
  // else if (line[0] == 'G') getTS(line);
   handler->registerCommand(new MenuEntry(F("A"), CMD_BEGIN, &AT_FW_Plugin::mockATCommand, F("AT FW Mock A:")));
   handler->registerCommand(new MenuEntry(F("G"), CMD_BEGIN, &AT_FW_Plugin::getTS, F("AT FW Mock G:")));
}


void AT_FW_Plugin::getTS(const char* line) {
  HTTPClient http;
  http.begin(String(HTTP_STR)  + atCIPSTART_IP + (line + 4));
  processResponseCodeATFW(&http, http.GET());
}

void AT_FW_Plugin::atCIPSTART(const char *p) {
  p = extractStringFromQuotes(p, atCIPSTART_IP, 20);
  p = extractStringFromQuotes(p, atCIPSTART_IP, 20);
}

void AT_FW_Plugin::mockATCommand(const char *line) {
  if (line[0] == 'A') {
    if (strstr(line, "AT+CWJAP_DEF")) WifiStuff.setWifi(line);
    if (strstr(line, "AT+CIPSTART")) atCIPSTART(line);

    if (strstr(line, "AT+CIPSEND"))  LOGGER << ">" << endl;
    else                             LOGGER << "OK" << endl;
  }
}

int AT_FW_Plugin::processResponseCodeATFW(HTTPClient *http, int rc) {
  if (rc <= 0) LOGGER << F("Error Code: ") << rc << " = " << http->errorToString(rc).c_str() << endl;
  if (rc > 0) {
    if (DEBUG) {
      LOGGER << F("Payload: [");
      String s = http->getString();
      const char *p = s.c_str();
      int i=0;
      while (i < s.length()) {
        LOGGER << p[i++];
        if ((i%50) == 0) {
          LOGGER.flush();
          delay(100);
          yield();
        }
      }
      LOGGER <<  "]" << endl;
    }
    LOGGER << F("CLOSED") << endl; // for compatibility with AT FW
  } else {
    LOGGER << F("Failed") << endl;
  }

  LOGGER.flush();
  //delay(100);
  return rc;
}
