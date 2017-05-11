#include <Wire.h>
#include "sensors/PMS7003Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "common.hpp"
#include <SoftwareSerialESP.h>
extern PMS7003Sensor _PMS7003Sensor;
#define PROP_PMS7003_ENABLE F("pms7003.enable")
PMS7003Sensor::PMS7003Sensor() {
  //enabled = true;
  registerSensor(this);
}

void PMS7003Sensor::onProperty(String &key, String &value) {
  if (key == PROP_PMS7003_ENABLE) enabled = PropertyList.toBool(value);
}

bool PMS7003Sensor::setup(MenuHandler *handler) {
  if (enabled) handler->registerCommand(new MenuEntry(F("pmstest"), CMD_EXACT, &PMS7003Sensor::test, F("testSensor toggle testSesnor")));
  return enabled;

}
void PMS7003Sensor::test(const char *ig) {
  PMS7003_framestruct thisFrame;
  _PMS7003Sensor.pms7003_read(thisFrame);
}
bool PMS7003Sensor::pms7003_read(PMS7003_framestruct &thisFrame) {
    int incomingByte = 0; // for incoming serial data
    char frameBuf[MAX_FRAME_LEN];
    int detectOff = 0;
    int frameLen = MAX_FRAME_LEN;
    bool inFrame = false;
    char printbuf[256];

    uint16_t calcChecksum = 0;

    //  Particle.publish("PMS7003", printbuf, 60, PRIVATE);
    // send data only when you receive data:
//    Serial.println("-- Reading PMS7003");
    //D7-rx, D6, Tx
    SoftwareSerialESP pms(D7,D6,128); // RX, TX
    pms.begin(9600);
    bool packetReceived = false;
    while (!packetReceived) {
        delay(1);
        if (pms.available() > 32) {
            int drain = pms.available();
            if (DEBUG) {
                Serial.print(F("-- Draining buffer: "));
                Serial.println(pms.available(), DEC);
            }
            for (int i = drain; i > 0; i--) {
                pms.read();
            }
        }
        if (pms.available() > 0) {
            if (DEBUG) {
                Serial.print(F("-- Available: "));
                Serial.println(pms.available(), DEC);
            }
            incomingByte = pms.read();

            if (DEBUG) {
                Serial.print(F("-- READ: "));
                Serial.println(incomingByte, HEX);
            }
            if (!inFrame) {
                if (incomingByte == 0x42 && detectOff == 0) {
                    frameBuf[detectOff] = incomingByte;
                    thisFrame.frameHeader[0] = incomingByte;
                    calcChecksum = incomingByte; // Checksum init!
                    detectOff++;
                }
                else if (incomingByte == 0x4D && detectOff == 1) {
                    frameBuf[detectOff] = incomingByte;
                    thisFrame.frameHeader[1] = incomingByte;
                    calcChecksum += incomingByte;
                    inFrame = true;
                    detectOff++;
                }
                else {
                    Serial.print(F("-- Frame syncing... "));
                    Serial.print(incomingByte, HEX);
                    if (DEBUG) {
                    }
                    Serial.println();
                }
            }
            else {
                frameBuf[detectOff] = incomingByte;
                calcChecksum += incomingByte;
                detectOff++;
                uint16_t val = frameBuf[detectOff-1]+(frameBuf[detectOff-2]<<8);
                switch (detectOff) {
                    case 4:
                        thisFrame.frameLen = val;
                        frameLen = val + detectOff;
                        break;
                    case 6:
                        thisFrame.concPM1_0_CF1 = val;
                        break;
                    case 8:
                        thisFrame.concPM2_5_CF1 = val;
                        break;
                    case 10:
                        thisFrame.concPM10_0_CF1 = val;
                        break;
                    case 12:
                        thisFrame.concPM1_0_amb = val;
                        break;
                    case 14:
                        thisFrame.concPM2_5_amb = val;
                        break;
                    case 16:
                        thisFrame.concPM10_0_amb = val;
                        break;
                    case 18:
                        thisFrame.rawGt0_3um = val;
                        break;
                    case 20:
                        thisFrame.rawGt0_5um = val;
                        break;
                    case 22:
                        thisFrame.rawGt1_0um = val;
                        break;
                    case 24:
                        thisFrame.rawGt2_5um = val;
                        break;
                    case 26:
                        thisFrame.rawGt5_0um = val;
                        break;
                    case 28:
                        thisFrame.rawGt10_0um = val;
                        break;
                    case 29:
                        val = frameBuf[detectOff-1];
                        thisFrame.version = val;
                        break;
                    case 30:
                        val = frameBuf[detectOff-1];
                        thisFrame.errorCode = val;
                        break;
                    case 32:
                        thisFrame.checksum = val;
                        calcChecksum -= ((val>>8)+(val&0xFF));
                        break;
                    default:
                        break;
                }

                if (detectOff >= frameLen) {
                    sprintf(printbuf, String(F("PMS7003 ")).c_str());
                    sprintf(printbuf, String(F("%s[%02x %02x] (%04x) ")).c_str(), printbuf,
                        thisFrame.frameHeader[0], thisFrame.frameHeader[1], thisFrame.frameLen);
                    sprintf(printbuf, String(F("%sCF1=[%04d %04d %04d] ")).c_str(), printbuf,
                        thisFrame.concPM1_0_CF1, thisFrame.concPM2_5_CF1, thisFrame.concPM10_0_CF1);
                    sprintf(printbuf, String(F("%samb=[%04d %04d %04d] ")).c_str(), printbuf,
                        thisFrame.concPM1_0_amb, thisFrame.concPM2_5_amb, thisFrame.concPM10_0_amb);
                    sprintf(printbuf, String(F("%sraw=[%04d %04d %04d %04d %04d %04d] ")).c_str(), printbuf,
                        thisFrame.rawGt0_3um, thisFrame.rawGt0_5um, thisFrame.rawGt1_0um,
                        thisFrame.rawGt2_5um, thisFrame.rawGt5_0um, thisFrame.rawGt10_0um);
                    sprintf(printbuf, String(F("%sver=%02x err=%02x ")).c_str(), printbuf,
                        thisFrame.version, thisFrame.errorCode);
                    sprintf(printbuf, String(F("%scsum=%04x %s xsum=%04x")).c_str(), printbuf,
                        thisFrame.checksum, (calcChecksum == thisFrame.checksum ? "==" : "!="), calcChecksum);
                    Serial.println(printbuf);
                   // Particle.publish("Data1", printbuf, 60, PRIVATE);
                    packetReceived = true;
                    detectOff = 0;
                    inFrame = false;
                }
            }
        }
    }
    //pms.end();
    return (calcChecksum == thisFrame.checksum);
}


void PMS7003Sensor::getData(LinkedList<Pair *> *data) {
 if (!enabled) return;
  LOGGER << F("PMS7003Sensor::getData") << endl;
  PMS7003_framestruct thisFrame;
  if (pms7003_read(thisFrame)) {
    data->add(new Pair("PM10", String(thisFrame.concPM10_0_amb)));
    data->add(new Pair("PM25", String(thisFrame.concPM2_5_amb)));
    data->add(new Pair("PM1", String(thisFrame.concPM1_0_amb)));
    data->add(new Pair("PM10C", String(thisFrame.concPM10_0_CF1)));
    data->add(new Pair("PM25C", String(thisFrame.concPM2_5_CF1)));
    data->add(new Pair("PM1C", String(thisFrame.concPM1_0_CF1)));
  }
}
