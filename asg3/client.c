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

FILE* dg_cli(FILE* fp, int sockfd, struct sockaddr* pservaddr, socklen_t servlen){
    /*
    int n;
    char sendline[MAXLINE], recvline[MAXLINE+1];

    connect(sockfd, (struct sockaddr*)pservaddr, servlen);

    while(fgets(sendline, MAXLINE, fp) != NULL){
        write(sockfd, sendline, strlen(sendline)); //, 0, pservaddr, servlen);
        n = read(sockfd, recvline, MAXLINE); //, 0, preply_addr, &len);

        recvline[n] = 0;
        fputs(recvline, stdout);
    }
    */
    return fp;
}

int main(int argc, char** argv){
    int sockfd;
    int n;
    char sendline[MAXLINE], recvline[MAXLINE+1];
    struct sockaddr_in servaddr;
    struct sockaddr* pservaddr;
    socklen_t servlen;
    FILE* fp;


    if(argc != 2){
        fprintf(stderr, "usage: >./server [port_number]\n");
        exit(1);
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    fp = dg_cli(stdin, sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
    pservaddr = (struct sockaddr*) &servaddr;
    servlen = sizeof(servaddr);
    connect(sockfd, (struct sockaddr*)pservaddr, servlen);

    while(fgets(sendline, MAXLINE, fp) != NULL){
        write(sockfd, sendline, strlen(sendline)); //, 0, pservaddr, servlen);
        n = read(sockfd, recvline, MAXLINE); //, 0, preply_addr, &len);

        recvline[n] = 0;
        fputs(recvline, stdout);
    }
    return 0;
}
