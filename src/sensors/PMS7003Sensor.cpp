#include <Wire.h>
#include "sensors/PMS7003Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "common.hpp"
#include <SoftwareSerialESP.h>
extern PMS7003Sensor _PMS7003Sensor;
#define PROP_PMS7003_ENABLE F("pms7003.enable")
#define PROP_PMS7003_PORT F("pms7003.port")
PMS7003Sensor::PMS7003Sensor() {
  //enabled = true;
  registerSensor(this);
}

void PMS7003Sensor::dump1(uint8_t *r, int len) {
  for (int i=0; i<len; i++) Serial << _HEX(*(r++)) << ",";
  Serial.println();
}

bool PMS7003Sensor::sendCmd(uint8_t *cmd, uint8_t *resp) {
  int rx = _RX, tx = _TX;
  #ifdef ESP8266
    SoftwareSerialESP PM1106_swSer(rx, tx, 128);
  #else
    SoftwareSerial PM1106_swSer(rx, tx);
  #endif
  PM1106_swSer.begin(9600);
  uint16_t sum = 0;
  Serial << F("Sending command: ") << endl;
//  dump(cmd);
  for (int i=1; i <= cmd[0]; i++) {
     sum+= cmd[i];
     Serial << F("sum is: ") << sum << endl;
   }
  cmd[cmd[0]+1] = sum>> 8;
  cmd[cmd[0]+2] = sum & 0xFF;


  _PMS7003Sensor.dump1(cmd+1, cmd[0]+2);
  PM1106_swSer.write(&cmd[1], cmd[0]+2);
  uint32_t start = millis();
  uint8_t buf[100];
  while (millis() - start <5000) {
    if (PM1106_swSer.available()) {
      int i=0;
      for (; i < 100 && PM1106_swSer.available(); i++) {
        buf[i] = PM1106_swSer.read();
      }
      Serial << F("Received: ");
      _PMS7003Sensor.dump1(buf, i);
    }
    delay(1);
  }
  Serial << F("end wait") << endl;
}

#define PMS7003_OPERATION_MODE_PASSIVE 0
#define PMS7003_OPERATION_MODE_ACTIVE 1
void PMS7003Sensor::changeMode(int mode) {
  Serial << F("Setting mode to: ") << mode << endl;
  uint8_t cmd[20];
  cmd[0] = 5;
  cmd[1] = 0x42;
  cmd[2] = 0x4D;
  cmd[3] = 0xE1;
  cmd[4] = 0;
  cmd[5] = mode;
  _PMS7003Sensor.sendCmd(cmd, cmd);
}

#define PMS7003_SLEEP_MODE_SLEEP 0
#define PMS7003_SLEEP_MODE_WAKEUP 1
void PMS7003Sensor::changeSleep(int sleep) {
  Serial << F("Setting sleep to: ") << sleep << endl;
  uint8_t cmd[20];
  cmd[0] = 5;
  cmd[1] = 0x42;
  cmd[2] = 0x4D;
  cmd[3] = 0xE4;
  cmd[4] = 0;
  cmd[5] = sleep;
  sendCmd(cmd, cmd);
}

void PMS7003Sensor::doRead() {
  uint8_t cmd[20];
  cmd[0] = 5;
  cmd[1] = 0x42;
  cmd[2] = 0x4D;
  cmd[3] = 0xE2;
  cmd[4] = 0;
  cmd[5] = 0;
  sendCmd(cmd, cmd);
}


void PMS7003Sensor::onProperty(String &key, String &value) {
  //if (key == PROP_PMS7003_ENABLE) enabled = PropertyList.toBool(value);
  if (key == PROP_PMS7003_PORT) {
    uint16_t _RX_TX = atoi(value.c_str());
    if (_RX_TX) {
      _RX = _RX_TX >> 8;
      _TX = _RX_TX & 0xFF;
      //Serial << ">>>> rx: " << _RX << _TX << endl;
    }
  }
}

