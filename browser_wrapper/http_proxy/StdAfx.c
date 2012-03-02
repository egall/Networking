/*
 * Precompiled header used by cl compiler.
 */
#include "StdAfx.h"

#ifndef _WIN32
#pragma comment(lib,"inet.lib")
#else
#pragma comment(lib,"ws2_32.lib")
#endif
