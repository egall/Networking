/*
 * Error handling function.
 * For each error, output the error messages.
 */

#include "stdafx.h"
#include "Error.h"
#include <lmerr.h>

BOOL ShowErrorMessage(DWORD dwError)
{
	HLOCAL hLocal = NULL;
	HMODULE hModule = NULL;
	DWORD dwBufferLength;
	DWORD dwBytesWritten;

    DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_IGNORE_INSERTS |
        FORMAT_MESSAGE_FROM_SYSTEM ;

    if(dwError >= NERR_BASE && dwError <= MAX_NERR)
	{
        hModule = LoadLibraryEx(TEXT("netmsg.dll"),NULL,LOAD_LIBRARY_AS_DATAFILE);
        if(hModule != NULL)
            dwFormatFlags |= FORMAT_MESSAGE_FROM_HMODULE;
    }
	//MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
	dwBufferLength = FormatMessage(
		dwFormatFlags, 
		NULL, dwError, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), 
		(PTSTR) &hLocal, 0, NULL);

	if (hLocal != NULL)
	{
		WriteFile(GetStdHandle(STD_ERROR_HANDLE),LocalLock(hLocal),dwBufferLength,&dwBytesWritten,NULL);
		LocalFree(hLocal);
	}
	else
	{
		WriteFile(GetStdHandle(STD_ERROR_HANDLE),_T("Error number not found."),lstrlen(_T("Error number not found.")),&dwBytesWritten,NULL);
	}

	if(hModule != NULL)
	{
        FreeLibrary(hModule);
	}

	return ( dwBufferLength != 0 );
}
