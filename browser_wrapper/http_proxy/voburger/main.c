/*
 *  Simple alpha PROXY Server
 *  ("Deliverer program for HTTP, main routine.
      In frankly, It is HTTP proxy server with cache, prefetch and conversion.")
 *  
 *  ORIGINAL by k-chinen@is.aist-nara.ac.jp, 1994
 *  UPDATED & EXTENDED by mike@vorburger.ch, Nov 1998
 *
 *  $Id: main.c,v 1.4 1994/12/04 23:17:03 k-chinen Exp k-chinen $
 */



#include "wcol.h"



char Authorization_Line[256];


/*
 * http_error_messages - send error messages by HTML.
 */
int http_error_messages(int fd, int err_no, char *message, char *opt,
	int http1)
{
	char *msg_http1="HTTP/1.0 %d %s\r\n"
			"MIME-version: 1.0\r\nContent-type: text/html\r\n\r\n";
	char *msg_http0="<HEAD><TITLE>Wcol Warning Message</TITLE></HEAD>\r\n"
			"<BODY>\r\n<H1>%d %s</H1>\r\n%s\r\n<HR>\r\n<em>"
			AGENT_INFO
			" v" WWWCOL_VER
			"</em>\r\n</BODY>\r\n";
	char msg[STRING_SIZE];

	if(http1) {
		sprintf(msg, msg_http1, err_no, message);
		write(fd, msg, strlen(msg));
	}
	sprintf(msg, msg_http0, err_no, message, opt);
	write(fd, msg, strlen(msg));
}

/*
 * Reception
 *      fd:     socket (client request/output)
 */
