#include "MenuHandler.hpp"
#include <LinkedList.h>
#include <Arduino.h>
#include <Streaming.h>
#include "common.hpp"


MenuHandler::MenuHandler() {
  commands = new LinkedList<MenuEntry*>();
  registerCommand(new MenuEntry(F("help"), CMD_EXACT, MenuHandler::cmdHelp, F("This help page")));

}

void MenuHandler::cmdHelp(const char *ignore) {
  for (int i=0; i < menuHandler.commands->size(); i++) {
    Serial << menuHandler.commands->get(i)->cmd << " - " << menuHandler.commands->get(i)->description << endl;
  }
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
    scheduleCommand(line);
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

void MenuHandler::scheduleCommand(const char *cmd) {
  String *s =new String(cmd);
  pendingCommands.add(s);
}

void MenuHandler::loop() {
  processUserInput();
  processCommands();
}

void MenuHandler::processCommands() {
  bool hadCommands = false;
  while (pendingCommands.size() > 0) {
    String *s = pendingCommands.shift();
    handleCommand(s->c_str());
    delete s;
    delay(1);
    hadCommands = true;
  }
  if (hadCommands)  handleCommand("nop");
}

void MenuHandler::handleCommand(const char *line) {
  Serial << F("Executing: ") << line << endl;
  for (int i=0; i < commands->size(); i++) {
    MenuEntry *m = commands->get(i);
    const char *cmd = String(m->cmd).c_str();
    if ( (m->cmdExactMatch && !strcmp(line, cmd)) ||
        (!m->cmdExactMatch &&  strstr(line, cmd) == line) ) {
          m->handler(line);
          return;
    }
  }
  Serial << F("not found") << endl;
}
