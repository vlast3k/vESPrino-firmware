#include "common.hpp"
#include "plugins/AT_FW_Plugin.hpp"
#include "plugins/PropertyList.hpp"
#include "plugins/Button.hpp"
#include "interfaces/Plugin.hpp"

int BTTN_PIN = D3;
//void ex2_attachInterrupt(void (*)());


ButtonClass::ButtonClass() {
  registerPlugin(this);
}

//uint32_t lastBttn=0;
uint32_t bttnClicked=0;
// void onButton() {
//   //Serial << digitalRead(BTTN_PIN) << endl;
//   if (digitalRead(BTTN_PIN) == 0) {
//     shouldSend = true;
//   }
// }

void setLedColorButton(int func) {
  String colors[] = {F("red"),F("blue"),F("green"),F("yellow")};
  String cmd1 = String("ledcolor ") + colors[func%4];
  menuHandler.handleCommand(cmd1.c_str());

}


enum ButtonModes  {BTTN_IDLE, BTTN_CYCLE, BTTN_CONFIRM, BTTN_EXECUTED};
int bttnMode = BTTN_IDLE;
void ButtonClass::loop() {
  static int lastFunc = -1;
  static uint32_t lastPressed = 0;
  if (digitalRead(D3) == 0) { //Button pressed
    switch (bttnMode) {
      case BTTN_IDLE: {
        menuHandler.setSchedulingEnabled(false);
        bttnClicked = millis();
        menuHandler.handleCommand("ledbrg 100");
        bttnMode = BTTN_CYCLE;
        break;
      }
      case BTTN_CYCLE: {
        uint32_t pressedTime = millis() - bttnClicked;
        int func = pressedTime / BTTN_FUNC_INTERVAL;
        int intensityDist = abs(BTTN_FUNC_INTERVAL/2 - (pressedTime % BTTN_FUNC_INTERVAL));
      //  Serial << "presstime: " << pressedTime << ", intensityDist: " << intensityDist << endl;
        double intensityPerc = (double)intensityDist / (BTTN_FUNC_INTERVAL/2);
        //Serial << "presstime: " << pressedTime << ", intensityDist: " << intensityDist << " perc: " << intensityPerc << endl;
        int ledBrgValue = ledBright +  intensityPerc* (ledDark - ledBright);
        if (lastFunc != func) {
          setLedColorButton(func);
          lastFunc = func;
        }
        String cmd2 = String("ledbrg ") + ledBrgValue;
        menuHandler.handleCommand(cmd2.c_str());
        break;
      }
      case BTTN_CONFIRM: {
        bttnClicked = millis();
        Serial << "Executed: " << lastFunc << endl;
        String event = String("onBttnFunc_") + lastFunc;
        fireEvent(event.c_str());
        bttnMode = BTTN_EXECUTED;
        break;
      }
      case BTTN_EXECUTED: break;
      }
  } else {
    switch (bttnMode) {
      case BTTN_IDLE: break;
      case BTTN_CYCLE:
        setLedColorButton(lastFunc);
        menuHandler.handleCommand("ledbrg 80");
        lastPressed = millis();
        bttnMode = BTTN_CONFIRM;
        break;
      case BTTN_CONFIRM:
        if ((millis() - lastPressed < 5000)) break;
      case BTTN_EXECUTED:
        menuHandler.handleCommand("ledcolor black");
        menuHandler.setSchedulingEnabled(true);
        lastPressed = 0;
        lastFunc = -1;
        bttnMode = BTTN_IDLE;
        break;
    }
  }
}

// void attachButton() {
//   //pinMode(BTTN_PIN, INPUT_PULLUP);
//   attachInterrupt(BTTN_PIN, onButton, CHANGE);
// }

// void checkButtonSend() {
//   if (shouldSend == false) return;
//   shouldSend = false;
//   //digitalWrite(2, LOW);
//   char tmp[200];
//   SERIAL_PORT << F("Button Clicked!") << endl;
//   if(WiFi.status() != WL_CONNECTED) {
//     SERIAL_PORT << F("Will not send: No WiFi") << endl;
//     return;
//   }
//   char p2[40], p3[40];
//  // SERIAL_PORT <<"getjscfg" <<getJSONConfig("vespBttn", p1, p2, p3)[0] << "!" << endl;
//   if (PropertyList.hasProperty("vespBttn")) {
//     if (!strcmp(tmp, "dw")) {
//       sprintf(tmp, "http://dweet.io/dweet/for/%s?%s", p2, p3);
//     } else if (!strcmp(tmp, "if")) {
//       sprintf(tmp, "http://maker.ifttt.com/trigger/%s/with/key/%s", p2, p3);
//     }
//
//     SERIAL_PORT << "Sending to URL: " << tmp << endl;
//     HTTPClient http;
//     http.begin(tmp);
//     //addHCPIOTHeaders(&http, token);
//     int rc = AT_FW_Plugin::processResponseCodeATFW(&http, http.GET());
//   }
// }
//
// int clicks = 5;
