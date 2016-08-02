#ifdef VTHING_STARTER
#include "common.hpp"
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

SSD1306  *display = NULL;

void oledHandleCommand(char *cmd) {
  Serial <<"1\n";
  if (!hasSSD1306)  return;
  Serial <<"2\n";
  if (!display) display = new SSD1306(0x3c, D5, D1);
  Serial <<"3\n";

  display->init();
  display->flipScreenVertically();
  display->setTextAlignment(TEXT_ALIGN_LEFT);
  if (strlen(cmd) < 8) {
    display->setFont(ArialMT_Plain_24);
  } else {
    display->setFont(ArialMT_Plain_16);
  }
  display->drawString(0, 0, cmd);
  display->display();
  Serial <<"4\n";

}
#endif
