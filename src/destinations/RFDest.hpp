#ifndef RFDest_h
#define RFDest_h

#include "interfaces/Destination.hpp"
#include "interfaces/Plugin.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "MenuHandler.hpp"
#include "Arduino.h"


#define X10_RF_SB_LONG          8960 	// Start burts (leader) = 9ms
#define X10_RF_SB_SHORT         4500 	//Start silecence (leader) = 4,5 ms
#define X10_RF_BIT_LONG         1120 	// Bit 1 pulse length
#define X10_RF_BIT_SHORT         560 	// Bit 1 pulse length
#define X10_RF_GAP             40000 	// Length between commands

//#define PROP_CUSTOM_URL_ARR F("custom_url_arr")
class RFDest : public Destination {
public:
  RFDest();
  bool setup(MenuHandler *handler);
  void loop() {};

  bool process(LinkedList<Pair*> &data);
  void sendPing(int num);
  const char* getName() {
    return "RFX10";
  }

private:
  bool enabled = false;
//  int port = D6;
//  int port = D1;
  int getGPIO();
  void checkIfDefaultsAreSet();
  static void cmdSetAddr(const char *line);
  static void cmdTest(const char *line);

  void rfBegin(uint8_t tx_pin, uint8_t led_pin, uint8_t rf_repeats);
  void RFXmeter(uint8_t rfxm_address, uint8_t rfxm_packet_type, long rfxm_value);
  void RFXsensor(uint8_t rfxs_address,uint8_t rfxs_type, char rfxs_packet_type, uint8_t rfxs_value);
  void SendX10RfByte(uint8_t data);
  void SendX10RfBit(unsigned int databit);
  void SendCommand(uint8_t *date, uint8_t size);
  void SEND_HIGH();
  void SEND_LOW();
  uint8_t _tx_pin;
  uint8_t _led_pin;
  uint8_t _rf_repeats;
};

#endif
