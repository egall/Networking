
#ifndef _MAIN_H
#define _MAIN_H

#if !defined(_WIN32)
#error ERROR: Only Win32 target supported!
#endif

#include "error.h"

#define prints(s) PsPrintf("%s\n",s);

void IntHandler(int);
void usage(const char*);

#endif
