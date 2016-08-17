#ifdef VTHING_VESPRINO
#include "common.hpp"


int ledMode = 0;

//const RgbColor allColors[] = {Cred, Cpink, Clila, Cviolet, Cblue, Cmblue, Ccyan, Cgreen, Cyellow, Corange};
const RgbColor allColors[] = {Cred, Clila, Cmblue, Cgreen, Cyellow};
#define TOTAL_COLORS 5

RgbColor ledNextColor() {
  static byte colorIdx = 0;
  return allColors[colorIdx++ % TOTAL_COLORS];
}

//RgbColor *currentColor;

void setLedColor(const RgbColor &color) {
  NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> strip(1, 2);
  strip.Begin();
  strip.SetPixelColor(0, color);
  strip.Show();
  delay(1);
  Serial1.end();
 // currentColor = color;
}

float ledBrg = 0.7f;
void ledHandleColor(char *colorNew) {
  static char color[10];
  if (colorNew) strcpy(color, colorNew);
  //SERIAL << "color: " << color;
  //color.trim();
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
    //SERIAL << " receveid int: " << data << endl;
    c = RgbColor((data & 0xFF0000) >> 16, (data & 0x00FF00) >> 8, (data & 0x0000FF));
  }
  c = RgbColor::LinearBlend(c, Cblack, ledBrg);
  setLedColor(c);
}

void ledSetBrg(char *s) {
  ledBrg = ((float)atoi(s))/100;
  ledHandleColor(NULL);
}


void ledHandleMode(char *cmd) {
  ledMode = atoi(cmd);
  if (ledMode == 2) { //blink 2_2
    int blinks = 1;
    int blOn = 200;
    int blOff = 200;
    if (cmd = strstr(cmd, "_")) {
      cmd++;
      blinks = atoi(cmd);
      if (cmd = strstr(cmd, "_")) {
        cmd++;
        blOn = atoi(cmd);
        if (cmd = strstr(cmd, "_")) {
          cmd++;
          blOff = atoi(cmd);
        }
      }
    }
    Serial << "Will Blink " << blinks << "x, on:" << blOn << " ms, off:" << blOff << " ms" << endl;
    for (int i=0; i<blinks; i++) {
      ledHandleColor(NULL);
      delay(blOn);
      setLedColor(0);
      if (i < blinks-1) delay(blOff);
    }
  }
}

uint32_t lastChange = 0;
void loopNeoPixel() {
  if (ledMode == 1) {
    if (millis() - lastChange > 500) {
      handleDWCommand("led_next");
      lastChange = millis();
    }
  }
}
#endif
