#ifndef TimerManager_h
#define TimerManager_h

#include "interfaces\Plugin.hpp"
#include <LinkedList.h>
#include <Timer.h>

#define TMR_START 1
#define TMR_STOPPED 0
class TimerManagerClass : public Plugin {
public:
  Timer *registerTimer(Timer *t, int mode = TMR_START) {
    timers.add(t);
    if (mode == TMR_START) t->Start();
    return t;
  };
  void loop() {
    for (int i=0; i < timers.size(); i++) {
      timers.get(i)->Update();
    }
  }
  const char* getName() {
    return "TimerManager";
  }

private:
  LinkedList<Timer*> timers = LinkedList<Timer*>();

};

#endif
