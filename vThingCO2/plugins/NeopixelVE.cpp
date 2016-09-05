#include "plugins\NeopixelVE.hpp"
#include "MenuHandler.hpp"
#include "common.hpp"
#include <NeoPixelBus.h>
RgbColor allColors[] = {Cred, Clila, Cmblue, Cgreen, Cyellow};
extern NeopixelVE neopixel;

NeopixelVE::NeopixelVE() {
  registerPlugin(this);
}
void NeopixelVE::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("ledcolor"), CMD_BEGIN, NeopixelVE::cmdLedHandleColor, F("ledcolor")));
  handler->registerCommand(new MenuEntry(F("ledbrg"), CMD_BEGIN, NeopixelVE::cmdLedSetBrg, F("ledbrg")));
  handler->registerCommand(new MenuEntry(F("ledmode"), CMD_BEGIN, NeopixelVE::cmdLedHandleMode, F("ledmode")));
  // else if (strstr(line, "led_"))       ledHandleColor(strstr(line, "_")+1);
  // else if (strstr(line, "ledbrg_"))    ledSetBrg(strstr(line, "_")+1);
  // else if (strstr(line, "ledmode_"))   ledHandleMode(strstr(line, "_")+1);
}

 void NeopixelVE::cmdLedHandleColor(const char* line) {neopixel.cmdLedHandleColorInst(line);}
 void NeopixelVE::cmdLedSetBrg(const char* line) {neopixel.cmdLedSetBrgInst(line);}
 void NeopixelVE::cmdLedHandleMode(const char* line) {neopixel.cmdLedHandleModeInst(line);}

void NeopixelVE::cmdLedHandleColorInst(const char *line) {
  static char color[10] = "";
  if (line) line = strstr(line, " ");
  if (line) strcpy(color, line + 1);
  //SERIAL_PORT << "color: " << color;
  //color.trim();
  if (strlen(color) == 0) return;
  RgbColor c;
       if (!strcmp(color, "red"))    c = Cred;
  else if (!strcmp(color, "blue"))   c = Cblue;
  else if (!strcmp(color, "green"))  c = Cgreen;
  else if (!strcmp(color, "yellow")) c = Cyellow;
  else if (!strcmp(color, "orange")) c = Corange;
  else if (!strcmp(color, "white"))  c = Cwhite;
  else if (!strcmp(color, "black"))  c = Cblack;
  else if (!strcmp(color, "pink"))   c = Cpink;
  else if (!strcmp(color, "lila"))   c = Clila;
  else if (!strcmp(color, "violet")) c = Cviolet;
  else if (!strcmp(color, "mblue"))  c = Cmblue;
  else if (!strcmp(color, "cyan"))   c = Ccyan;
  else if (!strcmp(color, "next"))   c = ledNextColor();
  else {
    uint32_t data = strtol(color, NULL, 0);
    //SERIAL_PORT << " receveid int: " << data << endl;
    c = RgbColor((data & 0xFF0000) >> 16, (data & 0x00FF00) >> 8, (data & 0x0000FF));
  }
  c = RgbColor::LinearBlend(c, Cblack, ledBrg);
  setLedColor(c);
}

void NeopixelVE::cmdLedSetBrgInst(const char *line) {
  line = strstr(line, " ");
  if (!line) return;
  ledBrg = ((float)atoi(line + 1))/100;
  cmdLedHandleColorInst(NULL);
}


void NeopixelVE::cmdLedHandleModeInst(const char *line) {
  line = strstr(line, " ");
  if (!line) return;
  ledMode = atoi(line + 1);
  if (ledMode == 2) { //blink 2_2
    int blinks = 1;
    int blOn = 200;
    int blOff = 200;
    if (line = strstr(line, "_")) {
      line++;
      blinks = atoi(line);
      if (line = strstr(line, "_")) {
        line++;
        blOn = atoi(line);
        if (line = strstr(line, "_")) {
          line++;
          blOff = atoi(line);
        }
      }
    }
    Serial << "Will Blink " << blinks << "x, on:" << blOn << " ms, off:" << blOff << " ms" << endl;
    for (int i=0; i<blinks; i++) {
      cmdLedHandleColor(NULL);
      delay(blOn);
      setLedColor(0);
      if (i < blinks-1) delay(blOff);
    }
  }
}

void NeopixelVE::loop() {
  if (ledMode == 1) {
    if (millis() - lastChange > 500) {
      handleDWCommand("led_next");
      lastChange = millis();
    }
  }
}

RgbColor NeopixelVE::ledNextColor() {
  static byte colorIdx = 0;
  return allColors[colorIdx++ % TOTAL_COLORS];
}

void NeopixelVE::setLedColor(const RgbColor &color) {
  NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> strip(1, 2);
  strip.Begin();
  strip.SetPixelColor(0, color);
  strip.Show();
  delay(1);
  Serial1.end();
  Serial1.flush();
  delay(10);
}
