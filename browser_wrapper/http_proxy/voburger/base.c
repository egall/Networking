/*
 *  Simple alpha PROXY Server
 *  ("Deliverer Program, file handle and remote access procedures")
 * 
 *  ORIGINAL by k-chinen@is.aist-nara.ac.jp, 1994
 *  UPDATED & EXTENDED by mike@vorburger.ch, Nov 1998 
 *
 *  $Id: base.c,v 1.4 1994/12/18 18:01:08 k-chinen Exp k-chinen $
 */

#include <stdio.h>
#include <stdlib.h>
#ifndef SEEK_SET
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>

#include "wcol.h"



time_t locktime=DEFAULT_LOCKTIME;
time_t keeptime=DEFAULT_KEEPTIME;

char pool_dir[STRING_SIZE];
char log_file[STRING_SIZE];

int verbose = 0;
int trace_flag = 0;


/*
 * ParseURL - Parse URL and split to protocol, host, port and path.
   NEW: Args that are of no interest are allowed to be NULL.
        port cannot be checked if host==NULL, though.
*/
int ParseURL(char *url,
	char *protocol, char *host, char *port, char *path)
{
	register char *p,*q;
	char *r;

	if ( url == NULL )
		return 1;

    p = url;
    /* Skip whites */
    while(*p&&(*p==' '||*p=='\t'||*p=='\r'||*p=='\n'))
        p++;

	/*
     * PROTOCOL
     */
	if ( protocol != NULL ) {
		r = p;
		q = protocol;
		while(*p&&*p!=':')
			*q++ = *p++;
		*q = '\0';

		/* No protocol */
		if(*p!=':') {
			protocol[0] = '\0';
			p = r;
		}
		else {
    		p++;
		}
	};

	/*
     * HOST and PORT
     */
	if ( host != NULL && port != NULL ) {
		if(*p=='/' && *(p+1)=='/') {
			p++;
			p++;

			q = host;
			while(*p&&*p!='/')
				*q++ = *p++;
			*q = '\0';

			/* check Port */
			q = host;
			while(*q&&*q!=':')
				q++;
			if(*q==':') {
				*q = '\0';
				q++;

				strcpy(port, q);
			}
			/* Not port */
			else {
				port[0] = '\0';
			}

		}
		/* No host */
		else {
			host[0] = '\0';
			port[0] = '\0';
		}
	};

	/*
     * PATH
     */
	if ( path != NULL ) {
		q = path;
		while(*p)
			*q++ = *p++;
		*q = '\0';
	};

	return 0;
}



/*
 * NormalizeURL - URL Normarization
 */
char *NormalizeURL(char *target)
{
	char tmp[STRING_SIZE];
	char *ret;

	char protocol[STRING_SIZE];
    char host[STRING_SIZE], port[STRING_SIZE];
	char path[STRING_SIZE];
	int	 chk;

	register char *p, *q;


	chk = ParseURL(target, protocol, host, port, path);

/*
   Trace("\tprotocol '%s'\n\thost     '%s'\n\tport     '%s'\n\tpath     '%s'\n",
        protocol, host, port, path);
*/

#if 0
	if(protocol[0]='\0') {
		strcpy(protocol, "file");
		strcpy(host, "localhost");
		strcpy(port, "");
		strcpy(path, url);

		return 0;
	}
#endif

	if(port[0]=='\0') {
		if(strcmp(protocol,"http")==0) {
			strcpy(port, "80");
		}
		else
		if(strcmp(protocol,"ftp")==0) {
			strcpy(port, "21");
		}
		else {
			port[0] = '\0';
		}
	}

/*
   Trace("\tprotocol '%s'\n\thost     '%s'\n\tport     '%s'\n\tpath     '%s'\n",
        protocol, host, port, path);
*/

	sprintf(tmp, "%s://%s:%s%s", protocol, host, port, path);
	ret = (char*) malloc(sizeof(char)*(strlen(tmp)+1));
	strcpy(ret, tmp);

/*
	Trace("Normalization: Result '%s'\n", ret);
*/

	return ret;
}


/*
 * AllocName - alloc pathname with URL
 */
