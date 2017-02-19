#include "plugins/NeopixelVE.hpp"
#include "plugins/PropertyList.hpp"
#include "plugins/PowerManager.hpp"
#include <Streaming.h>
//#include "plugins/WifiStuff.hpp"

extern PowerManagerClass PowerManager;
#define PROP_LDR_OVERRIDE F("ldr.override")
#define PROP_LDR_ENABLED  F("ldr.enabled")
RgbColor allColors[] = {Cred, Clila, Cmblue, Cgreen, Cyellow};
extern NeopixelVE neopixel;
void registerPlugin(Plugin *plugin);
extern boolean DEBUG;
NeopixelVE::NeopixelVE() {
  registerPlugin(this);


}

void NeopixelVE::onProperty(String &key, String &value) {
  if (key == PROP_LDR_OVERRIDE) lightPreset = atoi(value.c_str());
  else if (key == PROP_LDR_ENABLED) {
    ldrEnabled = PropertyList.toBool(value);
    lightOff = getAmbientLight(0);
    lightOn = lightOff;
  }
}

bool NeopixelVE::setup(MenuHandler *handler) {
  // String t = "true";
  // Serial <<"NEOPIXEL: " << ldrEnabled << " " << PropertyList.toBool(t);
  //delay(100);
  //lightOff = getAmbientLight(0);
//  int xt = getAmbientLight(200);
//  Serial << "Neopixel start, lightOff: " << lightOff << ", 200delay: " << xt << endl;
  handler->registerCommand(new MenuEntry(F("ledcolor"), CMD_BEGIN, NeopixelVE::cmdLedHandleColor, F("ledcolor")));
  handler->registerCommand(new MenuEntry(F("ledbrg"), CMD_BEGIN, NeopixelVE::cmdLedSetBrg, F("ledbrg")));
  handler->registerCommand(new MenuEntry(F("ledmode"), CMD_BEGIN, NeopixelVE::cmdLedHandleMode, F("ledmode")));
  //handler->registerCommand(new MenuEntry(F("ledmap"), CMD_BEGIN, NeopixelVE::testMapColor, F("ledmode")));
  // else if (strstr(line, "led_"))       ledHandleColor(strstr(line, "_")+1);
  // else if (strstr(line, "ledbrg_"))    ledSetBrg(strstr(line, "_")+1);
  // else if (strstr(line, "ledmode_"))   ledHandleMode(strstr(line, "_")+1);
  return false;

}

 void NeopixelVE::cmdLedHandleColor(const char* line) {neopixel.cmdLedHandleColorInst(line);}
 void NeopixelVE::cmdLedSetBrg(const char* line) {neopixel.cmdLedSetBrgInst(line);}
 void NeopixelVE::cmdLedHandleMode(const char* line) {neopixel.cmdLedHandleModeInst(line);}
 void NeopixelVE::signal(const __FlashStringHelper *seq, SignalType sig) {
   //Serial << F("signal: ") << seq << endl;
   //Serial.flush();
   bool show = false;
   if (DEBUG) show = true;
   if (!PowerManager.isWokeFromDeepSleep() && sig == SIGNAL_FIRST) show = true;
   if (show) {
     String cmd = String(F("ledcolor seq")) + seq;
     cmdLedHandleColorInst(cmd.c_str());
   }
 }

 float NeopixelVE::getAutoBrg() {
   int factor;
   //Serial << "getAutobrg = lightOff =" << lightOff << endl;
   if (lightOff < 400) factor = 30;
   else if (lightOff < 800) factor = 20;
   else factor = 15;
   float res = (99.0F - ((float)lightOff / factor)) / 100;
   if (DEBUG) Serial << F("LED AutoBrg: ") << res << endl;
   return res;
 }

 void NeopixelVE::handleSequence(const char *seq) {
   seq = seq + 3;
   //Serial << "NeoPixel.handleSequence: " << seq << endl;
   //Serial.flush();
   //RgbColor oldColor = currentColor;
   RgbColor c;
   while (*seq) {

     int b = atoi(seq);
     if (b > 10) {
       ledBrg = (float)b/100;
       isAutoBrg = false;
     } else if (b == 1) {
       ledBrg = getAutoBrg();
       isAutoBrg = true;
     }
     //Serial << "nw ledBrg = " << ledBrg << endl;

     for (char ch = *seq; ch >= '0' && ch <= '9'; ch = *(++seq));
     switch (*seq) {
       case 'r': c = Cred; break;
       case 'b': c = Cmblue; break;
       case 'g': c = Cgreen; break;
       case 'u': c = Cgreen2; break;
       case 'y': c = Cyellow; break;
       case 'w': c = Cwhite; break;
       case 'l': c = Clila; break;
       case 'c': c = Ccyan; break;
       case 'm': c = Cmblue; break;
       case 'd': c = rawCurrentColor;break;// Serial << "CurrentColor: " << c << endl; break;
       case 'n':
       default:  c = Cblack; break;
     }
     //old color - do not add blend, just reuse old color
     RgbColor cBrg = c;
     //if (*seq != 'd') {
       cBrg = RgbColor::LinearBlend(c, Cblack, ledBrg);
    //   lastColorChar = *seq;
     //}
     setLedColor(cBrg);
     delay(333);
     seq ++;
   }
   rawCurrentColor = c;
 }

 void NeopixelVE::cmdLedHandleColorInst(const char *line) {
  static char color[100] = "";
  if (line) line = strstr(line, " ");
  if (line) strncpy(color, line + 1, sizeof(color));
  if (DEBUG) Serial << F("LED SetColor: ") << line << endl;
  //Serial << "cmdLedHandleColorInst: " << line << endl;
  //color.trim();
  if (strlen(color) == 0) return;
  if (strstr(color, "seq") == color) {
    neopixel.handleSequence(color);
  } else {
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
  delay(50);
  lightOn = getAmbientLight(0);
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
  if (millis() - ambLightRecheck > 1000) {
    //Serial<< "will check. lightOn:" << lightOn << ":"  <<  getAmbientLight(0) << endl;
    if (abs(lightOn - getAmbientLight(0)) > (ldrEnabled?20:1)) {
      lightOn = getAmbientLight(0);
      lightOff = getAmbientLight(100);
      //Serial<< "new light off" << lightOff << endl;

      if (isAutoBrg) cmdLedHandleColorInst(F("ledcolor seq1d"));
    }
    ambLightRecheck = millis();
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
  currentColor = color;
  strip.Show();
  delay(1);
  Serial1.end();
  Serial1.flush();
  //delay(10);
}

int NeopixelVE::getAmbientLightRaw() {
  if (!ldrEnabled) {
    return (lightPreset+5)*(lightPreset+5);
  } else {
    uint32_t sum=0;
    int samples = 30;
    for (int i=0; i<samples; i++) sum += analogRead(A0);
    float res= (sum/samples) * ( 1.0F + (float)lightPreset/10);
    //Serial << "Ambient Light res: " << res << " " << ( 1.0F + (float)lightPreset/10) <<endl;
    return res;
  }
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


RgbColor NeopixelVE::getColorMapping(char c) {
  switch (c) {
    case 'r': return Cred;
    case 'v': return Cviolet;
    case 'b': return Cmblue;
    case 'u': return Cgreen2;
    case 'g': return Cgreen;
    case 'y': return Cyellow;
    case 'w': return Cwhite;
    case 'l': return Clila;
    case 'c': return Ccyan;
    case 'm': return Cmblue;
    default: return Cblack;
  }
}
