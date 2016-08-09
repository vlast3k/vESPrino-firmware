#ifdef VTHING_STARTER
#include "common.hpp"
#include "plugins\AT_FW_Plugin.hpp"
#include <Wire.h>
#include <Timer.h>
#include "PropertyList.hpp"

void onPIR() {
  Serial << "on PIR" << endl;
  handleDWCommand("oled_  MOVEMENT");
}

void onPIROff() {
  Serial << "on PIR off" << endl;
  handleDWCommand("oled_ NO COMMAND");
}

void handleCommandVESPrino(const char *line) {
  Serial <<"Vesprino cmd: " << line << endl;
  //      if (strstr(line, "vespBttnA "))  PropertyList.putProperty("vespBttn", strstr(line, " ")+1, true);
  // else if (strstr(line, "vespBttn "))   PropertyList.putProperty("vespBttn", strstr(line, " ")+1);
  // else if (strstr(line, "vespRFIDA"))   PropertyList.putProperty("vespRFID" , strstr(line, " ")+1, true);
  // else if (strstr(line, "vespRFID"))    PropertyList.putProperty("vespRFID" , strstr(line, " ")+1);
  // else if (strstr(line, "vespDWCmd"))   PropertyList.putProperty("vespDWCmd" , strstr(line, " ")+1);
  // else if (strstr(line, "vespSens"))    PropertyList.putProperty("vespSens", strstr(line, " ")+1);
  // else if (strstr(line, "vecmd "))    handleDWCommand(strstr(line, " ")+1);
}

void VESP_registerCommands(MenuHandler *handler) {
  Serial <<"register vesp\n";
  handler->registerCommand(new MenuEntry(F("vesp"), CMD_BEGIN, handleCommandVESPrino, F("")));
  handler->registerCommand(new MenuEntry(F("vecmd"), CMD_BEGIN, handleCommandVESPrino, F("")));
}

void handleDWCommand(char *line) {
       if (strstr(line, "oled_"))      oledHandleCommand(strstr(line, "_")+1);
  else if (strstr(line, "led_"))       ledHandleColor(strstr(line, "_")+1);
  else if (strstr(line, "ledbrg_"))    ledSetBrg(strstr(line, "_")+1);
  else if (strstr(line, "ledmode_"))   ledHandleMode(strstr(line, "_")+1);
  else if (strstr(line, "pirOn"))        onPIR();
  else if (strstr(line, "pirOff"))        onPIROff();
}

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
    if (PropertyList.readProperty("vespDWCmd", tmp)[0]) {
      SERIAL << F("Will read dweets from: http://dweet.io/get/latest/dweet/for/") << PropertyList.readProperty("vespDWCmd", tmp) << endl;
      SERIAL << F("Send commands to: https://dweet.io/dweet/for/") << PropertyList.readProperty("vespDWCmd", tmp) << F("vladi1?cmd=") << endl;
    } else {
      SERIAL << F("dweet id not set, use vespDWCmd <dweetid> to set it\n");

    }

  hasSSD1306 = checkI2CDevice(D5, D1, 0x3c);
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

  hasPIR = false;
  if(hasPIR) initPIR();


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
  if (hasPIR) handlePIR();
  loopNeoPixel();
}



#endif
