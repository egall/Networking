/*
 *  Simple alpha PROXY Server ("Deliverer Program, get procedures")
 * 
 *  ORIGINAL by k-chinen@is.aist-nara.ac.jp, 1994
 *  UPDATED & EXTENDED by mike@vorburger.ch, Nov 1998
 *
 *  $Id$
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>

#include "wcol.h"


extern Info *HTTP_Get(char*, char*, char*, int, char*);

/*
 * Get
 */

Info *Get(char *name, char *opt)
{
    char servername[STRING_SIZE], path[STRING_SIZE];
    char portname[STRING_SIZE], protocol[STRING_SIZE];
    int  port;
    Info *chk;
    char *p,*q;


    ParseURL(name, protocol, servername, portname, path);
    port = atoi(portname);

    if ( protocol[0] == '\0'  &&  servername[0] == '\0'  &&  port == 0 ) {
	/* This is a GET not to the PROXY server, but "direct" to our port.
	   Somebody wants some information, so we behave as a normal webserver. */

	Trace("Get: Stats request! Set count = -7 as flag for Send.\n");
	
    	chk = (Info*)malloc(sizeof(Info));
    	Generate_Info( chk, "", -1, 0, 0, 0, 1, "");
    	chk->attr.count = -7;  /* That's just a flag for us. See Send() */
    	return chk;
    };


    Trace("Get: Protocol=%s, Server=%s, Port=%d, Path=%s (%d).\n",
        protocol, servername, port, path, strncmp("http",protocol,4));

    if(strncmp("http",protocol,4)!=0) {
        Trace("Get: Sorry, unsupport protocol[%s], What is it ? %s\n",
            protocol, name);
        return (Info*)NULL;
    }


    if(Lock(name)) {
        Error("Get: Sorry, cannot lock %s, giveup.\n", name);
        UnLock(name);
        return (Info*)NULL;
    }

    chk = HTTP_Get(name, opt, servername, port, path);
    UnLock(name);

    return chk;
}
