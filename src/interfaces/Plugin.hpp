#ifndef Plugin_h
#define Plugin_h

#include "MenuHandler.hpp"

class Plugin {
public:
  virtual bool setup(MenuHandler *handler) {
    return false;
  };
  virtual void loop(){};
  virtual const char* getName() {
    return "No Name";
  }
};

#endif
