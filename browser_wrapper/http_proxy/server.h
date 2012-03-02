

#ifndef _SERVER_H
#define _SERVER_H

#define MAX_BUF_LEN 512
#define MAX_HOSTNAME_LEN 128
#define HTTP_403_STRING "HTTP/1.1 403 Forbidden \r\n"	\
	"\r\n"	\
	"\r\n"	\
    "<html><head><title>HTTP 403</title></head>"	\
	"<body><h1>HTTP Error 403 Forbidden</h1><p>The request was refused by the server.</body></html>"

#define HTTP_405_STRING "HTTP/1.1 405 Method Not Allowed \r\n"	\
	"Allow: GET,HEAD,POST \r\n"	\
	"\r\n"	\
	"<html><head><title>HTTP 405</title></head>"	\
	"<body><h1>HTTP Error 405 Method Not Allowed</h1>"	\
	"<p>A request was made with a method that is not supported for the requested URL.</body></html>"



int StartProxyServer();
int StopProxyServer();
int WINAPI ProxyServerReqThread(void* param);
int WINAPI ProxyServerRepThread(void* param);

#endif
