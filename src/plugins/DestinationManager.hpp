#ifndef DestinationManager_H
#define DestinationManager_H
#include "common.hpp"
#include "PropertyList.hpp"
#include "interfaces/Plugin.hpp"

class DestinationManagerClass : public Plugin {
public:
  DestinationManagerClass() ;
  bool setup(MenuHandler *handler);
  bool setupInt(MenuHandler *handler);
  const char* getName() {
    return "DestinationManager";
  }
  void loop();
  static void cmdStart(const char *line);
  void cmdStartInst(const char *line);
  void onProperty(String &key, String &value);
  void addCommonValues(LinkedList<Pair *> *data);
  void conditionalSend(bool forceSend);
  static void onRawRead() ;
  static void cmdSendNow(const char* ignore);
  static void cmdSendNowCond(const char* ignore);
  static void setSendInterval(const char *line);
  void onIterationStart();
  bool getWillSendThisIteration() {
    return willSendThisIteration;
  }
  void readSensorValues(LinkedList<Pair *> *values);
  bool sendDataToDestinations(LinkedList<Pair *> *values);
private:
  bool willSendThisIteration;
};
#endif
