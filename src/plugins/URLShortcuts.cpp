#include "plugins/AT_FW_Plugin.hpp"
#include "plugins/URLShortcuts.hpp"
#include "CustomURL_Plugin.hpp"
#include "SAP_HCP_IOT_Plugin.hpp"

String ubik;
void URLShortcuts::registerCommands(MenuHandler *handler) {
//  handler->registerCommand(new MenuEntry(F("tstest"), CMD_EXACT, &URLShortcuts::sendTS, F("")));
//  handler->registerCommand(new MenuEntry(F("smp"), CMD_EXACT, &URLShortcuts::sndSimple, F("")));
//  handler->registerCommand(new MenuEntry(F("test"), CMD_EXACT, &URLShortcuts::doTest, F("")));
//  handler->registerCommand(new MenuEntry(F("ubi"), CMD_EXACT, &URLShortcuts::testUBI, F("")));
//  handler->registerCommand(new MenuEntry(F("tskey"), CMD_BEGIN, &URLShortcuts::setTSKey, F("")));
//  handler->registerCommand(new MenuEntry(F("ubik"), CMD_BEGIN, &URLShortcuts::setUBIKey, F("")));
//  handler->registerCommand(new MenuEntry(F("ubiv"), CMD_BEGIN, &URLShortcuts::setUBIValue, F("")));
}

// void URLShortcuts::doTest(const char *line) {
//   sndIOT("sndiot 567");
// }

void URLShortcuts::setTSKey(const char *line) {
  // char s[150];
  // sprintf(s, String(F("cfggen http://api.thingspeak.com/update?key=%s&field1=%%s")).c_str(), &line[6]);
  // CustomURL_Plugin::cfgGENIOT(s);
}

void URLShortcuts::setUBIKey(const char *line) {
  // ubik = String(&line[5]);
}

void URLShortcuts::setUBIValue(const char *line) {
  // char s[150];
  // sprintf(s, String(F("cfggen http://50.23.124.66/api/postvalue/?token=%s&variable=%s&value=%%s")).c_str(), ubik.c_str(), &line[5]);
  // CustomURL_Plugin::cfgGENIOT(s);
}


void URLShortcuts::testUBI(const char *line) {
  // HTTPClient http;
  // http.begin(F("http://50.23.124.66/api/postvalue/?token=Cg5W22qmWFcsMqsALMik04VtEF7PYA&variable=565965867625420c74ec604b&value=456"));
  // AT_FW_Plugin::processResponseCodeATFW(&http, http.GET());
}

void URLShortcuts::sendTS(const char *line) {
  // HTTPClient http;
  // http.begin(F("http://api.thingspeak.com/update?key=2DB818ODLIFO8TLF&field1=456"));
  // AT_FW_Plugin::processResponseCodeATFW(&http, http.GET());
}

void URLShortcuts::sndSimple(const char *line) {
  // HTTPClient http;
  // http.begin(F("https://iotmmsi024148trial.hanatrial.ondemand.com/com.sap.iotservices.mms/v1/api/http/data/c5c73d69-6a19-4c7d-9da3-b32198ba71f9/2023a0e66f76d20f47d7/sync?co2=34"));
  // SAP_HCP_IOT_Plugin::addHCPIOTHeaders(&http, String(F("be4e6b1381f6989b195a402420399a8")).c_str());
  // AT_FW_Plugin::processResponseCodeATFW(&http, http.POST(""));
}
