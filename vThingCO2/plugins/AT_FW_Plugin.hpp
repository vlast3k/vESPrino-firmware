#ifndef AT_FW_Plugin_h
#define AT_FW_Plugin_h

#include "common.hpp"

class AT_FW_Plugin {
public:
    static void registerCommands(MenuHandler *handler);
    static void getTS(const char* line);
    static void atCIPSTART(const char *p);
    static void mockATCommand(const char *line);
    static int processResponseCodeATFW(HTTPClient *http, int rc);
  
private:



};


#endif
