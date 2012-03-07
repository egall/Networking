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
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define SERV_PORT 2424
#define LISTENQ 5
#define MAXLINE 4096

void sig_child(int signo){
    pid_t pid;
    int stat;
    while( (pid = waitpid(-1, &stat, WNOHANG)) > 0){
         printf("child %d terminated\n", pid);
    }
    return;
}

void proxy_http(int sockfd){
    ssize_t n;
    char line[10000], method[10000], url[10000], protocol[10000], host[10000], path[10000];
    char line[MAXLINE];
    while((n = read(sockfd, line, MAXLINE)) > 0){
        if(n < 0) perror("read failed\n");
        printf("line = %s\n", line);
    }
}


int main(int argc, char** argv){
    int listenfd, connfd;
    pid_t childpid;
    socklen_t cli_len;
    struct sockaddr_in cliaddr, servaddr;
    void sig_child(int);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);
    for(;;){
        cli_len = sizeof(cliaddr);
        if( (connfd = accept(listenfd, (struct sockaddr*) &cliaddr, &cli_len)) < 0){
            if(errno == EINTR) continue;
            else perror("accept error");
        }
        if( (childpid = fork()) == 0){
            close(listenfd);
            proxy_http(connfd);
            exit(0);
        }
        close(connfd);
     }
}
                
