#include <SoftwareSerialESP.h>
SoftwareSerialESP PM1106_swSer(D5, D7, 128);
uint8_t cmdReadCO2[] = {4, 0x11, 0x01, 0x01, 0xed};
uint8_t resp[25];

void sendCmd(uint8_t *cmd, uint8_t *r) {
  r[3] = 0;
  r[4] = 0;
  PM1106_swSer.write(cmd+1, cmd[0]);
  delay(100);
  int i =0;
  for (i=0; i < 24 && PM1106_swSer.available(); i++){
    *(r++) = PM1106_swSer.read();
  }
}

void dump(uint8_t *r) {
  for (int i=0; i<24; i++) {
    Serial.print(*(r++), HEX);
    Serial.print(",");
  } 
  Serial.println();
}


unsigned long toLong(uint8_t *p) {
  return (((unsigned long)*p)<<24) +(((unsigned long)*(p+1))<<16) + (((unsigned long)*(p+2)) << 8) + *(p+3); 
}

int CM1106_read() {
  PM1106_swSer.begin(9600);
  sendCmd(cmdReadCO2, resp);
  sendCmd(cmdReadCO2, resp);
  //PM1106_swSer.end();
  //dump(resp);
  //Serial << ((uint16_t)256)*resp[3] + resp[4] << endl;
  return ((uint16_t)256)*resp[3] + resp[4];
}
void CM1106_init() {
  PM1106_swSer.begin(9600);
  sendCmd(cmdReadCO2, resp);
  sendCmd(cmdReadCO2, resp);
  dump(resp);
}

int CM1106__getCO2() {
  sendCmd(cmdReadCO2, resp);
  dump(resp);
  Serial << ((uint16_t)256)*resp[3] + resp[4] << endl;
  return ((uint16_t)256)*resp[3] + resp[4];
}

