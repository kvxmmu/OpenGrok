//
// Created by nero on 12.06.2021.
//

#ifndef GROKPP_PLATFORM_H
#define GROKPP_PLATFORM_H

#if defined(_WIN32) || defined(_WIN64)
# define _PLATFORM_WINDOWS

#include <winsock2.h>
#endif

#ifdef _PLATFORM_WINDOWS
typedef SOCKET sock_t;
#else
typedef int sock_t;
#endif

#endif //GROKPP_PLATFORM_H
