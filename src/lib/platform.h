//
// Created by nero on 5/2/21.
//

#ifndef OPENGROK_PLATFORM_H
#define OPENGROK_PLATFORM_H

#include <winsock2.h>

#if defined(_WIN32) || defined(_WIN64)
#    define _PLATFORM_WINDOWS
#endif

#ifdef _PLATFORM_WINDOWS
typedef u_long address_t;
typedef int optlen_t;
#else
typedef in_addr_t address_t;
typedef size_t optlen_t;
#endif

#endif //OPENGROK_PLATFORM_H
