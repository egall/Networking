

#include "stdafx.h"
#include "http.h"
#include "error.h"
#include "console.h"
#include <ws2tcpip.h>

/*
 * Resolve host name, and connect to the remote web server,
 * return the result of this connect operation.
 */
SOCKET HttpConnect(char* host, int port)
{
	SOCKET svrSock;
	//struct addrinfo hints,*res;
	//int error;	
	struct hostent* phent;
	struct sockaddr_in addr[3];
	int cnt,error;
	char msg[512];

	/* why getaddrinfo does not work on my computer?
	 * that is very strange, I tried sevrial times...
	 */
	//res = NULL;
	//memset(&hints,0,sizeof(hints));
	//hints.ai_flags = AI_CANONNAME;
	//hints.ai_family = AF_INET;
	//hints.ai_socktype = SOCK_STREAM;
	//hints.ai_protocol = IPPROTO_TCP;
	//hints.ai_addrlen = sizeof(hints);
	//error = getaddrinfo("192.168.72.28","http",&hints,&res);
	//if(error != 0)
	//{
	//	handle_error("Address resolve failed.");
	//	return 0;
	//}

	//freeaddrinfo(res);

	phent = gethostbyname(host);
	if(!phent)
	{
		handle_error("Address resove failed.Error: ");
		return INVALID_SOCKET;
	}

	cnt = 0;
#ifdef _DEBUG
	PsPrintf("Official name:%s,\n",phent->h_name);
	while(phent->h_aliases[cnt])
	{
		PsPrintf("Host alias %d:%s,\n",cnt+1,phent->h_aliases[cnt]);
		++ cnt;
	}
#endif

	cnt = 0;
	while(phent->h_addr_list[cnt] && cnt < 3)
	{
		memcpy(&addr[cnt].sin_addr,phent->h_addr_list[cnt],phent->h_length);
		addr[cnt].sin_family = AF_INET;
		addr[cnt].sin_port = htons(port);
#ifdef _DEBUG
		PsPrintf("Address %d:%d.%d.%d.%d,\n",cnt+1,addr[cnt].sin_addr.S_un.S_un_b.s_b1,
			addr[cnt].sin_addr.S_un.S_un_b.s_b2,addr[cnt].sin_addr.S_un.S_un_b.s_b3,
			addr[cnt].sin_addr.S_un.S_un_b.s_b4);
#endif
		
		++ cnt;
	}

	svrSock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
	if(svrSock == INVALID_SOCKET)
	{
		handle_error("socket error.");
		return INVALID_SOCKET;
	}

	error = connect(svrSock,(struct sockaddr*)&addr[0],sizeof(struct sockaddr_in));
	if(error == SOCKET_ERROR)
	{
		_snprintf(msg,sizeof(msg),"connect web host %s on port %d failed.Error:",host,port);
		handle_error(msg);
		return INVALID_SOCKET;
	}

	return svrSock;
}

int HttpGet(char* url,char* content)
{
	return 1;
}


