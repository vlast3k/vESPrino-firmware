#include <Arduino.h>
#include "MenuHandler.hpp"
boolean hasSSD1306 = false, hasSI7021 = false, hasPN532=false, hasBMP180=false, hasBH1750=false, hasAPDS9960=false;
bool shouldSend = false;
boolean DEBUG = false;
boolean SKIP_LOOP = false;
MenuHandler menuHandler;
char atCIPSTART_IP[20];
