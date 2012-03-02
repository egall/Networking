/*
 *  Simple alpha PROXY Server
 *  ("Deliverer Program, TCP access procedures")
 *  
 *  ORIGINAL by k-chinen@is.aist-nara.ac.jp, 1994
 *  UPDATED & EXTENDED by mike@vorburger.ch, Nov 1998
 *
 *  $Id$
 */

#include "wcol.h"


int bind_port(int portno)
{
    int netd;
    struct sockaddr_in addr;

    if((netd=socket(AF_INET, SOCK_STREAM, 0))<0) {
        Error("bind_port: Cannot crate socket\n");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(portno);
    addr.sin_addr.s_addr = INADDR_ANY;
    if( bind(netd, (struct sockaddr*)&addr, sizeof(addr)) < 0 ) {
        Error("bind_port: bind() fail.\n");
        return -1;
    }
    if(listen(netd, 1)<0 ) {
        Error("bind_port: listen() fail.\n");
        return -1;
    }

    return netd;
}




int connect_server(char *hostname, int portno)
{
    int netd;
    struct hostent  *hp;
    struct sockaddr_in addr;
    char msg[BUF_SIZE];

    if((hp=gethostbyname(hostname)) == NULL ) {
        Error("connect_server: Cannot found host %s\n",hostname);

        return -1;
    }

    if((netd=socket(AF_INET, SOCK_STREAM, 0))<0) {
        Error("connect_server: Cannot crate socket\n");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = hp->h_addrtype;
    addr.sin_port = htons(portno);
    memcpy(&addr.sin_addr.s_addr, hp->h_addr, hp->h_length);

    if(connect(netd, (struct sockaddr*) &addr, sizeof(addr)) < 0) {
        Error("connect_server: Cannot connect %s\n",hostname);
        return -1;
    }

    return netd;
}

