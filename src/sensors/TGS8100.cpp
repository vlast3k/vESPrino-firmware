#include <Wire.h>
#include "sensors/TGS8100.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "common.hpp"
#include <I2CHelper.hpp>

extern int16_t globalTemp;
extern int8_t  globalHum;

#define MSG_SET_TEMP_HUM 0x1
#define MSG_RESET 0x2

#define PROP_TGS8100_MAXR0 F("tgs81.maxr0")

TGS8100::TGS8100() {
  //enabled = true;
  registerSensor(this);
}

void TGS8100::onProperty(String &key, String &value) {
  if (key == PROP_TGS8100_MAXR0) {
    cMaxR0 = atol(value.c_str());
  }
}

bool TGS8100::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("tg8test"), CMD_EXACT, &TGS8100::test, F("TGS8100 toggle testSesnor")));
  handler->registerCommand(new MenuEntry(F("tg8rst"), CMD_EXACT, &TGS8100::reset, F("TGS8100 reset")));
//  enabled = PropertyList.readBoolProperty(F("test.sensor"));
  if (I2CHelper::i2cSDA != -1 && I2CHelper::checkI2CDevice(0x8)) enabled = true;
  if (!enabled) {
    LOGGER << F("Scanning for TGS8100 ");
    for (int i=0; i < 100; i++) {
      if ((i%10) == 0) LOGGER << ".";
      if (enabled = I2CHelper::checkI2CDevice(0x8)) break;
    }
    LOGGER << endl;
  }
  return enabled;
}

void TGS8100::reset(const char *ignore) {
  Wire.setClock(10000L);
  //bool x = I2CHelper::checkI2CDevice(0x8);
  Wire.beginTransmission(0x8);
  Wire.endTransmission();
  delay(10);
  Wire.beginTransmission(0x8);
  Wire.write(MSG_RESET);
  Wire.write(42);
  Wire.endTransmission();
}

uint8_t TGS8100::getCrc(uint8_t *data, int len) {
  uint8_t crc = 0;
  for (int i=0; i<len; i++) {
    crc ^= data[i];
  }
  return crc;
}


int TGS8100::readSensorValue(uint16_t &raw, uint16_t &rs, double &ppm) {
  Wire.setClock(10000L);
  //bool x = I2CHelper::checkI2CDevice(0x8);
  Wire.beginTransmission(0x8);
  Wire.endTransmission();
  delay(10);
  Wire.beginTransmission(0x8);
  Wire.write(MSG_SET_TEMP_HUM);
  Wire.write((uint8_t) (globalTemp >> 8));
  Wire.write((uint8_t) (globalTemp & 0xFF));
  Wire.write((uint8_t) (globalHum & 0xFF));
  Wire.write(42);
  Wire.endTransmission();
  //delay(50);
  //Serial << "tgs811 state: " << x << endl;
  delay(10);
  int len = Wire.requestFrom((uint8_t)8, (size_t)15, true);    // request 6 bytes from slave device #8
  Serial << "Received " << len << " bytes\n";
  uint8_t rcv[20];
  int i = 0;
  for (int i=0; i < len; i++) {
    rcv[i] = Wire.read();
    Serial << _HEX(rcv[i]) << ",";
  }
  Serial << endl;
  if (rcv[6] != 42) {
    Serial << "TGS8100: ERROR in i2c Communication" << endl;
    return 0;
  }

  uint8_t header = rcv[0];
  uint8_t crc = getCrc(rcv, 9);
  if (rcv[9] != crc) {
    Serial << "TGS8100: ERROR in i2c Communication" << endl;
    return 0;
  }

  uint32_t iterations = (rcv[1] << 24) + (rcv[2] << 16) + (rcv[3] << 8) + rcv[4];
  raw = (rcv[5] << 8) + rcv[6];
  //rs      = (rcv[2] << 8) + rcv[3];
  //uint16_t ppm100 = (rcv[4] << 8) + rcv[5];
  //rsAdj  = (rcv[6] << 8) + rcv[7];
  uint16_t vcc    = (rcv[7] << 8) + rcv[8];
  //maxR0 = (rcv[10] << 8) + rcv[11];
  //ppm = (float)ppm100/100;
  processData(raw, vcc, rs, ppm);
  onIteration(iterations);
  return raw;

}


