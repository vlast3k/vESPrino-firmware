#include "plugins\AT_FW_Plugin.hpp"
#include "common.hpp"
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
    if (strstr(line, "AT+CWJAP_DEF")) setWifi(line);
    if (strstr(line, "AT+CIPSTART")) atCIPSTART(line);

    if (strstr(line, "AT+CIPSEND"))  SERIAL_PORT << ">" << endl;
    else                             SERIAL_PORT << "OK" << endl;
  }
}

int AT_FW_Plugin::processResponseCodeATFW(HTTPClient *http, int rc) {
  if (rc > 0) SERIAL_PORT << F("Response Code: ") << rc << endl;
  else SERIAL_PORT << F("Error Code: ") << rc << " = " << http->errorToString(rc).c_str() << endl;
  if (rc > 0) {
    if (DEBUG) SERIAL_PORT << F("Payload: [") << http->getString() << "]" << endl;
    SERIAL_PORT << F("CLOSED") << endl; // for compatibility with AT FW
  } else {
    SERIAL_PORT << F("Failed") << endl;
  }
  Serial.flush();
  delay(100);
  return rc;
}
