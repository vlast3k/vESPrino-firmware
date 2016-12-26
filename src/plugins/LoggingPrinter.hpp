#ifndef LoggingPrinterH
#define LoggingPrinterH
#include <Stream.h>
#include <Streaming.h>
#include "interfaces/Plugin.hpp"
#include "plugins/PropertyList.hpp"
#include <ESP8266HTTPClient.h>


#define PROP_LOG_DEST F("log.dest.url")
#define MAXSIZE 1000

class LoggingPrinter : public Stream, public Plugin {
public:
    uint8_t data[MAXSIZE];
    String logURL;
    int length = 0;

    void init() {
      logURL = PropertyList.readProperty(PROP_LOG_DEST);
    }

    void sendData() {
      if (logURL.length() == 0) return;
      Serial << "Sending log...";
      HTTPClient http;
      http.begin(logURL);
      http.POST(data, length);
      Serial << "sent" << endl;
    }

    void flushLog() {
      sendData();
      length = 0;
    }

    void myWrite(const uint8_t *buffer, size_t size) {
        if (logURL.length() == 0) return;
        while(size--)  myWrite(*buffer++);
    }

    void myWrite(uint8_t chr) {
      if (logURL.length() == 0) return;
      data[length++] = chr;
      data[length] = 0;
      if (length == MAXSIZE - 1) flushLog();
    }

    size_t write(const uint8_t *buffer, size_t size) override {
      Serial.write(buffer, size);
      myWrite(buffer, size);
    };

    size_t write(uint8_t data) override {
      Serial.write(data);
      myWrite(data);
    };

    int available() override { return 0;};
    int read() override { return 0;};
    int peek() override { return 0;};
    void flush() override {Serial.flush();};
};
#endif
