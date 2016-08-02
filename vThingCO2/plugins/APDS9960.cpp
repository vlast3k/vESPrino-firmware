#ifdef VTHING_STARTER
#include "common.hpp"
#include "plugins\AT_FW_Plugin.hpp"

  #include <Wire.h>

  #include <SparkFun_APDS9960.h>

SparkFun_APDS9960 apds = SparkFun_APDS9960();
int isr_flag = 0;
#define APDS9960_INT    D7

void interruptRoutine() {
  isr_flag = 1;
}

void handleGesture() {
  if ( apds.isGestureAvailable() ) {
    char *gesture;
    switch ( apds.readGesture() ) {
      case DIR_UP:
        gesture = "UP";
//        handleCommandVESPrino("vecmd led_green");
        break;
      case DIR_DOWN:
        gesture = "DOWN";
//        handleCommandVESPrino("vecmd led_blue");
        break;
      case DIR_LEFT:
        gesture = "LEFT";
//        handleCommandVESPrino("vecmd led_yellow");
        break;
      case DIR_RIGHT:
        gesture = "RIGHT";
//        handleCommandVESPrino("vecmd led_orange");
        break;
      case DIR_NEAR:
        gesture = "NEAR";
//        handleCommandVESPrino("vecmd led_lila");
        break;
      case DIR_FAR:
        gesture = "FAR";
//        handleCommandVESPrino("vecmd led_cyan");
        break;
      default:
        gesture = "NONE";
    }
    menuHandler.handleCommand("vecmd led_green");
    menuHandler.handleCommand("vecmd ledmode_2_2");
    SERIAL << "Gesture: " << gesture << endl;
    char tmp[200], tmp2[200];
    char p2[40], p3[100];
    if(WiFi.status() == WL_CONNECTED && getJSONConfig("vespRFID", tmp, p2, p3)[0]) {
      if (!strcmp(tmp, "dw")) {
        String s = String(p3);
        s.replace("%s", gesture);
        //sprintf(tmp2, p3, gesture);
        sprintf(tmp, "http://dweet.io/dweet/for/%s?%s", p2, s.c_str());
      } else if (!strcmp(tmp, "if")) {
        sprintf(tmp2, p3, gesture);
        sprintf(tmp, "http://maker.ifttt.com/trigger/%s/with/key/%s", p2, p3);
      }  else {
        sprintf(tmp2, tmp, gesture);
        strcpy(tmp, tmp2);
      }
      String s = String(tmp);
      //s.replace(" ", "%20");
      SERIAL << "Sending to URL: " << s << endl;
      HTTPClient http;
      http.begin(s);
      //addHCPIOTHeaders(&http, token);
      int rc = AT_FW_Plugin::processResponseCodeATFW(&http, http.GET());
    }

  }
}

void checkForGesture() {
  if( isr_flag == 1 ) {
    detachInterrupt(APDS9960_INT);
    handleGesture();
    isr_flag = 0;
    attachInterrupt(APDS9960_INT, interruptRoutine, FALLING);
  }
}

void initAPDS9960() {
  pinMode(APDS9960_INT, INPUT);

  // Initialize interrupt service routine
  attachInterrupt(APDS9960_INT, interruptRoutine, CHANGE);

  // Initialize APDS-9960 (configure I2C and initial values)
  Wire.begin(D6, D1);
  if ( apds.init() ) {
    Serial.println(F("APDS-9960 initialization complete"));
  } else {
    Serial.println(F("Something went wrong during APDS-9960 init!"));
  }

  // Start running the APDS-9960 gesture sensor engine
  if ( apds.enableGestureSensor(true) ) {
    Serial.println(F("Gesture sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during gesture sensor init!"));
  }
}


#endif
