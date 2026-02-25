#pragma once

#include <cassert>
#include <cerrno>
#include <cstdio>

#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#include <Windows.h>
#if _WIN32_WINNT <= 0x0600 // Windows XP or earlier, no GetProcessIdOfThread()
#include <winternl.h>
#endif
#ifndef __out_xcount
#define __out_xcount(x) // Workaround for the specstrings.h bug in the Platform SDK.
#endif
#define DBGHELP_TRANSLATE_TCHAR
#include "dbghelp.h"    // Provides portable executable (PE) image access functions.