void PMS7003Sensor::cmd_pms_sleep(const char *line) {
  char *s = strchr(line, ' ');
  _PMS7003Sensor.changeSleep(s[1] - '0');
}
void PMS7003Sensor::cmd_pms_mode(const char *line) {
  char *s = strchr(line, ' ');
  _PMS7003Sensor.changeMode(s[1] - '0');
}

void PMS7003Sensor::cmd_pms_read(const char *line) {
  char *s = strchr(line, ' ');
  _PMS7003Sensor.doRead();
}

bool PMS7003Sensor::hasSensor() {
  return _RX > -1;
}
bool PMS7003Sensor::setup(MenuHandler *handler) {
  //nt rx, tx;
  PMS7003_framestruct thisFrame;
  if (_RX == -2) {
    if (!_PMS7003Sensor.pms7003_read(_RX=D7, _TX=D6, thisFrame) &&
        !_PMS7003Sensor.pms7003_read(_RX=D6, _TX=D7, thisFrame) &&
        !_PMS7003Sensor.pms7003_read(_RX=D5, _TX=D1, thisFrame) &&
        !_PMS7003Sensor.pms7003_read(_RX=D1, _TX=D5, thisFrame) &&
        !_PMS7003Sensor.pms7003_read(_RX=D2, _TX=D8, thisFrame) &&
        !_PMS7003Sensor.pms7003_read(_RX=D8, _TX=D2, thisFrame) ) {
          _RX = _TX = -1;
    }
    Serial <<"storing PMS: " << _RX << "," << _TX << endl;
    uint16_t port = (_RX << 8) + _TX;
    PropertyList.putProperty(PROP_PMS7003_PORT, port);
  }

  if (_RX > -1) {
    handler->registerCommand(new MenuEntry(F("pmstest"), CMD_EXACT, &PMS7003Sensor::test, F("testSensor toggle testSesnor")));
    handler->registerCommand(new MenuEntry(F("pms_sleep"), CMD_BEGIN, &PMS7003Sensor::cmd_pms_sleep, F("testSensor toggle testSesnor")));
    handler->registerCommand(new MenuEntry(F("pms_mode"),  CMD_BEGIN, &PMS7003Sensor::cmd_pms_mode, F("testSensor toggle testSesnor")));
    handler->registerCommand(new MenuEntry(F("pms_read"),  CMD_BEGIN, &PMS7003Sensor::cmd_pms_read, F("testSensor toggle testSesnor")));
    return true;
  } else {
    return false;
  }

}
void PMS7003Sensor::test(const char *ig) {
  PMS7003_framestruct thisFrame;
  _PMS7003Sensor.pms7003_read(0, 0, thisFrame);
}

bool PMS7003Sensor::pms7003_read(int rx, int tx, PMS7003_framestruct &thisFrame) {
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
    if (rx == 0 && tx == 0) {
      rx = _RX;
      tx = _TX;
    }
    Serial << F("PMS RX: ") << rx << F(", tx: ") << tx << endl;
    SoftwareSerialESP pms(rx, tx, 128); // RX, TX
    pms.begin(9600);
    bool packetReceived = false;
    uint32_t st = millis();
    while (!packetReceived && millis() - st < 3000) {
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
                    if (DEBUG) {
                      Serial.print(F("-- Frame syncing... "));
                      Serial.print(incomingByte, HEX);
                      Serial.println();
                    }
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
    return packetReceived;
}


void PMS7003Sensor::getData(LinkedList<Pair *> *data) {
 if (_RX < 0) return;
  //LOGGER << F("PMS7003Sensor::getData") << endl;
  PMS7003_framestruct thisFrame;
  if (pms7003_read(0, 0, thisFrame)) {
    data->add(new Pair("PM10", String(thisFrame.concPM10_0_amb)));
    data->add(new Pair("PM25", String(thisFrame.concPM2_5_amb)));
    data->add(new Pair("PM1", String(thisFrame.concPM1_0_amb)));
    data->add(new Pair("PM10C", String(thisFrame.concPM10_0_CF1)));
    data->add(new Pair("PM25C", String(thisFrame.concPM2_5_CF1)));
    data->add(new Pair("PM1C", String(thisFrame.concPM1_0_CF1)));
  }
}
