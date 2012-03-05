

#ifndef _PROXY_H
#define _PROXY_H


int NetStartup();
int NetCleanup();
int WINAPI ProxyServerControlThread(void* param);

#endif 
