#ifdef VTHING_CO2
#include <SoftwareSerialESP.h>
uint8_t cmdReadCO2[] = {4, 0x11, 0x01, 0x01, 0xed};
uint8_t resp[25];

void sendCmd(SoftwareSerialESP *PM1106_swSer, uint8_t *cmd, uint8_t *r) {
  r[3] = 0;
  r[4] = 0;
  PM1106_swSer->write(cmd+1, cmd[0]);
  delay(100);
  int i =0;
  for (i=0; i < 24 && PM1106_swSer->available(); i++){
    *(r++) = PM1106_swSer->read();
  }
}

void dump(uint8_t *r) {
  for (int i=0; i<24; i++) {
    SERIAL.print(*(r++), HEX);
    SERIAL.print(",");
  } 
  SERIAL.println();
}


unsigned long toLong(uint8_t *p) {
  return (((unsigned long)*p)<<24) +(((unsigned long)*(p+1))<<16) + (((unsigned long)*(p+2)) << 8) + *(p+3); 
}

int CM1106_read() {
  SoftwareSerialESP PM1106_swSer(D5, D7, 128);
  PM1106_swSer.begin(9600);
  sendCmd(&PM1106_swSer, cmdReadCO2, resp);
  sendCmd(&PM1106_swSer, cmdReadCO2, resp);
  //PM1106_swSer.end();
  if (DEBUG) {
    dump(resp);
    SERIAL << ((uint16_t)256)*resp[3] + resp[4] << endl;
  }
  return ((uint16_t)256)*resp[3] + resp[4];
}
void CM1106_init() {
  SoftwareSerialESP PM1106_swSer(D5, D7, 128);
  PM1106_swSer.begin(9600);
  sendCmd(&PM1106_swSer, cmdReadCO2, resp);
  sendCmd(&PM1106_swSer, cmdReadCO2, resp);
  dump(resp);
}

int CM1106__getCO2() {
  SoftwareSerialESP PM1106_swSer(D5, D7, 128);
  PM1106_swSer.begin(9600);
  sendCmd(&PM1106_swSer, cmdReadCO2, resp);
  dump(resp);
  SERIAL << ((uint16_t)256)*resp[3] + resp[4] << endl;
  return ((uint16_t)256)*resp[3] + resp[4];
}

#endif
