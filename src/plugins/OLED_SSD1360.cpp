#include "common.hpp"
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`

SSD1306  *display = NULL;

void oledHandleCommand(const char *cmd) {
  //if (!hasSSD1306)  return;
  cmd = strchr(cmd, ' ');
  if (!display) {
     display = new SSD1306(0x3c, D5, D1);
     display->init();
     display->flipScreenVertically();
     display->setTextAlignment(TEXT_ALIGN_LEFT);
   }
   display->clear();
  if (strlen(cmd) < 8) {
    display->setFont(ArialMT_Plain_24);
  } else {
    display->setFont(ArialMT_Plain_16);
  }
  display->drawString(0, 0, cmd);
  display->display();
}
