
/*
 * Init console of this program, and defined the output funcation.
 */

#include "stdafx.h"
#include "Main.h"
#include "console.h"
#include "server.h"
#include <stdarg.h>

HANDLE hStdIn;
HANDLE hStdOut;
HANDLE hStdErr;

/*
 * Obtain the handle associated with the program.
 */
int ConsoleInit()
{
	hStdIn = GetStdHandle(STD_INPUT_HANDLE);
	HandleConsoleError(hStdIn);
	hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	HandleConsoleError(hStdOut);
	hStdErr = GetStdHandle(STD_ERROR_HANDLE);
	HandleConsoleError(hStdErr);

	return TRUE;
}

/*
 * Use this function instead of printf, which has potential buffer overrun problem.
 */
int PsPrintf(char* fmt,...)
{
	char buf[MAX_BUF_LEN + 1];
	int ret;
	DWORD dwWritten;
	va_list list;

	va_start(list,fmt);
	ret = _vsnprintf(buf,sizeof(buf),fmt,list);

	if(!WriteFile(hStdOut,buf,ret,&dwWritten,NULL))
	{
		handle_error("Write to stdout failed.");
	}

	va_end(list);
	return ret;
}


