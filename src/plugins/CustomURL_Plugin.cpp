#include "plugins/CustomURL_Plugin.hpp"
#include "plugins/AT_FW_Plugin.hpp"
#include "common.hpp"
#include "PropertyList.hpp"

void CustomURL_Plugin::registerCommands(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("cfggen"), CMD_BEGIN, &CustomURL_Plugin::cfgGENIOT, F("")));
}

void CustomURL_Plugin::cfgGENIOT(const char *p) {
  char genurl[140] = "";
  if (!p[6]) {
    SERIAL_PORT << F("Cleared Generic URL") << endl;
  } else {
    strncpy(genurl, p+7, sizeof(genurl)-1);
    SERIAL_PORT << F("Stored Generic URL: ") << genurl << endl;
  }
  PropertyList.putProperty(EE_GENIOT_PATH, genurl); // path
  SERIAL_PORT << F("DONE") << endl;
}

void CustomURL_Plugin::sndGENIOT(const char *line) {
  char str[140], str2[150];
  strcpy(str, PropertyList.readProperty(EE_GENIOT_PATH));
  if (strstr(str, "thingspeak")) {
    strcat(str, "&field2=%d");
    sprintf(str2, str, &line[7], millis()/60000L);
  } else {
    sprintf(str2, str, &line[7]);
  }
  SERIAL_PORT << F("Sending to URL: \"") << str2 << "\"" << endl;

  HTTPClient http;
  http.begin(str2);
  //addHCPIOTHeaders(&http, token);
  int rc = AT_FW_Plugin::processResponseCodeATFW(&http, http.GET());
  //SERIAL_PORT << "Result: " << http.errorToString(rc).c_str();
}
