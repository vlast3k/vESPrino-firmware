#ifdef VTHING_STARTER
#include "common.hpp"
#include "plugins\AT_FW_Plugin.hpp"

  #include <Wire.h>
  #include <PN532_I2C.h>
  #include <PN532.h>
  #include <SparkFun_APDS9960.h>
  #include <Timer.h>
  void initVThingStarter();
  void loopVThingStarter();
  PN532_I2C pn532i2c(Wire);
  PN532 nfc(pn532i2c);
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


void initPN532() {
  Wire.begin(D5, D7);
  nfc.begin();
  delay(1200);
  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    delay(100);
    while (1); // halt
  }
  // Got ok data, print it out!
  Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX);
  Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC);
  Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);

  // configure board to read RFID tags
  //nfc.setPassiveActivationRetries(0x19);
  nfc.SAMConfig();

  Serial.println("Waiting for an ISO14443A Card ...");

}

void    checkForNFCCart() {
  uint8_t success;
  uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
  uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)
  static uint8_t prevUid[10];
  static uint32_t prevMillis=0;

  // Wait for an ISO14443A type cards (Mifare, etc.).  When one is found
  // 'uid' will be populated with the UID, and uidLength will indicate
  // if the uid is 4 bytes (Mifare Classic) or 7 bytes (Mifare Ultralight)
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    // Display some basic information about the card
    if ((millis() - prevMillis < 1000) && ! memcmp(uid, prevUid, uidLength)) return;
    Serial.println("Found an ISO14443A card");
    Serial.print("  UID Length: ");Serial.print(uidLength, DEC);Serial.println(" bytes");
    Serial.print("  UID Value: ");
    nfc.PrintHex(uid, uidLength);
    memcpy(prevUid, uid, uidLength);
    prevMillis = millis();
    Serial.println("");

    char tmp[200], tmp2[200];
    char p2[40], p3[100];
    if(WiFi.status() == WL_CONNECTED && getJSONConfig("vespRFID", tmp, p2, p3)[0]) {
      char suid[20];
      for (int i=0; i < uidLength; i++) sprintf(&suid[i*2], "%02X", uid[i]);
      //sprintf(suid, "%02X%02X%02X%02X", uid[0],uid[1],uid[2],uid[3]);
      if (!strcmp(tmp, "dw")) {
        sprintf(tmp2, p3, suid);
        sprintf(tmp, "http://dweet.io/dweet/for/%s?%s", p2, tmp2);
      } else if (!strcmp(tmp, "if")) {
        sprintf(tmp2, p3, suid);
        sprintf(tmp, "http://maker.ifttt.com/trigger/%s/with/key/%s", p2, p3);
      }  else {
        sprintf(tmp2, tmp, suid);
        strcpy(tmp, tmp2);
      }
      SERIAL << "Sending to URL: " << tmp << endl;
      HTTPClient http;
      http.begin(tmp);
      //addHCPIOTHeaders(&http, token);
      int rc = AT_FW_Plugin::processResponseCodeATFW(&http, http.GET());
    }
  }
}

boolean checkI2CDevice(int sda, int sca, int addr) {
  Wire.begin(sda, sca);
  Wire.beginTransmission(addr);
  int res = Wire.endTransmission();
  SERIAL << "i2c addr:" << addr << ", i2c res: " << _HEX(res) << endl;
  return !res;
}

void onGetDweets();
Timer *tmrTempRead, *tmrCheckPushMsg, *tmrGetDweets;
void initVThingStarter() {
   // strip = new NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> (1, D4);
    //handleCommandVESPrino("vecmd led lila");
//    strip->Begin();
//    strip->SetPixelColor(0, RgbColor(20, 0, 10));
//    strip->Show();

  si7021init();
  dumpTemp();
  tmrGetDweets = new Timer(2000L,    onGetDweets);
  tmrTempRead = new Timer(15000L,    onTempRead);
  //tmrCheckPushMsg = new Timer(1000L, handleSAP_IOT_PushService);
  tmrTempRead->Start();
  //tmrCheckPushMsg->Start();
  tmrGetDweets->Start();
  attachButton();

      char tmp[200];
    if (getJSONConfig("vespDWCmd", tmp)[0]) {
      SERIAL << F("Will read dweets from: http://dweet.io/get/latest/dweet/for/") << getJSONConfig("vespDWCmd", tmp) << endl;
      SERIAL << F("Send commands to: https://dweet.io/dweet/for/") << getJSONConfig("vespDWCmd", tmp) << F("vladi1?cmd=") << endl;
    } else {
      SERIAL << F("dweet id not set, use vespDWCmd <dweetid> to set it\n");

    }

  hasSSD1306 = checkI2CDevice(D7, D5, 0x3c);
  hasPN532   = checkI2CDevice(D5, D7, 0x24);
  hasSI7021  = checkI2CDevice(D1, D6, 0x40);
  hasBMP180  = checkI2CDevice(D1, D6, 0x77);
  hasBH1750  = checkI2CDevice(D1, D6, 0x23);
  hasAPDS9960= checkI2CDevice(D6, D1, 0x39);
  SERIAL << "oled: " << hasSSD1306 << ", si7021: " << hasSI7021 << ", pn532: " << hasPN532 << ", bmp180: " << hasBMP180 << ", BH1750: " << hasBH1750 << endl;
  //initSSD1306();
  if (hasPN532) initPN532();
  if (hasSSD1306) oledHandleCommand("     vESPrino\n  IoT made easy\nPlay with sensors");
  menuHandler.handleCommand("vecmd led_black");
  //handleCommandVESPrino("vecmd ledmode 1");
  //  pinMode(2, OUTPUT);
  if (hasAPDS9960) initAPDS9960();

  if(!hasAPDS9960)   attachButton();


}

