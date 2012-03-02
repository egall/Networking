/*
 *  Simple alpha PROXY Server: HTTP Get Request etc.
 *  
 *  ORIGINAL by k-chinen@is.aist-nara.ac.jp, 1994
 *  UPDATED & EXTENDED by mike@vorburger.ch, Nov 1998
 *
 * $Id: http.c,v 1.1 1994/11/06 20:31:59 k-chinen Exp k-chinen $
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>

#include "wcol.h"

extern char Authorization_Line[256];

int Generate_Info(Info *info, char *name,
    		  int statu, long conn, long trans, long takes, 
    		  int http1_or_not, char *head)
{
    char line[STRING_SIZE];
    char *p,*q;
    time_t now;

    if(info==NULL) {
        return 1;
    }

    memset(info,0,sizeof(Info));
    strcpy(info->attr.id,WWWCOL_ID);
    strcpy(info->attr.name,name);
    if(http1_or_not)
        strcpy(info->attr.ver,"HTTP/1.x");
	info->attr.init_statu = statu;
    time(&now);
    info->attr.conn_time  = conn;
    info->attr.trans_time = trans;
    info->attr.take_time  = takes;
    info->attr.first = now;
    info->attr.last  = now;
    info->attr.count = 0;

    /*
     * analyze each line with care for dirty data
     */
    p = head;
    while(*p) {
        q = line;
        while(*p && *p!='\r' && *p!='\n')
            *q++ = *p++;
        *q = '\0';
        if(strncasecmp("Content-type: ",line,14)==0) {
            strcpy(info->attr.type,&line[14]);
        }
        while(*p && (*p=='\r' || *p=='\n'))
            p++;
    }
/*
    Trace("Generate_Info: done, Is it true ?\n",line);
    What(info);
*/

    return 0;
}





