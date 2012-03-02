/*
 * Entry and main function of this program.
 * Make some initialization.
 */

#include "StdAfx.h"
#include "Main.h"
#include "console.h"
#include "proxy.h"
#include "server.h"
#include "http.h"
#include "filter.h" 
#include <signal.h>

int ps_port;
char filters[10][50];

int main(int argc,char* argv[])
{
	int i;
	DWORD dwThreadID;
	char (*p) [50];

	if(!ConsoleInit())
	{
		exit(0);
	}

	if(argc<2)
	{
		usage(argv[0]);
	}

    ps_port = atoi(argv[1]);
	
	p = filters;
	for(i=2;i<argc;i++)
	{
		strncpy(p[i-2],argv[i],strlen(argv[i]));
#if defined _DEBUG
		prints(filters[i-2]);
#endif
	}

	//signal(SIGINT,IntHandler);

	if(!NetStartup())
	{
		prints("Net startup failed.\n");
		exit(0);
	}

	if(NULL == CreateThread(NULL,0,ProxyServerControlThread,(void*)0,0,&dwThreadID))
	{
		PsPrintf("Exit thread create error.\n");
		ShowErrorMessage(GetLastError());
		exit(0);
	}

	PsPrintf("Press q to exit.\n");

	if(!StartProxyServer())
	{
		StopProxyServer();
		prints("Proxy server start failed.\n");
		exit(0);
	}

	//if(!StopProxyServer())
	//{
	//	prints("Proxy server start failed.\n");
	//	exit(0);
	//}

	//system("pause");
	return 0;
}

void IntHandler(int sig)
{
	prints("Ctrl C raised.\n");
	if(!StopProxyServer())
	{
		prints("Proxy server start failed.\n");
		exit(1);
	}
	exit(0);
}

void usage(const char* program)
{
	PsPrintf("usage: %s [port] [filter list...]\nPress q to exit.",program);
	exit(0);
}
