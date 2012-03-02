/*
 *  Simple alpha PROXY Server
 *  ("Deliverer Program, send (HTTP) procedures")
 *  
 *  ORIGINAL by k-chinen@is.aist-nara.ac.jp, 1994
 *  UPDATED & EXTENDED by mike@vorburger.ch, Nov 1998
 *
 *  $Id: send.c,v 1.1 1994/12/01 07:35:42 k-chinen Exp k-chinen $
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





static int Send_Head_Direct(int sfd, char *name)
{
    char *hpath;
    char buf[BUF_SIZE];
    int head_fd;
    int sum,c;

	
    Trace("Send_HEAD_Direct: name=%s\n",name);
	

    hpath = AllocName(name,HEAD_FILE);
    if((head_fd=open(hpath,O_RDONLY))<0) {
        free(hpath);
        return 1;
    }

    sum = 0;
    while((c=read(head_fd,buf,BUF_SIZE))>0) {
        write(sfd,buf,c);
	sum += c;
    }
    close(head_fd);

    Trace("SenT_HEAD_Direct: %d bytes.\n", sum);

    free(hpath);
    return 0;
}




static int Send_Head(int sfd, char *name, char *type)
{
    char *hpath;
    char buf[BUF_SIZE];
    char X[] = "X-";
    char *p,*q;
    FILE *head_fp;
    int sum,c;

	
    Trace("Send_HEAD 'cooked' : name=%s, type=%s\n",name, type);
	

    hpath = AllocName(name,HEAD_FILE);
    if((head_fp=fopen(hpath,"r"))==NULL) {
        free(hpath);
        return 1;
    }

    sum = 0;
    while( fgets(buf, BUF_SIZE, head_fp) != NULL &&
        	(buf[0] != '\0'  &&  buf[0] != '\n'  &&  buf[0] != '\r')) {
        /*
         * hook output contents-type
         */
        if( strncasecmp("Content-Type: ", buf, 14) == 0 ) {
            if( type[0] == '\0' ) {
                write(sfd, buf, strlen(buf));
		sum += strlen(buf);

                Trace("Send_Head Output (default) %s", buf);
            }
            else {
                /* BAD, BAD: sprintf(buf, "X-%s", buf);
		   This overwrites buf, at least under Linux. Better: */

		write(sfd, X, strlen(X) );
		sum += strlen(X);

                write(sfd, buf, strlen(buf));
		sum += strlen(buf);

                sprintf(buf, "Content-Type: %s\n",type);
                write(sfd, buf, strlen(buf));
		sum += strlen(buf);

                Trace("Send_Head Output (hooked for Content-Type) %s", buf);
            }
        }
        else {
            write(sfd,buf,strlen(buf));
	    sum += strlen(buf);
        }
    }
    
    sprintf(buf,"X-ProxyAgent: %s\n", AGENT_INFO);
    write(sfd,buf,strlen(buf));

    sprintf(buf,"X-Target: %s\n\n",name);
    write(sfd,buf,strlen(buf));

    fclose(head_fp);

    Trace("SenT_HEAD 'cooked' : %d bytes.\n", sum);

    return 0;
}






/*
 * Send_Body_Direct
 *      sfd:    output socket
 *      name:   logical name
 *      opt:    options
 */

static int Send_Body_Direct(int sfd, char *name, Info *info, char *opt,
	int discard)
{
    char buf[BUF_SIZE];
    int c,sum;
    char *path;
    int fd;

    Trace("Send_Body_Direct: name=%s, opt=%s\n",name,opt);

    /*
     * Head Transfer
     */
    if(strncmp(name,"http:",5)==0 && strncmp(opt,"HTTP/1.",7)==0) {
        Send_Head_Direct(sfd, name);
    }

    path = AllocName(name,BODY_FILE);
    if((fd=open(path,O_RDONLY))<0) {
        Error("Send_Body_Direct: ERROR: Cannot open file %s.\n",path);

        free(path);
        return 1;
    }

    sum = 0;
    while((c=read(fd,buf,sizeof(buf)))>0) {
        write(sfd,buf,c);
        sum += c;
    }
    close(fd);

    shutdown(sfd,1);
    close(sfd);

    Trace("SenT_Body_Direct: BODY %d bytes\r\n",sum);

	if(discard) {
		DiscardInfo(name);
	}
	else {
		UpdateInfo(name);
	}

    free(path);
    return 0;
}


