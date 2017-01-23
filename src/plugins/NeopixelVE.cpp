#include "plugins/NeopixelVE.hpp"
#include "plugins/PowerManager.hpp"
#include <Streaming.h>
//#include "plugins/WifiStuff.hpp"

extern PowerManagerClass PowerManager;

RgbColor allColors[] = {Cred, Clila, Cmblue, Cgreen, Cyellow};
extern NeopixelVE neopixel;
void registerPlugin(Plugin *plugin);
extern boolean DEBUG;
NeopixelVE::NeopixelVE() {
  registerPlugin(this);
}
bool NeopixelVE::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("ledcolor"), CMD_BEGIN, NeopixelVE::cmdLedHandleColor, F("ledcolor")));
  handler->registerCommand(new MenuEntry(F("ledbrg"), CMD_BEGIN, NeopixelVE::cmdLedSetBrg, F("ledbrg")));
  handler->registerCommand(new MenuEntry(F("ledmode"), CMD_BEGIN, NeopixelVE::cmdLedHandleMode, F("ledmode")));
  // else if (strstr(line, "led_"))       ledHandleColor(strstr(line, "_")+1);
  // else if (strstr(line, "ledbrg_"))    ledSetBrg(strstr(line, "_")+1);
  // else if (strstr(line, "ledmode_"))   ledHandleMode(strstr(line, "_")+1);
  return false;

}

 void NeopixelVE::cmdLedHandleColor(const char* line) {neopixel.cmdLedHandleColorInst(line);}
 void NeopixelVE::cmdLedSetBrg(const char* line) {neopixel.cmdLedSetBrgInst(line);}
 void NeopixelVE::cmdLedHandleMode(const char* line) {neopixel.cmdLedHandleModeInst(line);}
 void NeopixelVE::signal(const __FlashStringHelper *seq, SignalType sig) {
   //Serial << "signal: " << seq << endl;
   bool show = false;
   if (DEBUG) show = true;
   if (!PowerManager.isWokeFromDeepSleep() && sig == SIGNAL_FIRST) show = true;
   if (show) {
     String cmd = String("ledcolor seq") + seq;
     cmdLedHandleColorInst(cmd.c_str());
   }
 }

 void NeopixelVE::handleSequence(const char *seq) {
   seq = seq + 3;
   RgbColor oldColor = currentColor;
   while (*seq) {
     RgbColor c;
     int b = atoi(seq);
     if (b > 0) ledBrg = (float)b/100;
     for (char ch = *seq; ch >= '0' && ch <= '9'; ch = *(++seq));
     switch (*seq) {
       case 'r': c = Cred; break;
       case 'b': c = Cmblue; break;
       case 'g': c = Cgreen; break;
       case 'y': c = Cyellow; break;
       case 'w': c = Cwhite; break;
       case 'l': c = Clila; break;
       case 'c': c = Ccyan; break;
       case 'm': c = Cmblue; break;
       case 'd': c = oldColor;break;// Serial << "CurrentColor: " << c << endl; break;
       case 'n':
       default:  c = Cblack; break;
     }
     //old color - do not add blend, just reuse old color

     if (*seq != 'd') c = RgbColor::LinearBlend(c, Cblack, ledBrg);
     setLedColor(c);
     delay(333);
     seq ++;
   }
 }

 void NeopixelVE::cmdLedHandleColorInst(const char *line) {
  static char color[100] = "";
  if (line) line = strstr(line, " ");
  if (line) strncpy(color, line + 1, sizeof(color));
  //Serial << "cmdLedHandleColorInst: " << line << endl;
  //color.trim();
  if (strlen(color) == 0) return;
  if (strstr(color, "seq") == color) {
    neopixel.handleSequence(color);
    return;
  }
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
    //LOGGER << " receveid int: " << data << endl;
    c = RgbColor((data & 0xFF0000) >> 16, (data & 0x00FF00) >> 8, (data & 0x0000FF));
  }
  c = RgbColor::LinearBlend(c, Cblack, ledBrg);
  setLedColor(c);
}

void NeopixelVE::cmdLedSetBrgInst(const char *line) {
  //Serial << "Handle brighjt: " << line << endl;
  line = strstr(line, " ");
  if (!line) return;
  ledBrg = ((float)atoi(line + 1))/100;
  cmdLedHandleColorInst((const char *)NULL);
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
  //  LOGGER << "Will Blink " << blinks << "x, on:" << blOn << " ms, off:" << blOff << " ms" << endl;
    for (int i=0; i<blinks; i++) {
      cmdLedHandleColor(NULL);
      delay(blOn);
      setLedColor(0);
      if (i < blinks-1) delay(blOff);
    }
  }
}

void NeopixelVE::loop() {
  // if (ledMode == 1) {
  //   if (millis() - lastChange > 500) {
  //     handleDWCommand("led_next");
  //     lastChange = millis();
  //   }
  // }
}

RgbColor NeopixelVE::ledNextColor() {
  static byte colorIdx = 0;
  return allColors[colorIdx++ % TOTAL_COLORS];
}

void NeopixelVE::setLedColor(const RgbColor &color) {
  NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> strip(1, 2);
  strip.Begin();
  strip.SetPixelColor(0, color);
  currentColor = color;
  strip.Show();
  delay(1);
  Serial1.end();
  Serial1.flush();
  //delay(10);
}

int NeopixelVE::getAmbientLightRaw() {
  uint32_t sum=0;
  int samples = 10;
  for (int i=0; i<samples; i++) sum += analogRead(A0);
  return sum/samples;
}

int NeopixelVE::getAmbientLight(int stopMs) {
  RgbColor cur = currentColor;
  if (stopMs) {
     setLedColor(Cblack);
     delay(stopMs);
   }
  int light = getAmbientLightRaw();
  if (stopMs) setLedColor(cur);
  return light;
}
