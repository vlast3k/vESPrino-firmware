#define CMD_EXACT true
#define CMD_BEGIN false
#ifndef MenuHandler_h
#define MenuHandler_h
#include <Arduino.h>
#include <LinkedList.h>



typedef void (*MenuHandlerFunction)(const char *line);
class MenuEntry {
  public:
    MenuEntry(const __FlashStringHelper *_cmd, bool _cmdExactMatch, MenuHandlerFunction _handler, const __FlashStringHelper *_description) :
      cmd(_cmd), cmdExactMatch(_cmdExactMatch), handler(_handler), description(_description) {};

    const __FlashStringHelper *cmd;
    bool cmdExactMatch;
    MenuHandlerFunction handler;
    const __FlashStringHelper *description;
};

class MenuHandler {
  public:
    MenuHandler();
    void handleCommand(char *line);
    void registerCommand(MenuEntry *command);
    void listCommands();
    bool processUserInput();
    LinkedList<MenuEntry*> *commands;
  private:
    static void cmdHelp(const char *ignore);

    byte readLine(int);
    static const int LINE_LEN=250;
    char line[LINE_LEN];

};

#endif
