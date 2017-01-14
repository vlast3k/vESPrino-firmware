#ifndef LoggingPrinterH
#define LoggingPrinterH
#include <Stream.h>
#include <Print.h>
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
    bool logToWss = false;
    void init();
    void sendData();
    void flushLog();
    void myWrite(const uint8_t *buffer, size_t size);
    void myWrite(uint8_t chr);
    size_t write(const uint8_t *buffer, size_t size) override;
    size_t write(uint8_t data) override;

    int available() override { return 0;};
    int read() override { return 0;};
    int peek() override { return 0;};
    void flush() override {Serial.flush();};
    void setLogToWSS(bool state) {logToWss = state;};
};
#endif
