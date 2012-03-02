
/*
 * Filter the http packet.
 */

#include "StdAfx.h"
#include "filter.h"
#include "console.h"

extern char filters[][50];

/*
 * Check if the requst host is in the filter list.
 */
int DoFilter(const char* hostname)
{
	int i;

	i = 0;
	while((char*)*filters[i] != NULL)
	{
		if(strstr(hostname,filters[i]))
		{
			return 1;
		}
		++i;
	}

	return 0;
}

/*
 * Modify each buffer, and add the Connection: close header line.
 */
int DoHeaderLine(char* buf, int len)
{
	char* proxypos;
	char* conpos;
	int cnt;

	conpos = strstr(buf,"Connection:");

	proxypos = strstr(buf,"Proxy-Connection");
	if(proxypos)
	{
		cnt = 0;
		conpos = proxypos;
		while(*conpos++ != '\r')
		{
			++cnt;
		}
		memset(proxypos,0,cnt);
		strcpy(proxypos,"Connection: close");
	}

	return 1;
}

/*
 * Check if the request's method is allowed by the proxy server.
 * The proxy server only allow three mothods:
 * GET,HEAD,POST.
 */
int DoMethodAllowed(const char* reqLine)
{
	if(strncmp(reqLine,"GET",3) == 0)
	{
		return 1;
	}

	if(strncmp(reqLine,"POST",4) == 0)
	{
		return 1;
	}

	if(strncmp(reqLine,"HEAD",4) == 0)
	{
		return 1;
	}

	return 0;
}
