
#ifndef ERROR_H
#define ERROR_H

#define handle_error(s) PsPrintf("%s\n",s);	\
	ShowErrorMessage(GetLastError());

BOOL ShowErrorMessage(DWORD);

#endif
