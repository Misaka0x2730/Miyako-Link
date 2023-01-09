#ifndef _SOCKET_H
#define _SOCKET_H

#include "Ethernet/socket.h"

extern void System_Delay(uint32_t ms);

#define SYSTEM_DELAY(ms)    do { System_Delay(ms); } while(0)

#endif //_SOCKET_H