int Reception(int fd, struct sockaddr_in *from)
{
    char *key;
    long int from_addr; 		/* need 4 bytes more */
    Info *info;
    char buf[BUF_SIZE+1];
    char line[STRING_SIZE];
    char cmd[STRING_SIZE];
    char path[STRING_SIZE];
    char opt[STRING_SIZE];
    char *p,*q,*r,*tmp;
    long n,c;
    int have_flag=0;
    int ready_flag=0;
    int force_flag=0;       	/* Force, for no-cache or sync */
    int header_end=0;

	unsigned char target[5];	/* */


	/*
	 * It is acceptable ?
	 */
    from_addr = ntohl(from->sin_addr.s_addr);
	target[0] = (unsigned char) (from_addr >> 24) & 0xFF;
	target[1] = (unsigned char) (from_addr >> 16) & 0xFF;
	target[2] = (unsigned char) (from_addr >>  8) & 0xFF;
	target[3] = (unsigned char) (from_addr >>  0) & 0xFF;
	target[4] = '\0';

	Trace("Access from %d.%d.%d.%d\n",
		target[0],target[1],target[2],target[3]);

	if(!Acceptable(target)) {
		/*
		 * Reject access
		 */
		Trace("Reject request from %d.%d.%d.%d\n",
			target[0],target[1],target[2],target[3]);
		
		shutdown(fd, 2);
		close(fd);

		return 1;
	}


	Authorization_Line[0] = '\0';

    /*
     * Read clients request
     *      if cannot read it, replace error mssage as dummy request.
     */
    if((c = read(fd,buf,BUF_SIZE))<=0) {
        strcpy(buf,"ERROR: Cannot read request socket\n");
        Trace("Ignore request, meybe empty request.\n");
    }
    buf[c]='\0';

#if 0
    Trace("\r\nClient said (%d bytes raw):\r\n%s\r\n--- END ---\r\n",c,buf);
#endif


    /* Cut 1 line to COMMAND */
    p = buf;
    q = line;
    while(*p&&*p!='\n'&&*p!='\r')
        *q++ = *p++;
    *q = '\0';
    Trace("Client said: <%s>\n", line);

    r = p;          /* store top of rest of command to 'r' */
    if(*r=='\r') r++;
    if(*r=='\n') r++; /* seek to next line */


    p = line;

    /* Cut COMMAND */
    q = cmd;
    while(*p&&*p!=' '&&*p!='\t'&&*p!='\r'&&*p!='\n')
        *q++ = *p++;
    *q = '\0';
    p++;

    /* Cut PATH */
    q = path;
    while(*p&&*p!=' '&&*p!='\t'&&*p!='\r'&&*p!='\n')
        *q++ = *p++;
    *q = '\0';

	tmp = NormalizeURL(path);
	strcpy(path, tmp);
	free(tmp);

    /* Cut OPT */
    opt[0] = '\0';
    if(*p)
        p++;
    if(*p&&*p!='\r' && *p!='\n')
        strcpy(opt,p);

    /*
    Trace("So...   CMD  = '%s'\n", cmd);
    Trace("        PATH = '%s'\n", path);
    Trace("        OPT  = '%s'\n", opt);
    */

    if(opt[0]) {
#if 0
        Trace("Parse rest of command. \n");
#endif

        header_end = 0;
        p = r;          /* restore rest of command from 'r' */
        do {

#ifdef DEBUG
			/*
			 * For Debug, Display 'What its read ?'
			 */
            Trace("Current Line [");
            q = p;
            if(*q) {
                while(*q&&*q!='\r'&&*q!='\n') {
                    Trace("%c",*q);
                    q++;
                }
                if(*q=='\0')
                    Trace("<NULL>");

                if(*q=='\r') {
                    Trace("<CR>");
					q++;
				}
                if(*q=='\n')
                    Trace("<LF>");
                Trace("]\n");
             }
             else {
                Trace("<EMPTY>]\n");
            }
#endif

            if(*p=='\r'||*p=='\n') {
                header_end = 1;
                break;
            }
            if(strncmp("Pragma:",p,7)==0) {
                p += 7;
                /*
                Trace("Oh. 'Pragma' found ... ");
                */
                while(*p==' '||*p=='\t')
                    p++;
                if(strncmp("no-cache",p,8)==0) {
					/*
                    Trace("no-cache ");
					*/
                    force_flag = 1;
                }
                /*
                Trace("\n");
                */
            }
            if(strncmp("Authorization:",p,14)==0) {
                Trace("Oh. 'Authorization' found ...'%s'.", p);
				q = Authorization_Line;
				while(*p && *p!='\r' && *p!='\n')
					*q++ = *p++;
				*q = '\0';
			}

            /* skip this line */
            while(*p&&*p!='\r'&&*p!='\n')
                p++;
            if(*p=='\r') p++;
            if(*p=='\n') p++;

            if((!header_end) && (!*p)) {
                if((c = read(fd,buf,BUF_SIZE))<=0) {
                    break;
                }
                else {
                    buf[c]='\0';
#if 0
    Trace("Client said more (%d bytes raw):\r\n%s\r\n--- END ---\r\n",c,buf);
#endif
                    p = buf;
                }
            }
        } while(!header_end);
    }
    else {
        Trace("No options...\n");
    }

#if 1
	if(force_flag) {
    	Trace("* FORCE Import (force_flag == TRUE)\n");
	}
	else {
    	Trace("* CHECK Import (force_flag == FALSE)\n");
	}
#endif

    if(strcmp("GET",cmd)==0) {
    	ready_flag=0;
        if(!force_flag && (have_flag=((info=Have(path))!=NULL))) {
            Trace("I have %s in the local file cache.\n",path);
            ready_flag = 1;
        }
        else {
            if((info=Get(path, "REQUEST"))!=NULL) {
                ready_flag = 1;
            }
            else {
                Error("Reception: GET Error %s\n",path);
            }
        }

        if(ready_flag) {
/* #ifdef DEBUG */
/*            What(info); */
/* #endif */

            Trace("SEND %s .\r\n",path);

            if(Send(fd, path, info, opt, Authorization_Line[0]!='\0')) {
                Error("SEND Error %s .\n",path);
                return 1;
            }

			/*
			 * Logging what doing.
			 */
            if(have_flag)
                Log("SEND %s .\n",path);
            else {
				if(force_flag)
                	Log("REGET-SEND %s .\n",path);
				else
                	Log("GET-SEND %s .\n",path);
			}
        }
		/*
		 * Not Found
		 */
        else {
			http_error_messages(fd,
				404, "Not Found", "Cannot get such URLs", opt[0]);
				
            Log("CANNOTGET %s\n",path);
        }
    }
	/*
	 * Unsupport Commands
	 */
    else {
		http_error_messages(fd,
			501, "Not Implemented", "Unsupport Command", opt[0]);
	
        Log("UNSUPPORT %s\n",path);
    }


    /*
     * Close Socket. So client will be free.
     */
    shutdown(fd,2);
    close(fd);


    /*
     * Try prefetch
     */
#ifdef PREFETCH
    if(ready_flag && prefetch_flag) 
		Prefetch(path);
#endif
	return 0;
}



void ReleseChild()
{
    /* union wait status; */

    while(wait3(NULL, WNOHANG, (struct rusage*) 0) >= 0 )
        ;
    /*
    Trace("ReleseChild:\n");
    */
}



int con_count;