static int Send_Homepage(int sfd) 
{

};


static int Send_Info(int sfd, char* path)
{
	Info  info;
    char* name;

    int fd;
    int c, found = 0;

	char* msg_error = "The requested path %s is not in the cache. (%s)";
	char* msg_http =  "Path: %s\n<P>Count: %d";
	char msg[ strlen(msg_http) + 100 ];

    if( CheckLock(path) )
		return -1;

    name = AllocName(path, INFO_FILE);
    if ( (fd = open(name,O_RDONLY)) < 0 ) {
        Trace("Send_Info: Cannot open or does not exist %s from %s.\n", name, path);
		sprintf(msg, msg_error, path, name);
	}
	else
	{
		c = read(fd, &info, sizeof(Info) );
		close(fd);

		if ( c != sizeof(Info) ) {
			Trace("Send_Info: Read of Info failed. c = %i, sizeof(Info) = %i.", c, sizeof(Info) );
			free(name);
			return -3;
		};

		sprintf(msg, msg_http, info.attr.name, info.attr.count);
	};	

	c=write(sfd, msg, strlen(msg) );
	if ( c != strlen(msg) )
		Trace("Send_Info: write() not complete.");

	free(name);
    return 0;

	
	/*
    fprintf(stdout,"HTTP Version:   %s\n",info->attr.ver);
    fprintf(stdout,"Name:           %s\n",info->attr.name);
    fprintf(stdout,"Type:           %s\n",info->attr.type);
    fprintf(stdout,"Connect Time:   %d\n",info->attr.conn_time);
    fprintf(stdout,"Translate Time: %d\n",info->attr.trans_time);
    fprintf(stdout,"Take Time:      %d\n",info->attr.take_time);
    fprintf(stdout,"First Date:     %s",ctime(&info->attr.first));
    fprintf(stdout,"Last Date:      %s",ctime(&info->attr.last));
    fprintf(stdout,"Count:          %d\n",info->attr.count);

    fprintf(stdout,"\n");
    fflush(stdout);
*/
};


static int Send_Stats(int sfd, char *name, char* opt)
{
    char servername[STRING_SIZE], path[STRING_SIZE];
    char portname[STRING_SIZE], protocol[STRING_SIZE];
    int  port;
	int  c;

	char *msg_http = "HTTP/1.0 200 OK\r\n"
			/* "MIME-version: 1.0\r\n" */
			 "Server: " AGENT_INFO "\r\n"
			 "Content-Type: text/html\r\n\r\n"
	"<HEAD><TITLE>alpha ware PROXY Server Information</TITLE></HEAD>\r\n"
	"<BODY>\r\n<H1>PROXY Server Information</H1>\r\n";
/*	"X-X-X\r\n</BODY></HTML>";
	"%s\r\n</BODY></HTML>\r\n"; */


    ParseURL(name, protocol, servername, portname, path);
    port = atoi(portname);

	c = write(sfd, msg_http, strlen(msg_http) );
	if ( c != strlen(msg_http) )
		Trace("Send_Stats: write() maybe failed: " "c = %i, strlen(msg_http) = %i, sizeof(msg_http) = %i\n", c, strlen(msg_http), sizeof(msg_http) );

	if ( strcmp(path, "://:/")==0 || strcmp(path, "/")==0 ) { /* That's an ugly compare, I know. */
		/* Homepage, no path. */
		Send_Homepage(sfd);
	} else {
		/* A path was added to the info/stat request. */
		Send_Info(sfd, path);
	};

	shutdown(sfd, 2);
	close(sfd);
	return 0;
};




