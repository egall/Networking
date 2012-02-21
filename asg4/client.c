/*
 * Erik Steggall
 * CMPS 156
 * 02/20/12
 * Assignement #4
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

#define MAXLINE 128
#define MAXSEND 512
#define BUFFSIZE 512
#define SERV_PORT 1234

int main(int argc, char** argv){
    int sock_fd;
    struct sockaddr_in servaddr;
    struct sctp_event_subscribe evnts;
    int echo_to_all = 0;
    if(argc < 2){
        perror("Missing host argument\nusage >./client [host]"); 
        return 1;
    }
    if(argc > 2){
        printf("echoing messages to all streams\n");
        echo_to_all = 1;
    }
    sock_fd = socket(AF_INET, SOCK_SEQPACKET, IPPROTO_SCTP);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
  
    bzero(&evnts, sizeof(evnts));
    evnts.sctp_data_io_event = 1;
    setsockopt(sock_fd, IPPROTO_SCTP, SCTP_EVENTS, &evnts, sizeof(evnts));
    if(echo_to_all == 0){
        sctpstr_cli(stdin, sock_fd, (struct sockaddr*) &servaddr, sizeof(servaddr));
    }
    else{
        sctpstr_cli_echoall(stdin, sock_fd, (struct sockaddr*) &servaddr, sizeof(servaddr));
    }
    close(sock_fd);
       
    return 0;
}