Mainloop(int portno)
{
    int chpid;
    int netd;
    int fd;
    int len;
    int try_no;
    struct sockaddr_in from;

#ifdef DEBUG
    try_no = 0;
    while((netd=bind_port(portno))<=0) {
        Error("Port %d is not available.\n", portno);
        portno++;

        if(try_no++>10) {
            Fatal("Give up, cannot success bind() and listen() .\n");
            exit(1);
        }
    }
#else
	if((netd=bind_port(portno))<=0) {
        Error("Port %d is not available.\n", portno);
		exit(1);
	}
    Trace("Bind port %d.\n",portno);
#endif

    signal(SIGCHLD, ReleseChild);

    Trace("Waiting to connections at port %d ...\n", portno);

    while(1) {
        len = sizeof(from);
        if((fd=accept(netd, (struct sockaddr*)&from, &len)) < 0 ) {
            if(errno == EINTR)
                continue;

            Error("Mainloop: ERROR, cannot accept request, errno=%d.\n", errno);
            exit(2);
        }

        con_count++;
        Trace("\nNew Connection #%d, netd %d,fd %d\n", con_count, netd, fd);

        if((chpid=fork())==0) {
            /* ----- Child process ----- */
            close(netd);

            Reception(fd, &from);

            Trace("disconnection...#%d, pid=%d\n", con_count, getpid());
            exit(0);
        }
        else {
            /* ----- Parent process ----- */
            close(fd);
        }
    }
}



extern char *optarg;
extern int  optind;


/*
 * main:    init and boot
 */

int main(int argc, char *argv[])
{
    int i;
    int port_no=HTTPPORT;
    char conv_file[STRING_SIZE];
    char accept_file[STRING_SIZE];

    strcpy(pool_dir,  	DEFAULT_POOLDIR);
    strcpy(log_file,  	DEFAULT_LOGFILE);
    strcpy(conv_file, 	DEFAULT_CONVFILE);
    strcpy(accept_file, DEFAULT_ACCEPTFILE);
    keeptime = DEFAULT_KEEPTIME;

    while((i=getopt(argc, argv, "h?vp:d:l:a:c:tVf")) != -1) {
        switch(i) {
        case 'h':
        case '?':
            printf(AGENT_INFO
				"\nORIGINAL ver 0.02, Dec 1994 by k-chinen@is.aist-nara.ac.jp\n"
				  "UPDATED & EXTENDED Nov 1998 by mike@vorburger.ch\n\n");

            printf("Usage: %s [option]\n\n",argv[0]);
            printf("Option:  -h , -?          Help\n");
            printf("         -v               version\n");
#ifdef PREFETCH			
            printf("         -f               prefetch\n");
#endif
            printf("         -a file          accetable table file\n");
            printf("         -c file          convert table file\n");
            printf("         -k seconds       keep time\n");
            
            printf("         -t & -V          trace (Verbose)\n");
            printf("         -p no            port number\n");
            printf("         -d dir           pool directory\n\n");
            printf("         -l file          logfile\n\n");
            printf("example: %s -p 8100 -d /tmp/www-pool -l /tmp/wcol.log\n\n",
                argv[0]);
            exit(0);
            break;
	case 'v':
	    printf(AGENT_INFO " Version %s, data id %s\n", WWWCOL_VER, WWWCOL_ID);
            exit(0);
			break;
        case 'p':
            port_no = atoi(optarg);
            break;
        case 'd':
            strcpy(pool_dir,optarg);
            break;
        case 'l':
            strcpy(log_file,optarg);
            break;
        case 'V':
        case 't':
            trace_flag = 1 - trace_flag;
            break;
#ifdef PREFETCH
        case 'f':
            prefetch_flag = 1 - prefetch_flag;
            break;
#endif
        case 'a':
            strcpy(accept_file,optarg);
            break;
        case 'c':
            strcpy(conv_file,optarg);
            break;
        case 'k':
            keeptime = (time_t)atoi(optarg);
            break;
        }
    }

    Trace("Port=%d, Pooldir=%s, Convfile=%s, Acceptfile=%s\n",
		port_no, pool_dir, conv_file, accept_file);
    Trace("Trace=%d, Logfile=%s\n", trace_flag, log_file);
#ifdef PREFETCH
    Trace("Prefetch=%d, Keeptime=%d, Locktime=%d\n",
		prefetch_flag, keeptime, locktime);
#endif
    Log("HELLO\n");

    Read_Convtable(conv_file);
    Read_Accepttable(accept_file);

    Mainloop(port_no);
}
