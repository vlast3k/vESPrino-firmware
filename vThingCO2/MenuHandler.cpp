#include "MenuHandler.hpp"
#include <LinkedList.h>
#include <Arduino.h>

MenuHandler::MenuHandler() {
  commands = new LinkedList<MenuEntry*>();
}

void MenuHandler::registerCommand(MenuEntry *command) {
  commands->add(command);
}

bool MenuHandler::processUserInput() {
  Serial.setTimeout(500);
  if (!Serial.available()) {
    return false;
  }

  Serial.setTimeout(30000);
  if (readLine(30000) >= 0) {
    Serial.flush();
    handleCommand(line);
   // SERIAL << endl << F("OK") << endl;
   return true;
  }
}

byte MenuHandler::readLine(int timeout) {
  unsigned long deadline = millis() + timeout;
  byte i = 0;
  while (millis() < deadline) {
    if (Serial.available()) {
      line[i++] = (char) Serial.read();
      if      (line[i-1] == '\r')  i--;
      else if (i == LINE_LEN - 1)  break;
      else if (line[i-1] == '\n')  {i--; break;}
    }
  }
  line[i] = 0;
  return i;
}

void MenuHandler::handleCommand(char *line) {
  for (int i=0; i < commands->size(); i++) {
    MenuEntry *m = commands->get(i);
    if ( (m->cmdExactMatch && !strcmp(String(m->cmd).c_str(), String(line).c_str())) ||
        (!m->cmdExactMatch &&  strstr(String(line).c_str(), String(m->cmd).c_str())) ) {
          m->handler(line);
    }
  }
}
