
#ifndef _HTTP_H
#define _HTTP_H

#define HOST "Host: "

typedef struct
{
	SOCKET sockl;
	SOCKET sockr;
}sockpair,*psockpair;

SOCKET HttpConnect(char*, int);
//int HttpGet(SOCKET,char* content);
//int HttpResponse

#endif ...
