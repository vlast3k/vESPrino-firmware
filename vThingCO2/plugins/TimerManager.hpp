#ifndef TimerManager_h
#define TimerManager_h

#include "interfaces\Plugin.hpp"
#include <LinkedList.h>
#include <Timer.h>

class TimerManagerClass : public Plugin {
public:
  Timer *registerTimer(Timer *t) {
    timers.add(t);
    t->Start();
    return t;
  };
  void loop() {
    for (int i=0; i < timers.size(); i++) {
      timers.get(i)->Update();
    }
  }

private:
  LinkedList<Timer*> timers = LinkedList<Timer*>();

};

#endif
