#include "commands/CommonCommands.hpp"
#include "MenuHandler.hpp"
#include <Arduino.h>
#include <Streaming.h>

void CommonCommands::cmdHeap(char *s) {
    Serial << "Heap: " << ESP.getFreeHeap() << endl;
}

void CommonCommands::registerCommands(MenuHandler *handler) {
  MenuEntry meHeap(F("heap"), CMD_EXACT, &CommonCommands::cmdHeap, F("Free heap"));

}
