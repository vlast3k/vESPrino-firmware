
#ifndef Button_h
#define Button_h


#include "MenuHandler.hpp"
#include "interfaces/Plugin.hpp"
#define BTTN_FUNC_INTERVAL 2000
class ButtonClass : public Plugin {
public:
  ButtonClass();
  //bool setup(MenuHandler *handler);
  const char* getName() {
    return "PowerManager";
  }
  void loop();
private:
  int ledDark = 100, ledBright = 70;


};



#endif
