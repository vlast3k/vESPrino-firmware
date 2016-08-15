#include <SI7021.h>
#include <Wire.h>
#include "sensors\SI7021Sensor.hpp"
#include <LinkedList.h>
#include "interfaces\Pair.h"
#include "common.hpp"

SI7021Sensor::SI7021Sensor() {
  registerSensor(this);
}

void SI7021Sensor::getData(LinkedList<Pair *> *data) {
  Serial << "SI721 get Data" << endl;
  delay(100);
   if (!initSensor()) return;
   data->add(new Pair("TEMP", String(si7021->readTemp())));
   data->add(new Pair("HUM", String(si7021->readHumidity())));
   closeSensor();
     Serial << "end si7021" << endl;
}

bool SI7021Sensor::initSensor() {
  si7021 = new SI7021();
  //si7021->begin(D1, D6); // Runs : Wire.begin() + reset()
  if (si7021->getDeviceID() == 255) {
    delete si7021;
    si7021 = NULL;
    return false;
  }
//  SERIAL << "Found SI7021 Temperature/Humidity Sensor\n" << endl;
  si7021->setHumidityRes(8); // Humidity = 12-bit / Temperature = 14-bit
  return true;
}

void SI7021Sensor::closeSensor() {
  delete si7021;
  si7021 = NULL;
}

// boolean si7021init() {
//     si7021 = new SI7021();
//     si7021->begin(D1, D6); // Runs : Wire.begin() + reset()
//     if (si7021->getDeviceID() == 255) {
//       delete si7021;
//       si7021 = NULL;
//       return false;
//     }
//     SERIAL << "Found SI7021 Temperature/Humidity Sensor\n" << endl;
//     si7021->setHumidityRes(8); // Humidity = 12-bit / Temperature = 14-bit
//     return true;
// }

// void onTempRead() {
//   if (!si7021) return;
//   float tmp = si7021->readTemp();
//   float hum = si7021->readHumidity();
//   SERIAL << F("Humidity : ") << hum << " %\t";
//   SERIAL << F("Temp : "    ) << tmp << " C" << endl;
//
//   String s = String("sndiot ") + tmp;
//   sndIOT(s.c_str());
// }
//
// void dumpTemp() {
//   if (!si7021) return;
//   SERIAL << F("Temp : ")     << si7021->readTemp() << " C" << endl;
// }
