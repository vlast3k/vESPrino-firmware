#ifndef Neopixel_ve_h
#define Neopixel_ve_h

#include "interfaces/Plugin.hpp"
#include "MenuHandler.hpp"
//#include "common.hpp"
#include <NeoPixelBus.h>
#define TOTAL_COLORS 5
#define Cred     RgbColor(255, 0, 0)
#define Cpink    RgbColor(255, 0, 128)
#define Clila    RgbColor(255, 0, 255)
#define Cviolet  RgbColor(128, 0, 255)
#define Cblue    RgbColor(0, 0, 255)
#define Cmblue   RgbColor(0, 128, 255)
#define Ccyan    RgbColor(0, 255, 255)
#define Cgreen   RgbColor(0, 255, 0)
#define Cyellow  RgbColor(255, 255, 0)
#define Corange  RgbColor(255, 100, 0)
#define Cwhite   RgbColor(255, 255, 255)
#define Cblack   RgbColor(0)

enum SignalType {SIGNAL_DEBUG, SIGNAL_FIRST};
class NeopixelVE : public Plugin {
public:
  NeopixelVE();
  bool setup(MenuHandler *handler);
  const char* getName() {
    return "Neopixel";
  }
  void loop();
  void cmdLedHandleColorInst(const char* ignore);
  void cmdLedSetBrgInst(const char* ignore);
  void cmdLedHandleModeInst(const char* ignore);
  void cmdLedHandleColorInst(const __FlashStringHelper* s) {cmdLedHandleColorInst(String(s).c_str());};
  void cmdLedSetBrgInst(const __FlashStringHelper* s) {cmdLedSetBrgInst(String(s).c_str());};
  void cmdLedHandleModeInst(const __FlashStringHelper* s){cmdLedHandleModeInst(String(s).c_str());};
  void handleSequence(const char *seq);
  void signal(const __FlashStringHelper *seq, SignalType sig = SIGNAL_DEBUG);
  int getAmbientLightRaw();
  int getAmbientLight(int stopMs);
private:
  static void cmdLedHandleColor(const char* line);
  static void cmdLedSetBrg(const char* line);
  static void cmdLedHandleMode(const char* line);
  void setLedColor(const RgbColor &color);
  RgbColor ledNextColor();
  uint32_t lastChange = 0;
  float ledBrg = 0.7f;
  int ledMode = 0;
  RgbColor currentColor = Cblack;
  //const RgbColor allColors[] = {Cred, Cpink, Clila, Cviolet, Cblue, Cmblue, Ccyan, Cgreen, Cyellow, Corange};


  //NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod>  *strip;// = NeoPixelBus(1, D4);


};

#endif
