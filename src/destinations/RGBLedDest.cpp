#include "interfaces/Destination.hpp"
#include "interfaces/Plugin.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "RGBLedDest.hpp"
#include "common.hpp"
#include "MenuHandler.hpp"
#include "plugins/PropertyList.hpp"
#include "plugins/AT_FW_Plugin.hpp"
#include "plugins/NeopixelVE.hpp"

extern NeopixelVE neopixel;
extern RGBLedDest _RGBLedDest;
#define PROP_RGBLED_CFG F("rgbled.cfg")
RGBLedDest::RGBLedDest() {
  registerDestination(this);
}

void RGBLedDest::onProperty(String &key, String &value) {
  if (key == PROP_RGBLED_CFG) {
    int items = getListItemCount(value.c_str());
    //if (getListItemCount(value.c_str()) 3) return;
    enabled = true;
    char key[20];
    char cvmin[20];
    char cvmax[20];
    char colors[20];
    if (items > 0) getListItem(value.c_str(), key, 0);
    if (items > 1) getListItem(value.c_str(), cvmin, 1);
    if (items > 2) getListItem(value.c_str(), cvmax, 2);
    if (items > 3) {
      getListItem(value.c_str(), colors, 3);
      vcolors = colors;
    }
    cfgKey = key;
    vmin = atoi(cvmin);
    vmax = atoi(cvmax);
  }

}
bool RGBLedDest::isEnabled() {
  return enabled;
}

bool RGBLedDest::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("rgbled_test"), CMD_EXACT, &RGBLedDest::test, F("serial_dump_toggle toggle serial dump output")));
  return false;
  //handler->registerCommand(new MenuEntry(F("ser"), CMD_EXACT, &RGBLedDest::toggle, F("serial_dump_toggle toggle serial dump output")));
}

 void RGBLedDest::test(const char *line) {
   _RGBLedDest.testInst();
 }

 void RGBLedDest::testInst() {
   if (!cfgKey.length()) {
     Serial << F("No RGB Led cfg") << endl;
     return;
   }
   float step = abs((vmax - vmin)/50);
   for (int i=vmin - step; i < vmax + step; i+=step) {
     LOGGER << F("Value: ") << i << F(" / ") << vmax << endl;
     LOGGER.flush();
     delay(150);
     _RGBLedDest.mapColor(i, vmin, vmax);
   }
 }
//
// void RGBLedDest::toggleInst() {
//   enabled != enabled;
//   LOGGER << F("LOGGER Dump Destination:") << (enabled ? F("ENABLED") : F("DISABLED")) << endl;
// }

void RGBLedDest::mapColor(int current, int minVal, int maxVal) {
  String colorRange;
  if (vcolors.length() > 0) colorRange = vcolors;
  else colorRange ="uyrv";
  // int minVal = 400;
  // int maxVal = 3400; //3000  ... /4 = 750
  //int current = 1300; // divv = 900
  current = constrain(current, minVal, maxVal-1);
  //Serial << "Current = " << current;
  int steps = colorRange.length() - 1;
  int numPerVal = abs ((maxVal - minVal)) / steps; // = 750
  int colorIdx = abs(current - minVal) / numPerVal; // = 1
  char colorStart = colorRange.charAt(colorIdx);
  char colorEnd   = colorRange.charAt(colorIdx + 1);
  int rangeStart = minVal + numPerVal * colorIdx;
  int currentX = current - rangeStart;
  float place = (float)currentX/numPerVal;
  //Serial << "place = " << place << ", colorIdx = " << colorIdx << endl;
  RgbColor cStart = neopixel.getColorMapping(colorStart);
  RgbColor cEnd   = neopixel.getColorMapping(colorEnd);

  RgbColor c = RgbColor::LinearBlend(cStart, cEnd, place);
  neopixel.putRawCurrentColor(c);
  c = RgbColor::LinearBlend(c, Cblack, neopixel.getAutoBrg());
  neopixel.setLedColor(c);

  //Serial << "curr: " << current << ", cs: " << colorStart << ", ce: " << colorEnd << ", place = " <<place << endl;
}
// void NeopixelVE::testMapColor(const char *ignore) {
//   for (int i=300; i < 3600; i+=50) {
//     Serial << i<< endl;
//     mapColor(i);
//     delay(110);
//   }
// }

bool RGBLedDest::process(LinkedList<Pair *> &data) {
  //if (!enabled) return false;
  //LOGGER << F("RGBLedDest::process") << endl;
  if (!cfgKey.length()) {
    if (millis() > 300L*1000) {
      if (!turnedOffLedOnce) {
        neopixel.cmdLedHandleColorInst(F("ledcolor seqn"));
        turnedOffLedOnce = true;
      }
    }
  } else {
    turnedOffLedOnce = true;
    for (int i=0; i < data.size(); i++) {
      Pair *p = data.get(i);
      if (cfgKey == p->key) {
        int current = atoi(p->value.c_str());
        mapColor(current, vmin, vmax);
      }
    }
  }
  return true;
}
