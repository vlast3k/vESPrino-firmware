#ifdef VTHING_VESPRINO
#include <PN532_I2C.h>
#include <PN532.h>
#include "common.hpp"
#include "plugins\AT_FW_Plugin.hpp"
#include <Wire.h>
PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);

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
#endif
