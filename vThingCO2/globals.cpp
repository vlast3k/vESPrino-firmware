#include <Arduino.h>
#include "MenuHandler.hpp"
#include "Timer.h"
#include "NeoPixelBus.h"
boolean hasSSD1306 = false, hasSI7021 = false, hasPN532=false, hasBMP180=false, hasBH1750=false, hasAPDS9960=false;
boolean hasPIR = false;
bool shouldSend = false;
boolean DEBUG = false;
boolean SKIP_LOOP = false;
MenuHandler menuHandler;
char atCIPSTART_IP[20];
Timer *tmrStopLED;
NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod>  *strip;// = NeoPixelBus(1, D4);
