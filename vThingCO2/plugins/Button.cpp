#include "common.hpp"
#include "plugins\AT_FW_Plugin.hpp"
#include "plugins\PropertyList.hpp"

int BTTN_PIN = D3;
void ex2_attachInterrupt(void (*)());

uint32_t lastBttn=0;
void onButton() {
  if (digitalRead(BTTN_PIN) == 0) {
    shouldSend = true;
  }
}

void attachButton() {
  //pinMode(BTTN_PIN, INPUT_PULLUP);
  attachInterrupt(BTTN_PIN, onButton, CHANGE);
}

void checkButtonSend() {
  if (shouldSend == false) return;
  shouldSend = false;
  //digitalWrite(2, LOW);
  char tmp[200];
  SERIAL << F("Button Clicked!") << endl;
  if(WiFi.status() != WL_CONNECTED) {
    SERIAL << F("Will not send: No WiFi") << endl;
    return;
  }
  char p2[40], p3[40];
 // SERIAL <<"getjscfg" <<getJSONConfig("vespBttn", p1, p2, p3)[0] << "!" << endl;
  if (PropertyList.readProperty("vespBttn",  p3)[0]) {
    if (!strcmp(tmp, "dw")) {
      sprintf(tmp, "http://dweet.io/dweet/for/%s?%s", p2, p3);
    } else if (!strcmp(tmp, "if")) {
      sprintf(tmp, "http://maker.ifttt.com/trigger/%s/with/key/%s", p2, p3);
    }

    SERIAL << "Sending to URL: " << tmp << endl;
    HTTPClient http;
    http.begin(tmp);
    //addHCPIOTHeaders(&http, token);
    int rc = AT_FW_Plugin::processResponseCodeATFW(&http, http.GET());
  }
}

int clicks = 5;