#define TEMP_ADJ_0_10  2.0F
#define TEMP_ADJ_10_20 1.6F
#define TEMP_ADJ_20_30 1.3F
#define TEMP_ADJ_30_40 0.9F
double TGS8100::getTempAdj(float from, float to) {
  bool inv = false;
  if (from > to) {
    double t = to;
    to = from;
    from = t;
    inv = true;
  }
  double adj = 0;
  if (from < 10) adj += std::max(0.0F, (std::min(10.0f, to) - std::max(from, 0.0F) ) * TEMP_ADJ_0_10);
  if (from < 20) adj += std::max(0.0F, (std::min(20.0f, to) - std::max(from, 10.0F)) * TEMP_ADJ_10_20);
  if (from < 30) adj += std::max(0.0F, (std::min(30.0f, to) - std::max(from, 20.0F)) * TEMP_ADJ_20_30);
  if (from < 40) adj += std::max(0.0F, (std::min(40.0f, to) - std::max(from, 30.0F)) * TEMP_ADJ_30_40);

  if (inv) adj = -adj;
  return adj;
}

#define HUM_ADJ_0_20 1.0F
#define HUM_ADJ_20_40 0.9F
#define HUM_ADJ_40_65 0.5F
#define HUM_ADJ_65_85 0.3F
double TGS8100::getHumAdj(float from, float to) {
  bool inv = false;
  if (from > to) {
    double t = to;
    to = from;
    from = t;
    inv = true;
  }
  double adj = 0;
  if (from < 20) adj += std::max(0.0F, (std::min(20.0f, to) - std::max(from, 0.0F)) * HUM_ADJ_0_20);
  if (from < 40) adj += std::max(0.0F, (std::min(40.0f, to) - std::max(from, 20.0F)) * HUM_ADJ_20_40);
  if (from < 65) adj += std::max(0.0F, (std::min(65.0f, to) - std::max(from, 40.0F)) * HUM_ADJ_40_65);
  if (from < 85) adj += std::max(0.0F, (std::min(85.0f, to) - std::max(from, 65.0F)) * HUM_ADJ_65_85);

  if (inv) adj = -adj;
  return adj;
}

void TGS8100::processData(uint16_t value, uint16_t vcc, uint16_t &rs, double &ppm) {
//  uint16_t value = raVoc.getAverage();
  double VIN = (float)vcc / 1000  ;
  double vRfix = VIN* (double)value/1024;
  double RFIX = 220;
  //double R0 = 147; // in room air
  //double R0 = 185; //in outside
  double GRAD = 1.8;
  double cRs = (3.0F - vRfix)/vRfix*RFIX;
  double adjFactor = 100.0F + getTempAdj(20, globalTemp) + getHumAdj(40, globalHum);
  Serial << "TempAdj: " << getTempAdj(20, globalTemp) << endl;
  Serial << "HumAdj:  " << getHumAdj(40, globalHum)   << endl;
  adjFactor /= (double)100;
  cRsAdj = cRs * adjFactor;
  double rsr0 = cRsAdj/cMaxR0;
  ppm = pow(1.0F/rsr0, GRAD);
  uint16_t ppm100 = ppm*100;
  rs = cRs;
  uint16_t rsuadj = cRsAdj;
  Serial << "cTemp: " << globalTemp;
  Serial.flush();
  Serial << ", chum: " << globalHum << ", adjF:" << adjFactor << ", cRsAdj: " << cRsAdj << ", rs:" << cRs << endl;
  Serial.flush();
  Serial << "rsuadj: " << rsuadj << ", vcc: " << vcc << ", cMaxR0: " << cMaxR0 << endl;
}

extern TGS8100 _TGS8100;
void TGS8100::test(const char *ignore) {
  uint16_t raw, rs, rsa, r0;
  double ppm;
  Serial << _TGS8100.readSensorValue(raw, rs, ppm) << endl;

  // for (int i=0; i < 100; i++) {
  //   Serial << i << ": " << _TGS8100.readSensorValue() << endl;
  //   delay(5000);
  // }
}

void TGS8100::onIteration(uint32_t iterations) {
  // sensor needs 4h to warmup
  if (iterations < 400) return;
  if (cMaxR0 < (uint16_t)cRsAdj) {
    cMaxR0 = cRsAdj;
    String s = String(cMaxR0);
    PropertyList.putProperty(PROP_TGS8100_MAXR0, s.c_str());
  }
}

void TGS8100::getData(LinkedList<Pair *> *data) {
  if (!enabled) return;

  LOGGER << F("TGS8100::getData") << endl;
  uint16_t raw, rs;
  //uint8_t state;
  double ppm;
  int x = readSensorValue(raw, rs, ppm);

  if (x > 0) {
    data->add(new Pair("VOC_RAW", String(raw)));
    data->add(new Pair("VOC_RS", String(rs)));
    data->add(new Pair("VOC_PPM", String(ppm)));
    data->add(new Pair("VOC_APPM", String(ppm*400)));
    data->add(new Pair("VOC_RSA", String(cRsAdj)));
    data->add(new Pair("VOC_R0", String(cMaxR0)));
  }
}