int Send(int sfd, char *name, Info *info, char *opt, int discard)
{
    char buf[BUF_SIZE];
    int c,sum;
    int fd,trans_type;
    char *path;
    char *tmp_path;
    int pos;
    char pgn[STRING_SIZE];


    if ( info->attr.count == -7 ) /* This is a special flag set in get() in get.c */
    {
	Trace("Send: count == -7 -> Send_Stats!\n");

    	Send_Stats(sfd, name, opt);
    
	shutdown(sfd, 2);
	close(sfd);
	return 0;    

    };	

    path = AllocName(name,BODY_FILE);
    tmp_path = tmpnam((char*)NULL);
    if((pos = check_hook(name,info->attr.type))<0) {
        return Send_Body_Direct(sfd, name, info, opt, discard);
    }

    Trace("Send_Body_Conv: name=%s, opt=%s\n",name,opt);

	/* Use inner-function */
	if(conv_tbl[pos].program[0]=='!') {
/*		if(strcmp(conv_tbl[pos].program,"!tosjis")!=0 &&
			strcmp(conv_tbl[pos].program,"!toeuc")!=0) {
*/
    		Error("Send_Body_Conv: Not support function. What is '%s' ?\n",
				conv_tbl[pos].program);
        	return Send_Body_Direct(sfd, name, info, opt, discard);
/*		}
*/
		if(strncmp(name,"http:",5)==0 && strncmp(opt,"HTTP/1.",7)==0) {
			Send_Head(sfd, name, conv_tbl[pos].output);
		}

		if((fd=open(path,O_RDONLY))<0) {
			Error("Send_Body_Conv: ERROR: cannot open %s .\n",tmp_path);
			free(path);
			return 1;
		}
/*
		if(strcmp(conv_tbl[pos].program,"!tosjis")==0) {
    		Trace("Send_Body_Conv: tosjis() start ... ");
			tosjis(fd, sfd);
		}
		else
		if(strcmp(conv_tbl[pos].program,"!toeuc")==0) {
    		Trace("Send_Body_Conv: toeuc() start ... ");
			toeuc(fd, sfd);
		}
*/
		close(fd);
    		Trace("done.\n");
	}

	/* Call external program */
	else {
		trans_type =
			conv_program_name(pgn, conv_tbl[pos].program, path, tmp_path);

		Trace("Send_Body_Conv: exec '%s'\n", pgn);

		if(fork()==0) {
			if(strncmp(name,"http:",5)==0 && strncmp(opt,"HTTP/1.",7)==0) {
				Send_Head(sfd, name, conv_tbl[pos].output);
			}

			if(trans_type & CONV_HAS_OUTPUT) {
				/*
				 *	Have output-file
				 *  	1. convert input to tmp-file as output
				 *		2. send tmp-file to socket.
				 */

				system(pgn);

				if((fd=open(tmp_path,O_RDONLY))<0) {
					Error("Send_Body_Conv: ERROR: cannot open %s .\n",tmp_path);
				}
				sum = 0;
				while((c=read(fd,buf,BUF_SIZE))>0) {
					write(sfd,buf,c);
					sum += c;
				}

				close(fd);
				unlink(tmp_path);
			}
			else {
				/*
				 *	Not have output-file
				 *  	1. duplicate socket for output
				 *		2. call convert program
				 */

				dup2(sfd,1);

				system(pgn);
			}

			shutdown(sfd,0);
			close(sfd);

			exit(1);
    	}
	}

    	shutdown(sfd,0);
    	close(sfd);

	if(discard) {
		DiscardInfo(name);
	}
	else {
		UpdateInfo(name);
	}


    free(path);

    return 0;
}