char *AllocName(char *name, int cmd)
{
    char tmp[STRING_SIZE];
    char *ret;
    register char *p,*q;

    char servername[STRING_SIZE], path[STRING_SIZE];
	char portname[STRING_SIZE], protocol[STRING_SIZE];

    /*
     * Check last char
     *      e.g.,
     *          proto://host/       -> proto://host/PADDING_NAME
     *          proto://host        -> proto://host/PADDING_NAME
     *          proto://host/path/  -> proto://host/path/PADDING_NAME
     *          (null)              -> /PADDING_NAME
     */
    if(*name) {
        p = q = name;

        p++;
        while(*p++)
            q++;
        if(*q=='/')
            sprintf(tmp,"%s/%s%s",pool_dir,name,PADDING_NAME);
        else {
            p = name;
            while(*p && *p!='/')
                p++;
            if(*p=='/' && *(p+1)!='/') {    /* Don't have double slashs */
                sprintf(tmp,"%s/%s",pool_dir,name);
            }
            else {
                q = p+2;
                while(*q && *q!='/')
                    q++;
                if(*q!='/') {               /* Don't have path, only host */
                    sprintf(tmp,"%s/%s/%s",pool_dir,name,PADDING_NAME);
                }
                else {                      /* Have path */
                    sprintf(tmp,"%s/%s",pool_dir,name);
                }
            }

        }
    }
    else
        sprintf(tmp,"%s/%s",pool_dir,PADDING_NAME);



    /* tmp + ext + \0 */
    ret = (char*)malloc(sizeof(char)*(strlen(tmp)+9+1));


    /*
     * copy from 'tmp' to 'ret'.
	 * replace '/' and ':' to '/'.
     */
    p = tmp;
    q = ret;

    while(*p) {
        if(*p==':')
            *q = '/';
        else
            *q = *p;

        if(*q=='/' && *(p+1)=='/') {
            /* Nothing, skip 1st slash in double slashs, (e.g., // -> / ) */
        }
        else
            q++;

        p++;
    }
    *q = '\0';


    switch(cmd) {
    case INFO_FILE:
        strcat(ret, INFO_EXT);
        break;
    case LOCK_FILE:
        strcat(ret, LOCK_EXT);
        break;
    case HEAD_FILE:
        strcat(ret, HEAD_EXT);
        break;
    case HEAD_TMP_FILE:
        strcat(ret, HEAD_TMP_EXT);
        break;
    case BODY_TMP_FILE:
        strcat(ret, BODY_TMP_EXT);
        break;
    }

/*
    Trace("AllocName: path=%s\n\t->%s\n", name, ret);
*/
    return ret;
}


int CheckLock(char *path);

/*
 *
 */

Info *Have(char *path)
{
    Info tmp,*ret;
    char *name;
    time_t now;
    int fd;
    int c,found=0;

    ret = (Info*)NULL;

	Trace("Have? path = %s\n", path);
 	
    if(CheckLock(path)) {
        return ret;
    }

    name = AllocName(path,INFO_FILE);
    if((fd=open(name,O_RDONLY))<0) {
        /*
        Error("Have: ERROR: Cannot open file %s.\n",name);
        */
        
        Trace("Have: Cannot open / does not exist %s from %s.\n", name, path);
        
    }
    else {
        time(&now);
        if((c=read(fd, &tmp, sizeof(Info)))) {
            if(strncmp(tmp.attr.id,WWWCOL_ID,strlen(WWWCOL_ID))==0
                && now - tmp.attr.last < keeptime) {
/*
                Trace("Have: Open file %s .\n",path);
*/

                ret = (Info*)malloc(sizeof(Info));
                memcpy((void*)ret,(void*)&tmp,sizeof(Info));
            }
            else {
                /*
                Error("Have: ERROR: Open file %s, but ignore ID.\n",path);
                */
                Trace("Have: Ignore ID of '%s'\n",path);
            }
        }
        else {
            Error("Have: ERROR: Open file %s, but it is empty.\n",path);
        }
        close(fd);
    }

    free(name);
    return ret;
}



