/*
 *  Simple alpha PROXY Server
 *  ("Deliverer program, message and logging procedures")
 *  
 *  ORIGINAL by k-chinen@is.aist-nara.ac.jp, 1994
 *  UPDATED & EXTENDED by mike@vorburger.ch, Nov 1998
 *
 *  $Id: msg.c,v 1.2 1994/08/02 17:34:33 k-chinen Exp k-chinen $
 */

/*
 * Simple:
 *		Trace:	show message.
 *		Log:	write message to log-file.
 *
 * Complex:
 *		Error:	message.
 *		Fatal:	message and log.
 *	
 */

#include "wcol.h"

#include <string.h>
#include <syslog.h>
#include <unistd.h>


#ifdef __STDC__
int Trace(const char *fmt, ...)
#else
int Trace(va_alist)
va_dcl
#endif
{
#ifndef __STDC__
    char *fmt;
#endif
    va_list args;

#ifdef __STDC__
    va_start(args,fmt);
#else
    va_start(args);
    fmt = va_arg(args, char *);
#endif

    if(trace_flag) {
		fprintf(stderr, "wcol(%i) : ", getpid() );
        vfprintf(stderr,fmt,args);
        fflush(stderr);
    }

    va_end(args);
}



#ifdef __STDC__
int Log(const char *fmt, ...)
#else
int Log(va_alist)
va_dcl
#endif
{
    va_list args;
#ifndef __STDC__
    char *fmt;
#endif
    FILE *log;
    time_t now;
    char report[STRING_SIZE],status[STRING_SIZE];
    char *p;

#ifdef __STDC__
    va_start(args,fmt);
#else
    va_start(args);
    fmt = va_arg(args, char *);
#endif

    time(&now);
    sprintf(status,"%d %s",getpid(),ctime(&now));
    /* remove '\n' because ctime() append it. */
    p = status;
    while(*p&&*p!='\n')
        p++;
    *p = '\0';

    vsprintf(report, fmt, args);

#ifdef USE_SYSLOG
    syslog(LOG_NOTICE,"wcol %s",report);
#else
    if((log=fopen(log_file,"a"))==NULL) {
        if((log=fopen("/dev/console","w"))==NULL) {
            fprintf(stderr,"Sorry, cannot open logfile and console\n");
            return;
        }
        else {
            fprintf(log,"Cannot open logfile %s.\n",log_file);
        }
    }
    fprintf(log,"%s %s",status,report);
    fflush(log);
    fclose(log);
#endif
#ifdef DEBUG
    fprintf(stderr,"*** LOG %s",report);
    fflush(stderr);
#endif
    va_end(args);
}



/*
 * Error --- show error messages with 'Error' like as Trace.
 */

#ifdef __STDC__
int Error(const char *fmt, ...)
#else
int Error(va_alist)
va_dcl
#endif
{
#ifndef __STDC__
    char *fmt;
#endif
    va_list args;
    char report[STRING_SIZE];

#ifdef __STDC__
    va_start(args,fmt);
#else
    va_start(args);
    fmt = va_arg(args, char *);
#endif

    vsprintf(report, fmt, args);
    fprintf(stderr, "WCOL === ERROR %s", report);
    fflush(stderr);

    va_end(args);
}


#ifdef __STDC__
int Fatal(const char *fmt, ...)
#else
int Fatal(va_alist)
va_dcl
#endif
{
    va_list args;
#ifndef __STDC__
    char *fmt;
#endif
    FILE *log;
    time_t now;
    char report[STRING_SIZE],status[STRING_SIZE];
    char *p;

#ifdef __STDC__
    va_start(args,fmt);
#else
    va_start(args);
    fmt = va_arg(args, char *);
#endif

    time(&now);
    sprintf(status,"%d %s",getpid(),ctime(&now));
    /* remove '\n' because ctime() append it. */
    p = status;
    while(*p&&*p!='\n')
        p++;
    *p = '\0';

    vsprintf(report, fmt, args);

#ifdef USE_SYSLOG
    syslog(LOG_NOTICE,"wcol %s",report);
#else
    if((log=fopen(log_file,"a"))==NULL) {
        if((log=fopen("/dev/console","w"))==NULL) {
            fprintf(stderr,"Sorry, cannot open logfile and console\n");
            return;
        }
        else {
            fprintf(log,"Cannot open logfile %s.\n",log_file);
        }
    }
    fprintf(log,"%s %s",status,report);
    fflush(log);
    fclose(log);
#endif

    fprintf(stderr,"*** FATAL %s",report);
    fflush(stderr);

    va_end(args);
}
