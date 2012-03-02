/*
 *  Simple alpha PROXY Server
 *  ("Deliverer program for HTTP, global header
      frankly, It is HTTP relay with prefetch")
 *  
 *  ORIGINAL by k-chinen@is.aist-nara.ac.jp, 1994
 *  UPDATED & EXTENDED by mike@vorburger.ch, Nov 1998
 *
 *  $Id$
 */

#ifndef WCOL_H

#include <sys/time.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#ifdef SYSV
#include <sys/select.h>
#endif
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
/*
#include <malloc.h>
*/

#include <errno.h>
#include <time.h>

#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/signal.h>

#ifdef SYSV
typedef struct fd_set   rec_fd_set;
#else
typedef fd_set          rec_fd_set;
#endif


#define WWWCOL_VER      "0.0300"
#define WWWCOL_ID      	"0.03a0"
#define AGENT_INFO      "alpha Proxy, based on wcol (WWW Collector)"

#define HTTPPORT        	(8100)

#define DEFAULT_POOLDIR 	"/tmp/www-pool"
#define DEFAULT_LOGFILE 	"/tmp/wcol.log"
#define DEFAULT_CONVFILE 	"./wcol-conv.cfg"
#define DEFAULT_ACCEPTFILE 	"./wcol-accept.cfg"

#define DEFAULT_LOCKTIME 	(60*10)       		/* 10 min. */
#define DEFAULT_KEEPTIME 	(60*60*24*7)       	/*  1 week */


#define PADDING_NAME    "_noname_.html"

#define INFO_EXT    	",info"
#define LOCK_EXT    	",lock"
#define HEAD_EXT    	",head"
#define HEAD_TMP_EXT	",head-tmp"
#define BODY_TMP_EXT	",body-tmp"
#define OLD_EXT    		",old"

#define INFO_FILE       (0x01)
#define LOCK_FILE       (0x02)
#define BODY_FILE       (0x04)
#define HEAD_FILE       (0x08)
#define BODY_TMP_FILE   (0x10)
#define HEAD_TMP_FILE   (0x20)

#define STRING_SIZE     (256)
#define BUF_SIZE        (102400)


/*
 * Retry LOCKRETRY_INTERVAL seconds , LOCKRETRY_TIME times.
 */
#define LOCKRETRY_TIME      (5)
#define LOCKRETRY_INTERVAL  (5)


#define TH_COUNT        (0)
#define TH_TIME         (10000)



#define INFOSIZE        (512)

#define INFOVERSIZE     (16)
#define INFOTYPESIZE    (64)
#define INFONAMESIZE    (256)

typedef union {
    char dummy[INFOSIZE];           /* Alloc Info Size */
    struct {
        char id[INFOVERSIZE];       /* Identifyer */
        char ver[INFOVERSIZE];      /* Version */
        char name[INFONAMESIZE];    /* Name (e.g., URN or URL) */
        char type[INFOTYPESIZE];    /* Type */

	int  init_statu;	    /* Initial Status */
        long count;                 /* Access Count */
	time_t first;		    /* Create Date */
        time_t last;                /* Last Access Date */

	long take_time;		    /* Take time to get [ms]*/
	long conn_time;		    /* Connection Time [ms] */
	long trans_time;	    /* Translation Time [ms] */
    } attr;
} Info ;



#ifdef __STDC__
int Trace(const char *fmt, ...);
int Log(const char *fmt, ...);
int Error(const char *fmt, ...);
int Fatal(const char *fmt, ...);
#else
int Trace();
int Log();
int Error();
int Fatal();
#endif



/*
 * base.c
 */
int  ParseURL(char *url, char *protocol, char *host, char *port, char *path);
char *NormalizeURL(char *target);
char *AllocName(char *path,int cmd);
char *JoinURL(char *base, char *name);
Info *Have(char *path);
int What(Info *info);

int Lock(char *name);
int UnLock(char *name);

int UpdateInfo(char *name);
int DiscardInfo(char *name);


/*
 * http.c
*/
int Generate_Info(Info *info, char *name,       
                  int statu, long conn, long trans, long takes, 
                  int http1_or_not, char *head);


/*
 * tcp.c
 */
int bind_port(int portno);
int connect_server(char *hostname, int portno);


/*
 * accept.c
 */
int Acceptable(unsigned char *address);

/*
 * conv.c
 */
int Read_Convtable(char *name);


/* 
 * get.c / send.c
 */
Info *Get(char *path, char *opt);
int Send(int ofd, char *path, Info *info, char *opt, int discard);

/*
 * prefetch.c
 */
extern char log_file[];
extern int trace_flag;
extern char pool_dir[];
extern time_t keeptime;
extern time_t locktime;
extern int verbose;

#ifdef PREFETCH
int Prefetch(char *path);
extern int prefetch_flag;
#endif /* PREFETCH */


#define WCOL_H
#endif /* WCOL_H */
