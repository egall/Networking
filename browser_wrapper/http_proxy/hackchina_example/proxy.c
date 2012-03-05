/*
 * Implement the startup and cleanup of the windows sockets network envrionment.
 */

#include "StdAfx.h"
#include "proxy.h"
#include "main.h"
#include "console.h"
#include "error.h"
#include "server.h"

extern HANDLE hStdIn;

int NetStartup()
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int error;

	wVersionRequested = MAKEWORD(2,2);

	error = WSAStartup(wVersionRequested,&wsaData);

	if(error)
	{
		PsPrintf("Windows sockets 2.0 unavailable.\n");
		return 0;
	}

	if( LOBYTE( wsaData.wVersion ) != 2 || HIBYTE( wsaData.wVersion ) != 2 )
	{
		PsPrintf("Windows sockets 2.0 unavailable.\n");
		WSACleanup();
		return 0;
	}

    return 1;
}

int NetCleanup()
{
	int error;

	error = WSACleanup();

	if(error == SOCKET_ERROR)
	{
		PsPrintf("Cleanup unsuccessfully.\n");
		return 0;
	}

	return 1;
}

/*
 * Read from STDIN and if the user input 'q' or 'Q',
 * the program end the server and exit.
 */
int WINAPI ProxyServerControlThread(void* param)
{
	char buf[128];
	DWORD dwRead;
	int ret;

	while(1)
	{
		ret = ReadFile(hStdIn,buf,1,&dwRead,NULL);

		if(!ret)
		{
			handle_error("Read from stdin failed.");
		}

		if(buf[0] == 'q' || buf[0] == 'Q')
		{
			if(!StopProxyServer())
			{
				prints("Proxy server start failed.\n");
				return 1;
			}
		}
	}

	return 0;
}
