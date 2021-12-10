#include "configuration.h"


// Commandline Options amd Defaults
volatile unsigned int gDebugLevel = APP_DEFAULT_DEBUGLEVEL;
char gOptionA2L_Path[MAX_PATH] = APP_DEFAULT_A2L_PATH;
char a2l_filename[100] = "XCPlite";
int gOptionJumbo = APP_DEFAULT_JUMBO;
unsigned char gOptionSlaveAddr[4] = { 127,0,0,1 };
uint16_t gOptionSlavePort = APP_DEFAULT_SLAVE_PORT;
int gOptionUseXLAPI = FALSE;

#ifdef _WIN 
#ifdef APP_ENABLE_XLAPI_V3
char gOptionXlSlaveNet[32] = APP_DEFAULT_SLAVE_XL_NET;
char gOptionXlSlaveSeg[32] = APP_DEFAULT_SLAVE_XL_SEG;
#endif
#endif 

// Infos needed by A2lHeader() in A2L.c
char* getA2lSlaveIP() {
    static char tmp[32];
    inet_ntop(AF_INET, &gXcpTl.SlaveAddr.addr.sin_addr, tmp, sizeof(tmp));
    return tmp;
}

uint16_t getA2lSlavePort() {
    return htons(gXcpTl.SlaveAddr.addr.sin_port);
}
