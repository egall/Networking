/*
 *  Simple alpha PROXY Server
 *  ("Deliverer Program, send (HTTP) procedures")
 * 
 *  ORIGINAL by k-chinen@is.aist-nara.ac.jp, 1994
 *  UPDATED & EXTENDED by mike@vorburger.ch, Nov 1998 
 *
 *  $Id: conv.c,v 1.1 1994/12/01 07:25:36 k-chinen Exp k-chinen $
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>

#include "wcol.h"

#include "conv.h"


conv_elem conv_tbl[CONV_TBL_NUM+1];


int Read_Convtable(char *name)
{
#define GET_TMP \
    q = tmp;\
    while(*p && (*p=='\t'||*p==' ')) p++; if(*p!='"') continue; p++;\
    while(*p && *p!='"') { if(*p=='\\') p++; *q++ = *p++;} p++; *q='\0'


    FILE *fp;
    char buf[BUF_SIZE];
    char tmp[STRING_SIZE];
    char *p,*q;
    int n;

    if((fp=fopen(name,"r"))==NULL) {
        Error("Read_Convtable: ERROR: Cannot open %s\n",name);
        return 1;
    }

    n = 0;
    while(n<CONV_TBL_NUM && fgets(buf,BUF_SIZE,fp)!=NULL) {
        if(buf[0]=='#')
            continue;

        p = buf;

        GET_TMP; strcpy(conv_tbl[n].target, tmp);
        GET_TMP; strcpy(conv_tbl[n].target_ext, tmp);
        GET_TMP; strcpy(conv_tbl[n].output, tmp);
        GET_TMP; strcpy(conv_tbl[n].program, tmp);

        Trace("conv_tbl[%d] \"%s\" or \"%s\" --[%s]-> \"%s\"\n",
            n, conv_tbl[n].target, conv_tbl[n].target_ext,
            conv_tbl[n].program, conv_tbl[n].output);

        n++;
    }
    conv_tbl[n].target[0] = '.';

    return 0;

#undef GET_TMP
}


int conv_program_name(char *dest,char *mask, char *in, char *out)
{
    char *p;
    char *q;
    char *r;
    int ret=0x0;

    Trace("conv_program_name: [%s]\n\tin [%s], out [%s]\n",mask,in,out);

    p = mask;
    q = dest;
    while(*p) {
        if(*p=='\\') {
            p++;
            *q++ = *p++;
        }
        else
        if(*p=='@') {
            if(*(p+1)=='i') {
                r = in;
                while(*r)
                    *q++ = *r++;
                ret = ret | CONV_HAS_INPUT;
            }
            if(*(p+1)=='o') {
                r = out;
                while(*r)
                    *q++ = *r++;
                ret = ret | CONV_HAS_OUTPUT;
            }
            p+=2;
        }
        else {
            *q++ = *p++;
        }
    }
    *q = '\0';

    Trace("conv_program_name: after [%s]\n",dest);

    return ret;
}


static int extcmp(char *path, char *ext)
{
    char *p;

    Trace("extcmp: enter '%s' vs '%s'\n",path,ext);

    p = path;
    while(*p)
        p++;
    while(p!=path && *p!='.')
        p--;
    if(*p=='.') {
        Trace("extcmp: try '%s' vs '%s'\n",p,ext);
        return strcmp(p,ext);
    }
    Trace("extcmp: diff. %s vs %s\n",p,ext);
    return 1;
}



/*
 * Check hook or not
 */

int check_hook(char *name, char *type)
{
    int i;

    i = 0;
    while(conv_tbl[i].target[0]!='.') {
        if(type,conv_tbl[i].target[0] &&
          strncmp(type,conv_tbl[i].target,strlen(conv_tbl[i].target))==0) {
            Trace("check_hook: Found convertor %s type=%s (%d)\n",
                type,conv_tbl[i].target,i);
            return i;
        }
        i++;
    }

    i = 0;
    while(conv_tbl[i].target[0]!='.') {
        if(conv_tbl[i].target_ext[0] &&
          extcmp(name,conv_tbl[i].target_ext)==0) {
            Trace("check_hook: Found convertor %s ext=%s (%d)\n",
                type,conv_tbl[i].target_ext,i);
            return i;
        }
        i++;
    }

    return -1;
}

