#include <Arduino.h>
#include "MenuHandler.hpp"
#include "Timer.h"
#include "NeoPixelBus.h"
#include "LinkedList.h"
#include "interfaces\Destination.hpp"
#include "interfaces\Plugin.hpp"
#include "interfaces\Sensor.hpp"
#include "interfaces\Pair.h"
#include "sensors\CO2Sensor.hpp"
#include "destinations\CustomHTTPDest.hpp"
#include "plugins\PropertyList.hpp"
#include "plugins\TimerManager.hpp"
#include "plugins\PowerManager.hpp"
#include "sensors\SI7021Sensor.hpp"

boolean hasSSD1306 = false, hasSI7021 = false, hasPN532=false, hasBMP180=false, hasBH1750=false, hasAPDS9960=false;
boolean hasPIR = false;
bool shouldSend = false;
boolean DEBUG = false;
boolean SKIP_LOOP = false;
MenuHandler menuHandler;
char atCIPSTART_IP[20];
char commonBuffer200[200];
bool deepSleepWake = false;

NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod>  *strip;// = NeoPixelBus(1, D4);

LinkedList<Plugin *> plugins = LinkedList<Plugin *>();
LinkedList<Sensor *> sensors = LinkedList<Sensor *>();
LinkedList<Destination *> destinations = LinkedList<Destination *>();
LinkedList<Thr *> thresholds = LinkedList<Thr *>();
LinkedList<Timer *> timers = LinkedList<Timer *>();

CustomHTTPDest customHTTPDest;
CO2Sensor co2Sensor;
PropertyListClass PropertyList;
PowerManagerClass PowerManager;
TimerManagerClass TimerManager;
SI7021Sensor si7021Sensor;