Info *HTTP_Get(char *name, char *opt, char *servername, int port, char *path)
{
    struct timeval  start_tp,  conn_tp,  end_tp;
    struct timezone start_tpz, conn_tpz, end_tpz;
    long conn, trans, takes;

    char *fname, *tmp_name;
    Info *info;
    int info_fd;
    int head_fd;
    int body_fd, sock_fd;
    int rcount, rtime, pos, hsum, bsum;
    char head[BUF_SIZE+2], buf[BUF_SIZE];
    int http1=1;
    int in_head;
    int server_statu;

    int secret_flag;


    if( port==-1 ) {
        port = HTTPPORT;
    }
    if( Authorization_Line[0] ) {
	secret_flag = 0;
    }
    else {
	secret_flag = 1;
    }

#if 1
    Trace("HTTP_get: HTTP %c opt=%s Server=%s Port=%d Path=%s. Authorization=%s\n",
		secret_flag ? 'S' : '-', opt, servername, port, path, Authorization_Line);
#endif

    /*
     * Setup HEAD file
     */
    fname = AllocName(name,HEAD_TMP_FILE);
    if(Setup(fname)) {
        Error("HTTP_get: Fail setup HEAD \"%s\".\n",fname);
        free(fname);
        return (Info*)NULL;
    }

    if((head_fd=open(fname,O_WRONLY|O_CREAT,0644))<0) {
        Error("HTTP_get: Cannot open HEAD file in pool\"%s\".\n",fname);
        free(fname);
        return (Info*)NULL;
    }
    free(fname);


    /*
     * Setup BODY file
     */

    fname = AllocName(name,BODY_TMP_FILE);
    if(Setup(fname)) {
        Error("HTTP_get: Fail setup BODY \"%s\".\n",fname);
        free(fname);
        return (Info*)NULL;
    }

    if((body_fd=open(fname,O_WRONLY|O_CREAT,0644))<0) {
        Error("HTTP_fet: Cannot open BODY file in pool\"%s\".\n",fname);
        free(fname);
        return (Info*)NULL;
    }
    free(fname);

    gettimeofday(&start_tp, &start_tpz);

    if((sock_fd=connect_server(servername,port))<0) {
        Error("HTTP_get: Cannot connect to %s...\n",servername);
        close(body_fd);

        return (Info*)NULL;
    }

    gettimeofday(&conn_tp, &conn_tpz);

    /*
     * Send Request to Server
     */
    sprintf(buf,
    "GET %s HTTP/1.0\r\nUser-Agent: %s\r\nAccept: */*\r\n%s\r\n",
	path, AGENT_INFO, Authorization_Line);

    rcount = write(sock_fd, buf, sizeof(char)*strlen(buf) );
    shutdown(sock_fd,1);
/*
    Trace("HTTP_get: Sent Request -- ... (rcount == %i, strlen(buf) == %i)\n%s",
		rcount, strlen(buf), buf );
*/

	/*
	 * Loop for HEAD parsing and writing
	 *
     	 * 		- Read Head+Body (HTTP/1)
	 * 		- Read Body (HTTP/0)
	 *
     	*/
    memset(head, 0, sizeof(char)*(BUF_SIZE+2) ); /* init and set sentinel */

    rtime = 0;
    hsum = 0;
    bsum = 0;
    in_head = 1;

    while(in_head && ( rcount=read(sock_fd,head,BUF_SIZE) ) >0 ) {
        /*
         * Check HTTP/1.0 or not in first time.
	 * If target server is not support HTTP/1, server replay
	 * as HTTP/0. Then we must be ready for reply of HTTP/0.
         */
        pos=0;

	/* Is it HTTP/1.x ? */
        if(rtime==0 && strncmp(head,"HTTP/1.",7)==0){
			server_statu = atoi(&head[9]);
			/*
			 * Accept only 2XX - 3XX, others must be reject !!!
			 */
			Trace("HTTP_get: Server replay status %d.\n", server_statu);
			if(server_statu==401) {
				Trace("Authorization Request");
			}
			else
			if(server_statu<200 || server_statu>400) {
				Error("HTTP_get: Ignore replay statu %d, Reject !\n",
					server_statu);
				return (Info*)NULL;
			}
        }
	/* HTTP/0 or rest of header */
	else {
            	pos = rcount;
            	http1 = 0;
		in_head = 0;
	}

        /*
         * Search separator between head and data in buffer
		 *
		 * 	 Remark:
		 *      - NCSA Server's reply was splited with '\n'   -> \n \n
		 *      - CERN Server's reply was splited with '\r\n' -> \r \n \r \n
		 *
		 *   So, We need search
		 *		- \n \n
		 *		- \n \r \n
		 *	 And, skip
		 *		- 2chars 
		 *		- 3chars
		 *
         */
        while(pos<=rcount && !(head[pos]=='\n' && head[pos+1]=='\n')
            && !(head[pos]=='\n' && head[pos+1]=='\r' && head[pos+2]=='\n'))
            pos++;
		
		Trace("HTTP_get: HEAD %d bytes %#x (in %d bytes).\n", pos, pos, rcount);
		

		/* case: This 'head' is HEADER and PART of BODY */
        if(pos<=rcount) {
            if(head[pos+1]=='\r')
                pos+=3;				/* Skip 3chars (\n \r \n) */
            else
                pos+=2;				/* Skip 2chars (\n \n)    */

			
		Trace("HTTP_get: Found separator in %d (of %d bytes).\n",
				pos, rcount);
			

            write(head_fd,head,pos);          		/* write head */
            write(body_fd,&head[pos],rcount-pos); /* write rest, so its body */
            bsum = rcount-pos;
            head[pos] = '\0';                 		/* truncate head */

			
    		Trace("HTTP_get: HEAD %d :BODY %d in %d.\n", pos, bsum, rcount);
			

			in_head = 0;
        }
		else {
			/* case: This 'head' is PART of HEADER */
			if(http1==1) {
				write(head_fd, head, rcount);		/* write head */
				head[0] = '\0';             		/* clear head */
				bsum = 0;
				
				Trace("HTTP_get: HEAD %d bytes.\n", rcount);
				
			}
			/* case: This 'head' is PART of BODY */
			else {
				write(head_fd, head,rcount);     	/* write body */
				bsum = rcount;
				head[0] = '\0';             		/* clear head */
				
				Trace("HTTP_get: NO HEAD :BODY %d in %d bytes.\n",
					bsum, rcount);
				
				in_head = 0;
			}
		}
		hsum += pos;
		rtime ++;
	}
	ftruncate(head_fd, hsum);
	close(head_fd);


	/*
	 * Loop for BODY writing
	 */
	while((rcount=read(sock_fd,buf,BUF_SIZE))>0) {
        if(write(body_fd,buf,rcount)!=rcount) {
            Error("HTTP_get: Cannot write reading %s\n",name);
        }
        bsum += rcount;
    }

    shutdown(sock_fd, 2);
    close(sock_fd);

    gettimeofday(&end_tp, &end_tpz);

    ftruncate(body_fd, bsum);
    close(body_fd);

    conn = (conn_tp.tv_usec/1000    + conn_tp.tv_sec*1000)
			- (start_tp.tv_usec/1000 + start_tp.tv_sec*1000);
    trans = (end_tp.tv_usec/1000     + end_tp.tv_sec*1000)
			- (conn_tp.tv_usec/1000  + conn_tp.tv_sec*1000);
    takes = (end_tp.tv_usec/1000     + end_tp.tv_sec*1000)
			- (start_tp.tv_usec/1000 + start_tp.tv_sec*1000);

    /* Trace("--- START '%s'\r\n", path); */
    Trace("HTTP_get: [byte] HEAD %d, BODY %d, TOTAL %d\r\n",
		hsum, bsum, hsum+bsum);
    Trace("HTTP_get: [ms]   CONN. %d, TRANS. %d, total %d\r\n",
		conn, trans, takes);

	if(bsum==0) {
        Error("HTTP_get: Empty data\n");
		return (Info*)NULL;
	}



	/*
	 * Swap temporary file and formal file.
	 */
	fname = AllocName(name, BODY_FILE);
	tmp_name = AllocName(name, BODY_TMP_FILE);
	rename(tmp_name, fname);
	free(fname);
	free(tmp_name);

	fname = AllocName(name, HEAD_FILE);
	tmp_name = AllocName(name, HEAD_TMP_FILE);
	rename(tmp_name, fname);
	free(fname);
	free(tmp_name);


    /*
     * Setup INFO file
     */
    fname = AllocName(name,INFO_FILE);
    if(Setup(fname)) {
        Error("HTTP_get: Fail setup INFO \"%s\".\n",fname);
        free(fname);
        return (Info*)NULL;
    }

    if((info_fd=open(fname,O_WRONLY|O_CREAT,0644))<0) {
        Error("HTTP_get: Cannot open INFO file in pool\"%s\".\n",fname);

        free(fname);
        return (Info*)NULL;
    }
    free(fname);

    /*
     * Analayse HEAD
     */
    info = (Info*)malloc(sizeof(Info));
    if(strcmp("REQUEST",opt)==0) {
    	Generate_Info(info, name, 1, conn, trans, takes, http1, head);
	info->attr.count = 1;
    }
    else
    	if(strcmp("PREFETCH",opt)==0) {
    		Generate_Info(info, name, 2, conn, trans, takes, http1, head);
	}
	else
    		Generate_Info(info, name, 0, conn, trans, takes, http1, head);


    if((rcount=write(info_fd,info,sizeof(Info)))<sizeof(Info)) {
        Error("HTTP_get: Cannot write INFO.(%d/%d)\r\n"
            ,rcount,sizeof(Info));

        return (Info*)NULL;
    }

    close(info_fd);

    return info;
}