int What(Info *info)
{
    if(info==NULL) {
        fprintf(stdout,"---------------------------\n");
        fprintf(stdout,"Null, no data !!!\n");
        fprintf(stdout,"---------------------------\n");
        return 1;
    }

    fprintf(stdout,"---------------------------\n");
    fprintf(stdout,"Identifier:     %s\n",info->attr.id);
    fprintf(stdout,"Version:        %s\n",info->attr.ver);
    fprintf(stdout,"Name:           %s\n",info->attr.name);
    fprintf(stdout,"Type:           %s\n",info->attr.type);
    fprintf(stdout,"Connect Time:   %d\n",info->attr.conn_time);
    fprintf(stdout,"Translate Time: %d\n",info->attr.trans_time);
    fprintf(stdout,"Take Time:      %d\n",info->attr.take_time);
    fprintf(stdout,"First Date:     %s",ctime(&info->attr.first));
    fprintf(stdout,"Last Date:      %s",ctime(&info->attr.last));
    fprintf(stdout,"Count:          %d\n",info->attr.count);
    switch(info->attr.init_statu) {
    case 1:
        fprintf(stdout,"Init Status:    Requested\n");
        break;
    case 2:
        fprintf(stdout,"Init Status:    Prefetched\n");
        break;
    default:
        fprintf(stdout,"Init Status:    Unknow [%d]\n",info->attr.init_statu);
        break;
    }
    fprintf(stdout,"\n");
    fflush(stdout);

    return 0;
}



/*
 * JoinURL --- join name to base-name (URL or path)
 */
char *JoinURL(char *base, char *name)
{
    char *work;
    char *p, *q;

/*
    Trace("JoinURL: base=%s, name=%s\n",base,name);
*/

    work = (char*)malloc(sizeof(char)*(strlen(base)+strlen(name)+1));

    /* Skip '#' */
    if(*name=='#') {
        strcpy(work,base);
        return work;
    }

    p = name;
    q = work;
    while(*p&&*p=='/')
        *q++ = *p++;
    *q = '\0';
    p = name;
    while(*p&&*p!=':')
        p++;

    /*
     * have protocol
     */
    if(*p==':') {
        strcpy(work,name);
    }
    /*
     * have pass (without protocol)
     */
    else if(*name=='/') {
        p = base;
        q = work;
        while(*p&&*p!=':')
            *q++ = *p++;
        /* copy '://' */
        *q++= *p++;
        *q++= *p++;
        *q++= *p++;
        while(*p&&*p!='/')
            *q++ = *p++;
        strcpy(q,name);
    }
    else {
    /*
     * append name to base (without filename)
     */
        strcpy(work,base);
        p = work + strlen(work);
        while(p!=work && *p!='/')
            p--;
        if(*p=='/') {
            p++;
            strcpy(p, name);
        }
    }

/*
    Trace("JoinURL: base=\"%s\", name=\"%s\"\n\treturn=\"%s\"\n",
        base,name,work);
*/
    return work;
}



/*
 * Setup - make path
 */
int Setup(char *name)
{
    char *p;
    char tmp[STRING_SIZE], oldname[STRING_SIZE];
    struct stat sbuf;
    int ret=0;

/*
    Trace("Setup: target=\"%s\".\n",name);
*/

    /*
     * Loop parts of name separated by '/'
     */
    p = tmp;
    while(*name) {
        if(*name=='/') {
            *p++ = *name++;
        }
        while(*name&&*name!='/')  {
            *p++ = *name++;
        }
        *p = '\0';

        /*
        Trace("* Setup: check \"%s\".\r\n",tmp);
        */

        /* name is not terminated --- so, it is directory */
        if(*name) {
            /*
             * Not found it. So, create such directory.
             */
            if(stat(tmp,&sbuf)) {
                if(mkdir(tmp,040755)<0) {
                    Error("Setup: Cannot mkdir %s\n",tmp);
                    ret = 1;
                    break;
                }
            }
            /*
             * Found it. Is it directory ?
             */
            else {
                /* it is direcotry */
                if(sbuf.st_mode & S_IFDIR) {
                    continue;
                }

                /* it is not direcotry */
#if 0
                Trace("Setup: Exist same file. realry ?\n");
#else
                strcpy(oldname, tmp);
                strcat(oldname, ",old");

                Trace("Setup: Exist same name file. rename to ,old and mkdir new one\n");
                rename(tmp,oldname);
                if(mkdir(tmp,040755)<0) {
                    Error("Setup: Cannot mkdir %s\n",tmp);
                    ret = 1;
                    break;
                }
#endif
            }
        }
        /* name is terminated --- so, it is path (directory+filename) */
        else {
            break;
        }
    }

    return ret;
}




