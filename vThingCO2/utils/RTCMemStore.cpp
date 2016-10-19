#include "utils\RTCMemStore.hpp"
#include "Streaming.h"
bool RTCMemStore::test = false;
RTCData *RTCMemStore::rtcData = NULL;
bool RTCMemStore::dataExisted=false;

void RTCMemStore::init() {
  dataExisted = readData();
  updateData();
}

uint8_t RTCMemStore::getIterations() {
  readData();
  uint8_t x = rtcData->interations;
  updateData();
  return x;
}

void RTCMemStore::setIterations(uint8_t i) {
  readData();
  rtcData->interations = i;
  updateData();
}

double RTCMemStore::getAverage() {
  readData();
  double x = rtcData->avg1.getAverage();
  updateData();
  return x;
}

void RTCMemStore::addAverageValue(double val, int weight) {
  readData();
  for (int i=0; i < weight; i++) rtcData->avg1.addValue(val);
  updateData();
}

uint32_t RTCMemStore::getMillis() {
  readData();
  uint32_t x = rtcData->millisStartIteration;
  updateData();
  return x;
}

void RTCMemStore::addMillis(uint32_t ms) {
  readData();
  rtcData->millisStartIteration += ms;
  updateData();
}

uint32_t RTCMemStore::getGenData(int idx) {
  readData();
  uint32_t x = rtcData->genData[idx];
  updateData();
  return x;
}

void RTCMemStore::setGenData(int idx, uint32_t value){
  readData();
  rtcData->genData[idx] = value;
  updateData();
}


bool RTCMemStore::wasInDeepSleep() {
  readData();
  bool res = rtcData->inDeepSleep == 1;
  updateData();
  return res;
}
void RTCMemStore::setDeepSleep(bool state) {
  readData();
  rtcData->inDeepSleep = state ? 1:0;
  updateData();
}

uint32_t RTCMemStore::calculateCRC32(const uint8_t *data, size_t length) {
  uint32_t crc = 0xffffffff;
  while (length--) {
    uint8_t c = *data++;
    for (uint32_t i = 0x80; i > 0; i >>= 1) {
      bool bit = crc & 0x80000000;
      if (c & i) {
        bit = !bit;
      }
      crc <<= 1;
      if (bit) {
        crc ^= 0x04c11db7;
      }
    }
  }
  return crc;
}

bool RTCMemStore::readData() {
  rtcData = new RTCData();
  //RTCData2 r2;
  //Serial << "sizeof rtcdat: " <<RTCData::rtcDataSize() << endl;
  if (ESP.rtcUserMemoryRead(0, (uint32_t*) rtcData, RTCData::rtcDataSize())) {
    uint32_t crcOfData = calculateCRC32(((uint8_t*) rtcData) + 4, RTCData::rtcDataSize() - 4);
    if (crcOfData != rtcData->crc32) {
    //  Serial.println(F("CRC32 in RTC memory doesn't match CRC32 of data. Data is probably invalid!"));
      delete rtcData;
      rtcData = new RTCData();
      return false;
    } else {
    //  Serial.println(F("CRC32 check ok, data is probably valid."));
      return true;
    }
  }
  return false;
}

void RTCMemStore::updateData() {
  uint32_t cnew = calculateCRC32(((uint8_t*) rtcData) + 4, RTCData::rtcDataSize() - 4);
  if (cnew != rtcData->crc32) {
    rtcData->crc32 = cnew;
    ESP.rtcUserMemoryWrite(0, (uint32_t*) rtcData, RTCData::rtcDataSize());
  }
  delete rtcData;
}

bool RTCMemStore::getDataExisted() {
  return  RTCMemStore::dataExisted;
}
