/* main.h */

/* Copyright(c) Vector Informatik GmbH.All rights reserved.
   Licensed under the MIT license.See LICENSE file in the project root for details. */

#ifndef __MAIN_H_
#define __MAIN_H_

// Windows or Linux ?
#if defined(_WIN32) || defined(_WIN64)
  #define _WIN
  #if defined(_WIN32) && defined(_WIN64)
    #undef _WIN32
  #endif
#else
  #if defined (_ix64_) || defined (__x86_64__)
    #define _LINUX64
  #else
    #define _LINUX32
  #endif
  #define _LINUX
#endif

#ifdef _LINUX // Linux

  #define _LINUX
  
  //#define _POSIX_C_SOURCE 200809L
  //#define _GNU_SOURCE
  #include <link.h>
  #include <stdlib.h>
  #include <stdio.h>
  #include <string.h>
  #include <stdint.h>
  #include <stdarg.h>
  #include <math.h>
  #include <assert.h>
  #include <errno.h>
  #include <sys/time.h>
  #include <time.h>
  #include <sys/stat.h>
  #include <pthread.h> 
  #include <sys/socket.h>
  #include <arpa/inet.h>

  #define MAX_PATH 256

#endif // Linux
#ifdef _WIN // Windows

  #define WIN32_LEAN_AND_MEAN
  #define _CRT_SECURE_NO_WARNINGS // to simplify Linux and Windows single source
  #include <windows.h>
  #include <stdio.h>
  #include <stdlib.h>
  #include <stdint.h>
  #include <assert.h>
  #include <time.h>
  #include <conio.h>
  #ifdef __cplusplus
  #include <thread>
  #endif


  // Need to link with Ws2_32.lib
  #pragma comment(lib, "ws2_32.lib")
  #include <winsock2.h>
  #include <ws2tcpip.h>

#endif // Windows


//-----------------------------------------------------------------------------------------------------
// Application configuration:
// XCP configuration is in xcp_cfg.h and xcptl_cfg.h

// XCP slave name
#define XCPSIM_SLAVE_ID_LEN       7    /* Slave device identification length */
#define XCPSIM_SLAVE_ID          "XCPlite"  /* Slave device identification */

#define XCPSIM_ENABLE_A2L_GEN // Enable A2L generation

//#define CLOCK_USE_APP_TIME_US // Use us timestamps relative to application start
#define CLOCK_USE_UTC_TIME_NS // Use ns timestamps relative to 1.1.1970

#define XCPSIM_DEFAULT_DEBUGLEVEL 1

#define XCPSIM_MULTI_THREAD_SLAVE // Receive and transmit in seperate threads

#ifdef _LINUX // Linux

  #define XCPSIM_SLAVE_PORT 5555 // Default UDP port
  #define XCPSIM_SLAVE_UUID {0xdc,0xa6,0x32,0xFF,0xFE,0x7e,0x66,0xdc} // Default slave clock UUID

  // Option defaults
  #define XCPSIM_DEFAULT_A2L 1
  #define XCPSIM_DEFAULT_A2L_PATH "./"
  #define XCPSIM_DEFAULT_JUMBO 1

  // A2L defaults
  #ifdef _LINUX64
    #define getA2lSlaveIP() ("172.31.31.84")  // A2L IP address
  #else
   #define getA2lSlaveIP() ("172.31.31.194")  // A2L IP address
  #endif
  #define getA2lSlavePort() XCPSIM_SLAVE_PORT // for A2L generation

#endif // Linux

#ifdef _WIN // Windows

  // Option defaults
  #define XCPSIM_DEFAULT_A2L 1
  #define XCPSIM_DEFAULT_A2L_PATH "./CANape"

  #define XCPSIM_SLAVE_PORT 5555 // Default UDP port
  #define XCPSIM_SLAVE_UUID {0xdc,0xa6,0x32,0xFF,0xFE,0x7e,0x66,0xdc} // Default slave clock UUID

  #define XCPSIM_ENABLE_XLAPI_V3
  #ifdef XCPSIM_ENABLE_XLAPI_V3

    #define XCPSIM_DEFAULT_JUMBO 0

    // XL-API UDP stack parameters
    #define XCPSIM_SLAVE_XL_NET "NET1" // Default V3 Network name
    #define XCPSIM_SLAVE_XL_SEG "SEG1" // Default V3 Segment name
    #define XCPSIM_SLAVE_XL_MAC {0xdc,0xa6,0x32,0x7e,0x66,0xdc} // Default V3 Ethernet Adapter MAC
    #define XCPSIM_SLAVE_XL_IP_S "172.31.31.194" // Default V3 Ethernet Adapter IP as string
    #define XCPSIM_SLAVE_XL_IP {172,31,31,194} // Default V3 Ethernet Adapter IP

    // A2L defaults
    #define getA2lSlavePort() XCPSIM_SLAVE_PORT // for A2L generation
    #define getA2lSlaveIP() ((gOptionUseXLAPI)?XCPSIM_SLAVE_XL_IP_S:"127.0.0.1") // A2L IP address

    // Need to link with vxlapi.lib
    #ifdef _WIN64
    #pragma comment(lib, "vxlapi64.lib")
    #endif
    #ifdef _WIN32
    #pragma comment(lib, "vxlapi.lib")
    #endif

  #else

    #define XCPSIM_DEFAULT_JUMBO 1

    // A2L defaults
    #define getA2lSlavePort() XCPSIM_SLAVE_PORT // for A2L generation
    #define getA2lSlaveIP() ("127.0.0.1") // A2L IP address

  #endif

#endif // Windows





//-----------------------------------------------------------------------------------------------------

#include "xcpLite.h" // XCP protocoll layer
#include "clock.h" // DAQ clock
#ifdef _WIN 
  #ifdef XCPSIM_ENABLE_XLAPI_V3
    #include "vxlapi.h" // Vector XL-API V3
  #endif
  #include "udp.h" // UDP stack for Vector XL-API V3 
#endif
#include "xcpTl.h" // XCP on UDP transport layer
#include "xcpSlave.h" // XCP slave
#ifdef XCPSIM_ENABLE_A2L_GEN 
#include "A2L.h" // A2L generator
#endif
#include "ecu.h" // Demo measurement task C
#include "ecupp.hpp" // Demo measurement task C++


//-----------------------------------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif

// Options
extern volatile unsigned int gDebugLevel;
extern int gOptionJumbo;
extern uint16_t gOptionSlavePort;
extern int gOptionA2L;
extern char gOptionA2L_Path[MAX_PATH];
#ifdef _WIN
#ifdef XCPSIM_ENABLE_XLAPI_V3
extern int gOptionUseXLAPI;
extern char gOptionXlSlaveNet[32];
extern char gOptionXlSlaveSeg[32];
#endif
#endif // _WIN


extern int createA2L(const char* path_name);

#ifdef __cplusplus
}
#endif

#endif
