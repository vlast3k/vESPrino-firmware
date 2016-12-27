#include "plugins/LoggingPrinter.hpp"
#include <Stream.h>
#include <Streaming.h>
#include "interfaces/Plugin.hpp"
#include "plugins/PropertyList.hpp"
#include <ESP8266HTTPClient.h>
#include "plugins/WebSocketServer.hpp"

extern WebSocketServerClass myWSS;
void LoggingPrinter::init() {
  logURL = PropertyList.readProperty(PROP_LOG_DEST);
  if (logURL.length() > 0) Serial << "Will log to: " << logURL << endl;
}

void LoggingPrinter::sendData() {
  if (logURL.length() == 0) return;
//      Serial << "Sending log...:" << String(data, length) << ":";
  Serial << "Sending log...";// << String((char*)data);
  Serial.flush();
  HTTPClient http;
  //http.setTimeout(1000);
  http.begin(logURL);
  //http.GET();
  http.POST(data, length);
  Serial << "sent" << endl;
}

void LoggingPrinter::flushLog() {
  sendData();
  length = 0;
}

void LoggingPrinter::myWrite(const uint8_t *buffer, size_t size) {
    if (logURL.length() == 0) return;
    while(size--)  myWrite(*buffer++);
}

void LoggingPrinter::myWrite(uint8_t chr) {
  if (logURL.length() == 0) return;
  data[length++] = chr;
  data[length] = 0;
  if (length == MAXSIZE - 1) flushLog();
}

size_t LoggingPrinter::write(const uint8_t *buffer, size_t size) {
  //Serial.write('[');
  Serial.write(buffer, size);
  if (logToWss) myWSS.sendData(data, size);
  //Serial.write(']');
  myWrite(buffer, size);
};

size_t LoggingPrinter::write(uint8_t data) {
  //Serial.write('[');
  Serial.write(data);
  if (logToWss) myWSS.sendData(data);

  //Serial.write(']');
  myWrite(data);
};