void loopVThingStarter() {
  if (hasSI7021) tmrTempRead->Update();
//    tmrCheckPushMsg->Update();
    tmrGetDweets->Update();
    //doSend();
    checkButtonSend();
//
//    delay(1000);
  if (hasPN532) checkForNFCCart();
  if (hasAPDS9960) checkForGesture();
  loopNeoPixel();
}

#include <Si7021.h>
SI7021 *si7021;
boolean si7021init() {
    si7021 = new SI7021();
    si7021->begin(D1, D6); // Runs : Wire.begin() + reset()
    if (si7021->getDeviceID() == 255) {
      delete si7021;
      si7021 = NULL;
      return false;
    }
    SERIAL << "Found SI7021 Temperature/Humidity Sensor\n" << endl;
    si7021->setHumidityRes(8); // Humidity = 12-bit / Temperature = 14-bit
    return true;
}

void onTempRead() {
  if (!si7021) return;
  float tmp = si7021->readTemp();
  float hum = si7021->readHumidity();
  SERIAL << F("Humidity : ") << hum << " %\t";
  SERIAL << F("Temp : "    ) << tmp << " C" << endl;

  String s = String("sndiot ") + tmp;
  sndIOT(s.c_str());
}

void dumpTemp() {
  if (!si7021) return;
  SERIAL << F("Temp : ")     << si7021->readTemp() << " C" << endl;
}

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
  if (getJSONConfig("vespBttn", tmp, p2, p3)[0]) {
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

//void httpGET(String url) {
//    if(WiFi.status() == WL_CONNECTED) {
//        HTTPClient http;
//
//        SERIAL.print("[HTTP] begin...\n");
//        http.begin(url); //HTTP
//
//        SERIAL.print("[HTTP] GET...\n");
//        // start connection and send HTTP header
//        int httpCode = http.GET();
//
//        // httpCode will be negative on error
//        if(httpCode > 0) {
//            // HTTP header has been send and Server response header has been handled
//            SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
//            String payload = http.getString();
//            SERIAL.println(payload);
//        } else {
//            SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
//        }
//
//        http.end();
//    }
//}

void processMessage(String msgIn) {
  StaticJsonBuffer<200> jsonBuffer;
  char msg[301];
  strncpy(msg, msgIn.c_str(), 300);
  JsonArray& root = jsonBuffer.parseArray(msg);
  //[{"messageType":"42c3546a088b3ef8b8d3","sender":"IoT App","messages":[{"command":"switch on"}]}]

  if (!root.success()) {
    //if server has returned empyy response
    if (DEBUG) SERIAL << F("parseObject() failed: ") << msgIn << endl;
    return;
  }
  //SERIAL.print(root[0].is<JsonObject&>());
  //SERIAL  << "type:" << root.get(0) << endl;
  if (root[0].is<JsonObject&>()) {
    //SERIAL << root[0].asObject().containsKey("messages") << endl;
    //SERIAL << root[0]["messages"].is<JsonArray&>() << endl;
    //SERIAL << root[0]["messages"][0].is<JsonObject&>() << endl;
    //SERIAL << root[0]["messages"][0].asObject().containsKey("color") << endl;
    //SERIAL << " Received cmd: " << root[0]["messages"][0]["color"].asString() << endl;
    if (root[0]["messages"][0]["color"].asString()) {
      //SERIAL << "to process command" << endl;
      processCommand(root[0]["messages"][0]["color"].asString());
    } else {
      SERIAL << F("command not recognized") << endl;
     }
  }
//  else {
//    SERIAL << "no cmd" << endl;
//  }

}

#endif
