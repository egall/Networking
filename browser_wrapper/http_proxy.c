/*
 * Erik Steggall
 * CMPS 156
 * 03/03/12
 * http_proxy.c
*/

/*
** http_proxy.c -- proxy http server
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <syslog.h>

#define SERV_PORT 2424
#define LISTENQ 5
#define MAXLINE 4096

static void trim( char* line ){
    int l;

    l = strlen( line );
    while ( line[l-1] == '\n' || line[l-1] == '\r' )
	line[--l] = '\0';
}

void sig_child(int signo){
    pid_t pid;
    int stat;
    while( (pid = waitpid(-1, &stat, WNOHANG)) > 0){
         printf("child %d terminated\n", pid);
    }
    return;
}

static int open_client_sock(char* hostname, unsigned short port){
    socklen_t cli_len;
    struct sockaddr_in cliaddr;
    cli_len = sizeof(cliaddr);
    struct hostent *he;
    int sock_family, sock_type, sock_protocol;
    int sockfd;
/*
    he = gethostbyname(hostname);
    if( he == (struct hostent*) 0){
        perror("Not found: Unknown host\n");
    }
    sock_family = he->h_addrtype;
    sock_type = SOCK_
*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) perror("Internal Error: Couldn't create socket.\n");
    if( connect(sockfd, (struct sockaddr*)&cliaddr, cli_len) < 0)
        perror("Service unavailable: connection refused\n");
    return sockfd;
} 

void get_request(int sockfd){
    ssize_t n;
    unsigned short port;
    char line[10000], method[10000], url[10000], protocol[10000], host[10000], path[10000];
    int iport;
    while((n = read(sockfd, line, MAXLINE)) > 0){
        if(n < 0) perror("read failed\n");
        trim(line);
        if(sscanf(line, "%[^ ] %[^ ] %[^ ]", method, url, protocol) != 3)
            perror("Bad request: Can't parse request\n");
        if( url == (char*) 0)
            perror("Bad request: NULL url\n");
        if( strncasecmp(url, "http://", 7) == 0){
	    (void) strncpy( url, "http", 4 );	/* make sure it's lower case */
	    if ( sscanf( url, "http://%[^:/]:%d%s", host, &iport, path ) == 3 ){
                printf("Got host, path and port\n");
	        port = (unsigned short) iport;
            }
	    else if ( sscanf( url, "http://%[^/]%s", host, path ) == 2 ){
                printf("Got host path\n");
	        port = 80;
            }
	    else if ( sscanf( url, "http://%[^:/]:%d", host, &iport ) == 2 ){
                printf("Got host and port\n");
	        port = (unsigned short) iport;
	        *path = '\0';
	    }
            else if ( sscanf( url, "http://%[^/]", host ) == 1 ){
                printf("Got host\n");
                port = 80;
                *path = '\0';
            }
            else
                perror("Bad Request: Can't parse URL.\n" );
        }
        else
            perror("Bad request: Unknown URL type.\n");

        printf("host = %s\n", host);   
        printf("port = %d\n", (int) port);


    }
}

void str_echo(int sockfd){
    ssize_t n;
    char buf[MAXLINE];
    while((n = read(sockfd, buf, MAXLINE)) > 0){
        if(n < 0) perror("read failed\n");
        printf("buf = %s\n", buf);
    }
}

int main(int argc, char** argv){
    int listenfd, connfd;
    pid_t childpid;
    socklen_t browser_len;
    struct sockaddr_in browseraddr, servaddr;
    void sig_child(int);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);
    for(;;){
        browser_len = sizeof(browseraddr);
        if( (connfd = accept(listenfd, (struct sockaddr*) &browseraddr, &browser_len)) < 0){
            if(errno == EINTR) continue;
            else perror("accept error");
        }
        if( (childpid = fork()) == 0){
            close(listenfd);
            get_request(connfd);
            exit(0);
        }
        close(connfd);
     }
}
                
