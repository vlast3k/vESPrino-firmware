#include "MenuHandler.hpp"
#include <LinkedList.h>
#include <Arduino.h>
#include <Streaming.h>
#include "common.hpp"


MenuHandler::MenuHandler() {
  commands = new LinkedList<MenuEntry*>();
  registerCommand(new MenuEntry(F("help"), CMD_EXACT, MenuHandler::cmdHelp, F("This help page")));
  registerCommand(new MenuEntry(F("##"), CMD_BEGIN, MenuHandler::cmdListN, F("##cmd1##cmd2##..##cmdN##")));

}

void MenuHandler::cmdHelp(const char *ignore) {
  for (int i=0; i < menuHandler.commands->size(); i++) {
    Serial << menuHandler.commands->get(i)->cmd << F(" - ") << menuHandler.commands->get(i)->description << endl;
    Serial.flush();
    if ((i % 15) == 0) delay(110);
  }
}

void MenuHandler::registerCommand(MenuEntry *command) {
  //Serial << "registerCommand:" << command->cmd << endl;
  commands->add(command);
}

bool MenuHandler::processUserInput() {
//  Serial.setTimeout(5000);
  //if (!Serial.available()) {
  if (!Serial.available()) {
    return false;
  }

  //Serial.setTimeout(30000);
  if (readLine(2000) >= 0) {
    //Serial.flush();
    //Serial.setTimeout(5000);
    scheduleCommand(line);
    scheduleCommand("nop");
    SERIAL_PORT << endl << F("OK") << endl;
    return true;
  }
  SERIAL_PORT << endl << F("OK2") << endl;
  return false;
  //Serial.setTimeout(5000);
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

void MenuHandler::scheduleCommandProperty(const char *prop) {
  if (DEBUG)  Serial << F("schedule commands from prop: ") << prop << endl;
  char *list = PropertyList.readProperty(prop);
  if (*list) scheduleCommand(list);
}

void MenuHandler::cmdListN(const char *list) {
  list = list + 2;
  char *x;
  while ((x = strstr(list, "##")) != NULL) {
    *x = 0;
    menuHandler.scheduleCommand(list);
    list = x+2;
  }
  //if (DEBUG)  Serial << F("schedule command liast  from prop DONE") << endl;
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
  if (!schedulingEnabled) return;
  bool hadCommands = false;
  while (pendingCommands.size() > 0) {
    String *s = pendingCommands.shift();
    handleCommand(s->c_str());
    delete s;
    delay(1);
    hadCommands = true;
  }
   if (hadCommands)  {
  //   handleCommand("nop");
     Serial << F("ready >");
   }
}

void MenuHandler::handleCommand(const char *line) {
  if (DEBUG) {
    Serial << F("Executing: ");
    const char *x = line;
    for (int i=0; *x; x++, i++) {
      Serial << *x;
      if ((i%50) == 0) Serial.flush();
    }
    Serial << endl;
    Serial.flush();
  }
  //dumpArray(line);
  String s1;
  for (int i=0; i < commands->size(); i++) {
    MenuEntry *m = commands->get(i);
    s1 = String(m->cmd);
    const char *cmd = s1.c_str();
    // Serial <<    cmd << endl;
    // Serial << m->cmd << endl;
    if ( (m->cmdExactMatch && !strcmp(line, cmd)) ||
        (!m->cmdExactMatch &&  strstr(line, cmd) == line) ) {
          m->handler(line);
          return;
    }
  }
  Serial << F("not found: ") << line << endl;
}
