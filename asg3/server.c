/*
 * Erik Steggall
 * CMPS 156
 * 02/15/12
 * Assignement #3
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

#define SERV_PORT 1234
#define MAXLINE 128
#define MAXSEND 512


void dg_echo(int sockfd, struct sockaddr * pcliaddr, socklen_t clilen){
    int n;
    socklen_t len;
    char mesg[MAXLINE];

    for(;;){
        bzero(&mesg, sizeof(mesg));
        len = clilen;
        n = recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);
        printf("msg = %s\n", mesg);
        sendto(sockfd, mesg, n, 0, pcliaddr, len);
    }
}

int main(int argc, char** argv){
    int sockfd;
    int n;
    socklen_t len, clilen;
    char mesg[MAXLINE];
    struct sockaddr_in servaddr, cliaddr;
    struct sockaddr * pcliaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);

    bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
//    dg_echo(sockfd, (struct sockaddr*) &cliaddr, sizeof(cliaddr));
    pcliaddr = (struct sockaddr*) &cliaddr;
    clilen = sizeof(cliaddr);
    for(;;){
        bzero(&mesg, sizeof(mesg));
        len = clilen;
        n = recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);
        printf("msg = %s\n", mesg);
        sendto(sockfd, mesg, n, 0, pcliaddr, len);
    }
}
    
