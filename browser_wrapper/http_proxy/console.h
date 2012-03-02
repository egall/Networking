
#ifndef _CONSOLE_H
#define _CONSOLE_H


#define HandleConsoleError(hConsole) \
	if(hConsole == INVALID_HANDLE_VALUE)	\
	{	\
		handle_error("GetStdHandle failed.");	\
		return FALSE;	\
	}	\
	if(!hConsole)	\
	{	\
		PsPrintf("This application does not have associated standard handles.\n");	\
		return FALSE;	\
	}	\

int ConsoleInit();
int PsPrintf(char*,...);

#endif
