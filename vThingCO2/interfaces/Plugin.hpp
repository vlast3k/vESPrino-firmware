#ifndef Plugin_h
#define Plugin_h

#include "MenuHandler.hpp"

class Plugin {
public:
  virtual void setup(MenuHandler *handler){};
  virtual void loop(){};
};

#endif
