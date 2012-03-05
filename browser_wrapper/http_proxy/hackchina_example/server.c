/*server.c  -- This is the proxy server.
 *
 */

#include "StdAfx.h"
#include "server.h"
#include "error.h"
#include "proxy.h"
#include "console.h"
#include "http.h"
#include "filter.h"

extern int ps_port;

SOCKET svrSock;
BOOL svrRunning = TRUE;

/*
 * Start the proxy server, and listen on the port, to client's connection.
 */
int StartProxyServer()
{
	struct sockaddr_in saddr;
	int error;

	svrSock = socket(AF_INET,SOCK_STREAM,IPPROTO_IP);
	if(svrSock == INVALID_SOCKET)
	{
		PsPrintf("socket error.\n");
		ShowErrorMessage(WSAGetLastError());
		return 0;
	}

	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(ps_port);
	saddr.sin_addr.S_un.S_addr = htonl(ADDR_ANY);

	error = bind(svrSock,(struct sockaddr*)&saddr,sizeof(saddr));
	if(error == SOCKET_ERROR)
	{
		PsPrintf("bind error.\n");
		ShowErrorMessage(WSAGetLastError());
		//closesocket(svrSock);
		//NetCleanup();
		return 0;
	}

	error = listen(svrSock,SOMAXCONN);
	if(error == SOCKET_ERROR)
	{
		PsPrintf("listen error.\n");
		ShowErrorMessage(WSAGetLastError());
		//closesocket(svrSock);
		//NetCleanup();
		return 0;
	}

	while(svrRunning)
	{
		SOCKET reqSock;
		struct sockaddr_in reqaddr;
		int addrlen;
		HANDLE hThread;
		DWORD dwThreadID;

		memset(&reqaddr,0,sizeof(reqaddr));
		addrlen = sizeof(reqaddr);
		
		reqSock = accept(svrSock,(struct sockaddr*)&reqaddr,&addrlen);

		if(!svrRunning)
		{
			break;
		}

		if(reqSock == INVALID_SOCKET)
		{
			PsPrintf("accept error.\n");
			ShowErrorMessage(WSAGetLastError());
			continue;
		}

#ifdef _DEBUG
		PsPrintf("A connection request form client %d.%d.%d.%d:%d.\n",reqaddr.sin_addr.S_un.S_un_b.s_b1,
			reqaddr.sin_addr.S_un.S_un_b.s_b2,reqaddr.sin_addr.S_un.S_un_b.s_b3,
			reqaddr.sin_addr.S_un.S_un_b.s_b4,reqaddr.sin_port);
#endif

		
		hThread = CreateThread(NULL,0,ProxyServerReqThread,(void*)reqSock,0,&dwThreadID);
		if(hThread == NULL)
		{
			PsPrintf("Thread create error.\n");
			ShowErrorMessage(GetLastError());
		}
	}


	return 1;
}

/*
 * Stop the proxy server and clean the network envrionment.
 */
int StopProxyServer()
{
	int error;

	svrRunning = FALSE;
	Sleep(2000);

	error = closesocket(svrSock);
	if(error == SOCKET_ERROR)
	{
		PsPrintf("closesocket error.\n");
		ShowErrorMessage(WSAGetLastError());
		return 0;
	}

	return NetCleanup();
}

/*
 * Server function, proceed each connection of client browser.
 * Each request from browser are check as follows:
 * if it should be filtered, then if its method is allowed, 
 * then modify the Connection header line, and Proxy-Connection header line,
 * then send the data to web server, and make a new thread to transfer data 
 * between the web server and the browser.
 */
