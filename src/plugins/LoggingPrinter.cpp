#include "plugins/LoggingPrinter.hpp"
#include <Stream.h>
#include <Streaming.h>
#include "interfaces/Plugin.hpp"
#include "plugins/PropertyList.hpp"
#include <ESP8266HTTPClient.h>
#include "plugins/WebSocketServer.hpp"
#include <Hash.h>
#include "plugins/WifiStuff.hpp"
extern WifiStuffClass WifiStuff;

extern WebSocketServerClass myWSS;
void LoggingPrinter::init() {
  logURL = PropertyList.readProperty(PROP_LOG_DEST);
  if (logURL.length() > 0) {
    Serial << F("Will log to: ") << logURL << endl;
    Serial.flush();
    //waitForWifi();
  }
}

void LoggingPrinter::sendData() {
  if (!length) return;
  if (logURL.length() > 0 && WifiStuff.waitForWifi() == WL_CONNECTED) {
    Serial << F("Sending log...") << length << F(":") << logURL << endl;// << String((char*)data);
    HTTPClient http;
    //heap("");
    if (!http.begin(logURL)) {
      Serial << F("FAILED TO BEGIN SEND LOG") << endl;
    };
    int res = http.POST(data, length);
    Serial << F("sent: ") << res << endl;

  }
  if (logToWss && WifiStuff.waitForWifi() == WL_CONNECTED) {
    //Serial << "Sending wss:" << _HEX(*data) << endl;
    myWSS.sendData(data, length);
  }
}

void LoggingPrinter::flushLog() {
  sendData();
  length = 0;
  data[0] = 0;
}

void LoggingPrinter::myWrite(const uint8_t *buffer, size_t size) {
    if (logURL.length() == 0 && !logToWss) return;
    while(size--)  myWrite(*buffer++);
}

void LoggingPrinter::myWrite(uint8_t chr) {
  if (logURL.length() == 0 && !logToWss) return;
  //Serial << '_' << chr;
  if (length == MAXSIZE -1){
    return; // the case when flushLog connects to Wifi, and it dumps
  }
  data[length++] = chr;
  data[length] = 0;
  //if (logToWss && data[length-1] == '\n') flushLog();
  if (length == MAXSIZE - 50) flushLog(); // flush a bit earlier ot allow for Wifi connection to be logged
}

size_t LoggingPrinter::write(const uint8_t *buffer, size_t size) {
//  Serial.write('{');
  Serial.write(buffer, size);
  //if (logToWss) myWSS.sendData(data, size);
  //Serial.write('}');
  myWrite(buffer, size);
};

size_t LoggingPrinter::write(uint8_t data) {
  //Serial.write('{');
  Serial.write(data);
  if (data == '\n') Serial.flush();
  //if (logToWss) myWSS.sendData(data);
  //Serial.write('}');
  myWrite(data);
};
