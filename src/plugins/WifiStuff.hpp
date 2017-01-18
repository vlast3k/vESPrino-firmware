#ifndef WifiStuff_H
#define WifiStuff_H
#include "common.hpp"
#include "PropertyList.hpp"
#include "interfaces/Plugin.hpp"
#include "plugins/SAP_HCP_IOT_Plugin.hpp"
#include "plugins/CustomURL_Plugin.hpp"
#include "plugins/PropertyList.hpp"
#include "plugins/NeopixelVE.hpp"
//#include <ESP8266WiFiMulti.h>
#include "WiFiManager.h"

#ifdef VESP_PING_SSL
#include <WiFiClientSecure.h>
#endif

class WifiStuffClass : public Plugin {
public:
  WifiStuffClass() ;
  bool setup(MenuHandler *handler);
  const char* getName() {
    return "WifiStuff";
  }
  void loop(){};
  void onProperty(String &key, String &value);
  void handleWifi();
  wl_status_t waitForWifi(uint16_t timeoutMs=10000);
  void activeWait();
  void connectToWifi(const char *s1, const char *s2, const char *s3);
  static void wifiScanNetworks(const char *ignore);
  static void setWifi(const char* p);
  static void sendPingPort(const char *p);
  static void cmdSleeptype(const char *line);
  static void cmdDelay(const char *line);
  void applyStaticWifiConfig();
  void wifiConnectMulti();
  void wifiOff();
  static void setStaticWifi(const char* cmd);
  static void cmdIPConfig(const char *ignore);
  void WIFI_registerCommands(MenuHandler *handler);
  static void cbOnSaveConfigCallback();
  void stopAutoWifiConfig();
  static void startAutoWifiConfig(const char *ch);
  void storeStaticWifiInRTC();
  void loadStaticIPConfigFromRTC();

private:
  WiFiManager *wifiManager = NULL;
  //ESP8266WiFiMulti  *wifiMulti = NULL;
  uint32_t lostWifiConnection = 0;
  IPAddress ip = WiFi.localIP();
  wl_status_t wifiState = WL_NO_SHIELD;
  bool wifiAlreadyWaited = true;
  IPAddress staticIp, gateway, subnet, dns1, dns2;
  String ssid, pass;


};
#endif
