
#include <stdio.h>
#include <stdlib.h>


#ifdef _WIN32
//#define WIN32_LEAN_AND_MEAN
#include <tchar.h>
#include <winsock2.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#endif
