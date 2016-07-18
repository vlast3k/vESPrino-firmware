#ifdef VTHING_STARTER
#include "common.hpp"
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
SSD1306  *display = NULL;
int ledMode = 0;
void handleDWCommand(char *line);
//const RgbColor allColors[] = {Cred, Cpink, Clila, Cviolet, Cblue, Cmblue, Ccyan, Cgreen, Cyellow, Corange};
const RgbColor allColors[] = {Cred, Clila, Cmblue, Cgreen, Cyellow};
#define TOTAL_COLORS 5
void initVThingStarter();
void loopVThingStarter();
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


void oledHandleCommand(char *cmd) {
  if (!hasSSD1306)  return;
  if (!display) display = new SSD1306(0x3c, D7, D5);
  display->init();
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  if (strlen(cmd) < 8) {
    display->setFont(ArialMT_Plain_24);
  } else {
    display->setFont(ArialMT_Plain_16);
  }
  display->drawString(0, 0, cmd);
  display->display();
}

void handleCommandVESPrino(char *line) {
       if (strstr(line, "vespBttnA "))  putJSONConfig("vespBttn", strstr(line, " ")+1, true);
  else if (strstr(line, "vespBttn "))   putJSONConfig("vespBttn", strstr(line, " ")+1);
  else if (strstr(line, "vespRFIDA")) putJSONConfig("vespRFID" , strstr(line, " ")+1, true);
  else if (strstr(line, "vespRFID")) putJSONConfig("vespRFID" , strstr(line, " ")+1);
  else if (strstr(line, "vespDWCmd")) putJSONConfig("vespDWCmd" , strstr(line, " ")+1);
  else if (strstr(line, "vespSens"))  putJSONConfig("vespSens", strstr(line, " ")+1);
  else if (strstr(line, "vecmd "))    handleDWCommand(strstr(line, " ")+1);
}

boolean getDweetCommand(char *cmd) {
    static char lastDweet[30];

    char tmp[200];
    if (!getJSONConfig("vespDWCmd", tmp)[0]) return false;
    String url = String("http://dweet.io/get/latest/dweet/for/") + getJSONConfig("vespDWCmd", tmp);

    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    if(httpCode <= 0) {
      SERIAL << F("Failed to getDweet from: ") << url << ", due to: " << http.errorToString(httpCode) << endl;
      return false;
    }

    String payload = http.getString();
    StaticJsonBuffer<400> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(payload.c_str());
    if (!root.success()) {
      SERIAL << F("Parsing failed! : ") << payload.c_str() << endl;
      return false;
    }
    if (!root.containsKey("this") || strcmp(root["this"].asString(), "succeeded")) return false;
    if (strcmp(lastDweet, root["with"][0]["created"].asString()) == 0) return false;
    strcpy(lastDweet, root["with"][0]["created"].asString());
    strcpy(cmd, root["with"][0]["content"]["cmd"].asString());
    SERIAL << "New Dweet: " << cmd << endl;
    return true;
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

void handleDWCommand(char *line) {
       if (strstr(line, "oled_"))      oledHandleCommand(strstr(line, "_")+1);
  else if (strstr(line, "led_"))       ledHandleColor(strstr(line, "_")+1);
  else if (strstr(line, "ledbrg_"))    ledSetBrg(strstr(line, "_")+1);
  else if (strstr(line, "ledmode_"))   ledHandleMode(strstr(line, "_")+1);
}

void onGetDweets() {
  char line[200];
  boolean res = getDweetCommand(line);
  if (!res) return;
  handleDWCommand(line);
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