int WINAPI ProxyServerReqThread(void* param)
{
	SOCKET sock = (SOCKET)param;
	SOCKET s;
	sockpair spair;
	char buf[MAX_BUF_LEN+1];
	char hostname[MAX_HOSTNAME_LEN+1];
	char reqLine[MAX_BUF_LEN];
	char hostport[8];
	int nbytes;
	struct sockaddr_in addr;
	int addrlen = sizeof(addr);
	char* hostpos;
	char* portpos;
	int first;
	int i;
	int port;
	int error;
	int filtered;
	int connected;
	HANDLE hThread;
	DWORD dwThreadID;

	first = 0;
	connected = 0;
	port = 80;
	while(1)
	{
		/* when the socket is closed by client,
		 * the recv always return the last data received, infinitely,
		 * how to deal with this problem?
		 */
		/*
		The following code dose not work,
		if(SOCKET_ERROR == getsockname(sock,&addr,&addrlen))
		{
			if(WSAGetLastError() == WSAENOTSOCK)
			{
				break;
			}
		}
		*/

		/*This does not work, too.*/
		//if(SOCKET_ERROR == getpeername(sock,&addr,&addrlen))
		//{
		//	if(WSAGetLastError() == WSAENOTSOCK)
		//	{
		//		break;
		//	}
		//}

		filtered = 0;
		nbytes = recv(sock,buf,512,0);

		/*
		 *connection closed by client host.
		 *simply return, and thread terminated.
		 */
		if(nbytes <= 0)
		{
			if(SOCKET_ERROR == getpeername(sock,(struct sockaddr*)&addr,&addrlen))
			{
				if(WSAGetLastError() == WSAENOTSOCK)
				{
					break;
				}
			}
#ifdef _DEBUG
			PsPrintf("Connection closed by client %d.%d.%d.%d:%d.\n",addr.sin_addr.S_un.S_un_b.s_b1,
				addr.sin_addr.S_un.S_un_b.s_b2,addr.sin_addr.S_un.S_un_b.s_b3,
				addr.sin_addr.S_un.S_un_b.s_b4,addr.sin_port);
#endif
			break;
		}

		++first;

		buf[nbytes] = '\0';

		//format: Host : www.tom.com:8000
		hostpos = strstr(buf,HOST);
		i = 0;
		if(hostpos)
		{
			first = 1;
			hostpos += 6;
			while(*hostpos != '\r')
			{
				hostname[i++] = *hostpos;
				++hostpos;
			}
			hostname[i] = '\0';
#ifdef _DEBUG
			PsPrintf("Host: [%s]\n",hostname);
#endif

			portpos = strchr(hostname,':');
			if(portpos)
			{
				strcpy(hostport,portpos+1);
				*portpos = '\0';
				port = atoi(hostport);
#ifdef _DEBUG
				PsPrintf("Host Port: [%s]\n",hostport);
#endif
			}
		}

		if(first == 1)
		{
			i = 0;
			hostpos = buf;
			while(*hostpos != '\r')
			{
				reqLine[i++] = *hostpos;
				++hostpos;
			}
			reqLine[i] = '\0';

			/*
			 * The request is filtered, and should be refused, send HTTP 403 error to client browser.
			 */
			if(DoFilter(hostname))
			{
				filtered = 1;
				error = send(sock,HTTP_403_STRING,(int)strlen(HTTP_403_STRING),0);
				if(error == SOCKET_ERROR)
				{
					handle_error("send 403 to client failed. Error: ");
					break;
				}
				PsPrintf("%s:\tFILTERED %s\n",hostname,reqLine);
				break;
			}

			/*
			 * The request's method is not allowed, send HTTP 405 to client browser.
			 */
			if(!DoMethodAllowed(reqLine))
			{
				error = send(sock,HTTP_405_STRING,(int)strlen(HTTP_405_STRING),0);
				if(error == SOCKET_ERROR)
				{
					handle_error("send 405 to client failed. Error: ");
					break;
				}
				PsPrintf("%s:\tMETHOD NOT ALLOWED %s\n",hostname,reqLine);
				break;
			}

			DoHeaderLine(buf,nbytes);

			/*
			 * Valid request output.
			 */
			PsPrintf("%s:\t%s\n",hostname,reqLine);
			s = HttpConnect(hostname,port);
			if(s == INVALID_SOCKET)
			{
				PsPrintf("Connect to web server failed.\n");
				break;
			}
			connected = 1;
			spair.sockl = sock;
			spair.sockr = s;
			hThread = CreateThread(NULL,0,ProxyServerRepThread,&spair,0,&dwThreadID);
			if(!hThread)
			{
				handle_error("Server response thread creation failed.Error:");
				break;
			}
		}

#ifdef _DEBUG
		PsPrintf("%s\n",buf);
#endif

		error = send(s,buf,nbytes,0);
		if(error == SOCKET_ERROR)
		{
			handle_error("send to web host failed. Error: ");
			break;
		}
	}

	if(connected && SOCKET_ERROR == closesocket(s))
	{
		handle_error("closesocket s failed in ProxyServerReqThread routine.Error:");
	}

	if(SOCKET_ERROR == closesocket(sock))
	{
		handle_error("closesocket sock failed in ProxyServerReqThread routine.Error:");
	}

	return 0;
}

/*
 * Thread function, repeatly read from web server and send the data to client browser,
 * until connection closed or error occured.
 * sl  <------  sr
 */
int WINAPI ProxyServerRepThread(void* param)
{
	sockpair spair = *(sockpair*)param;
	SOCKET sl = spair.sockl;
	SOCKET sr = spair.sockr;
	char buf[MAX_BUF_LEN+1];
	int nbytes;

	while(1)
	{
		nbytes = recv(sr,buf,MAX_BUF_LEN,0);
		if(nbytes <= 0)
		{
			break;
		}
		nbytes = send(sl,buf,nbytes,0);
		if(nbytes <= 0)
		{
			break;
		}
	}

	return 0;
}