/*
 * CheckLock - check 'Is it locked ?'
 */

int CheckLock(char *name)
{
    struct stat sbuf;
    char *path;
    int c,found=0;
    time_t now;

    c = 0;
    path = AllocName(name,LOCK_FILE);
    while(!stat(path,&sbuf)) {
        /*
         * locked long time, unlock
         */
        time(&now);
        if(now-sbuf.st_mtime>locktime) {
            Trace("CheckLock: %s was locked, But it is too old. remove !\n",
                name);

            unlink(path);
            return 0;
        }

        Trace("CheckLock: Locked (%dth try) %s\n\t%s\n",c,name,path);
        if(c>=LOCKRETRY_TIME) {
/*
            Trace("CheckLock: Locked %s\n\t%s\n",c,name,path);
*/
            free(path);
            return -1;
        }

        sleep(LOCKRETRY_INTERVAL);
        c++;
    }
    free(path);

    return 0;
}


int old_CheckLock(char *path)
{
    struct stat sbuf;
    char *name;
    int c,found=0;
    time_t now;

    c = 0;
    name = AllocName(path,LOCK_FILE);
    while(!stat(name,&sbuf)) {
        /*
         * locked long time, unlock
         */
        time(&now);
        if(now-sbuf.st_mtime>locktime) {
            Trace("CheckLock: %s was locked, But it is too old. remove !\n",
				path);

            unlink(name);
            return 0;
        }

        Trace("CheckLock: Locked (%dth try) %s\n\t%s\n",c,path,name);
        if(c>=LOCKRETRY_TIME) {
/*
            Trace("CheckLock: Locked %s\n\t%s\n",c,path,name);
*/
            free(name);
            return -1;
        }

        sleep(LOCKRETRY_INTERVAL);
        c++;
    }
    free(name);

    return 0;
}




/*
 *  Lock - make LOCK-file.
 */
int Lock(char *name)
{
    char *fname;
    int fd;

    fname = AllocName(name, LOCK_FILE);
    if(Setup(fname)) {
        free(fname);

        Error("Lock: Cannot setup %s\n", fname);
        return 1;
    }
    if((fd=open(fname, O_WRONLY|O_CREAT, 0644))<0) {
        free(fname);

        Error("Lock: Cannot lock %s\n", fname);
        return 1;
    }
    close(fd);

    free(fname);
    return 0;
}

/*
 * UnkLock - remove LOCK-file.
 */
int UnLock(char *name)
{
    char *fname;
    int fd;

    fname = AllocName(name, LOCK_FILE);
    unlink(fname);

    free(fname);
    return 0;
}




int UpdateInfo(char *name)
{
    Info tmp;
    char *path;
    time_t now;
    int fd;
    int c;
    int done=1;

    Lock(name);

    path = AllocName(name,INFO_FILE);
    if((fd=open(path,O_RDWR))<0) {
        Error("UpdateInfo: ERROR: Cannot open file %s.\n",path);
    }
    else {
        time(&now);
        if((c=read(fd, &tmp, sizeof(Info)))) {
            if(strncmp(tmp.attr.id,WWWCOL_ID,strlen(WWWCOL_ID))==0) {
/*
                Trace("UpdateInfo: Open file %s (size=%d).\n",path,c);
*/
                tmp.attr.count ++;
                tmp.attr.last = now;
                lseek(fd, 0L, SEEK_SET);
                if((c = write(fd, &tmp, sizeof(Info)))==sizeof(Info)) {
/* */
                    Trace("UpdateInfo: inc. to %d, %s .\n",
                        tmp.attr.count, name);
/* */
                }
                else {
                    Error("UpdateInfo: ERROR: Cannot write to %s (size=%d).\n",
                        path, c);
                }
                done = 0;
            }
            else {
                Error("UpdateInfo: ERROR: Open file %s, but ignore ID.\n",
                    path);
            }
        }
        else {
            Error("UpdateInfo: ERROR: Open file %s, but it is empty.\n",path);
        }
        close(fd);
    }

    free(path);

    UnLock(name);

    return done;
}



int DiscardInfo(char *name)
{
    char *path;

    Lock(name);

    path = AllocName(name,INFO_FILE);
    unlink(path);

    UnLock(name);

    Trace("DiscardInfo: name=%s.\n", name);

    return 0;
}

