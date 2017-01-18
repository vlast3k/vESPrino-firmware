#include "MenuHandler.hpp"
#include <LinkedList.h>
#include <Arduino.h>
#include <Streaming.h>
#include "common.hpp"


MenuHandler::MenuHandler() {
  commands = new LinkedList<MenuEntry*>();
  registerCommand(new MenuEntry(F("help"), CMD_EXACT, MenuHandler::cmdHelp, F("This help page")));
  registerCommand(new MenuEntry(F("##"), CMD_BEGIN, MenuHandler::cmdListN, F("##cmd1##cmd2##..##cmdN##")));
  registerCommand(new MenuEntry(F("***"), CMD_BEGIN, MenuHandler::cmdListN, F("***cmd1***cmd2***..***cmdN***")));
  registerCommand(new MenuEntry(F("crc"), CMD_BEGIN, MenuHandler::cmdCRC, F("crc6Fcmd - XOR of all bytes of msg has to be == to 6F in this case")));

}

void MenuHandler::cmdCRC(const char *line) {
  char c_crc[3] = {line[3], line[4], 0};
  uint8_t expCrc = strtoul(c_crc, NULL, 16);

//  LOGGER << "Expected CRC is: " << _HEX(expCrc) << endl;
  uint8_t crc = 0;
  for (int i=5; line[i]; i++) crc ^= line[i];
//  LOGGER << "Computed LineCRC is: " << _HEX(crc) << endl;
  if (crc == expCrc) menuHandler.scheduleCommand(line+5);
  else LOGGER << F("ERROR: Bad CRC. Got 0x") << _HEX(crc) << F(", expected: 0x") << _HEX(expCrc) << endl;
}

void MenuHandler::cmdHelp(const char *ignore) {
  for (int i=0; i < menuHandler.commands->size(); i++) {
    LOGGER << menuHandler.commands->get(i)->cmd << F(" - ") << menuHandler.commands->get(i)->description << endl;
    LOGGER.flush();
    if ((i % 15) == 0) delay(110);
  }
}

void MenuHandler::registerCommand(MenuEntry *command) {
  //LOGGER << "registerCommand:" << command->cmd << endl;
  commands->add(command);
}

bool MenuHandler::processUserInput() {
//  LOGGER.setTimeout(5000);
  //if (!LOGGER.available()) {
  if (!Serial.available()) {
    return false;
  }

  //LOGGER.setTimeout(30000);
  if (readLine(2000) >= 0) {
    //LOGGER.flush();
    //LOGGER.setTimeout(5000);
    scheduleCommand(line);
    scheduleCommand("@nop");
    //LOGGER << endl << F("OK") << endl;
    return true;
  }
  LOGGER << endl << F("OK2") << endl;
  return false;
  //LOGGER.setTimeout(5000);
}

byte MenuHandler::readLine(int timeout) {
  unsigned long deadline = millis() + timeout;
  int i = 0;
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
  if (DEBUG)  LOGGER << F("schedule commands from prop: ") << prop << endl;
  char *list = PropertyList.readProperty(prop);
  if (*list) scheduleCommand(list);
}

void MenuHandler::cmdListN(const char *list) {
  char delimDash[] = "##";
  char delimStar[] = "***";
  char *delim;
  if (strstr(list, delimDash) == list) delim = delimDash;
  else delim = delimStar;
  LOGGER << "delim = " << delim << " " << strlen(delim);
  list = list + strlen(delim);
  char *x;
  while ((x = strstr(list, delim)) != NULL) {
    *x = 0;
    menuHandler.scheduleCommand(list);
    list = x+strlen(delim);
  }
  //if (DEBUG)  LOGGER << F("schedule command liast  from prop DONE") << endl;
}

void MenuHandler::scheduleCommand(const char *cmd) {
  if (cmd[0] && cmd[0] == '!') {
    handleCommand(cmd + 1);
  } else {
    String *s =new String(cmd);
    if (DEBUG) LOGGER << F("Scheduling command: " ) << *s << endl;
    pendingCommands.add(s);
  }
}

void MenuHandler::loop() {
  processUserInput();
  processCommands();
}

void MenuHandler::processCommands() {
  if (!schedulingEnabled) return;
  if (currentlyExecutingCommand) return;
  bool hadCommands = false;
  bool isSilent = true;
  while (pendingCommands.size() > 0) {
    String *s = pendingCommands.shift();
    isSilent = isSilent && (s->charAt(0) == '@');
    currentlyExecutingCommand = true;
    handleCommand(s->c_str());
    currentlyExecutingCommand = false;
    delete s;
    delay(1);
    hadCommands = true;
  }
   if (hadCommands && !isSilent)  {
  //   handleCommand("nop");
     LOGGER << F("ready >\n");
     LOGGER.flushLog();
   }
}

void MenuHandler::handleCommand(const char *line) {
  // check for silent command
  bool silent = false;
  if (line[0] == '@') {
    silent = true;
    line ++;
  }
  if (!silent && strcmp("nop", line)) {
    const char *x = line;
    for (int i=0; *x; x++, i++) {
      LOGGER << *x;
      if ((i%50) == 0) LOGGER.flush();
    }
    LOGGER << endl;
  //   LOGGER.flush();
  }

  String s1;
  for (int i=0; i < commands->size(); i++) {
    MenuEntry *m = commands->get(i);
    s1 = String(m->cmd);
    const char *cmd = s1.c_str();
    // LOGGER <<    cmd << endl;
    // LOGGER << m->cmd << endl;
    if ( (m->cmdExactMatch && !strcmp(line, cmd)) ||
        (!m->cmdExactMatch &&  strstr(line, cmd) == line) ) {
          if (m->handler) m->handler(line);
          else m->fhandler(line);
          return;
    }
  }
  if (line[0] && line[0] != 10 && line[0] != 13)
    LOGGER << F("ERROR: Command not recognized") << endl;
}
