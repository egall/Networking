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
#include <netinet/sctp.h>
#include <time.h>

#define MAXLINE 128
#define MAXSEND 512
#define BUFFSIZE 512
#define SERV_PORT 1234
#define SERV_MAX_SCTP_STRM 8

void sctpstr_cli(FILE* fp, int sock_fd, struct sockaddr* to, socklen_t tolen){
    struct sockaddr_in peeraddr;
    struct sctp_sndrcvinfo sri;
    char sendline[MAXLINE], recvline[MAXLINE];
    socklen_t len;
    int out_sz, rd_sz;
    int msg_flags;

    bzero(&sri, sizeof(sri));
    while(fgets(sendline, MAXLINE, fp) != NULL){
        if(sendline[0] != '['){
            printf("Error, usage: > [streamnum] text\n");
            continue;
        }
        printf("sendline = %s\n", sendline);
        sri.sinfo_stream = strtol(&sendline[1], NULL, 0);
        out_sz = strlen(sendline);
        sctp_sendmsg(sock_fd, sendline, out_sz, to, tolen, 0, 0, sri.sinfo_stream, 0, 0);
        len = sizeof(peeraddr);
        rd_sz = sctp_recvmsg(sock_fd, recvline, sizeof(recvline), (struct sockaddr*) &peeraddr, &len, &sri, &msg_flags);
        printf("From str:%d seq:%d (assoc:0x%x:", sri.sinfo_stream, sri.sinfo_ssn, (u_int) sri.sinfo_assoc_id);
        printf("%.*s", rd_sz, recvline);
    }
}
void sctpstr_cli_echoall(FILE* fp, int sock_fd, struct sockaddr* to, socklen_t tolen){
    struct sockaddr_in peeraddr;
    struct sctp_sndrcvinfo sri;
    char sendline[MAXLINE], recvline[MAXLINE];
    socklen_t len;
    int itor,  rd_sz, str_sz;
    int msg_flags;

    bzero(&sri, sizeof(sri));
    bzero(sendline, sizeof(sendline));

    while(fgets(sendline, MAXLINE - 9, fp) != NULL){
        str_sz = strlen(sendline);
        if(sendline[str_sz-1] == '\n'){
            sendline[str_sz-1] = '\0';
            str_sz--;
        }
        printf("echo sendline = %s\n", sendline);
        for(itor = 0; itor < SERV_MAX_SCTP_STRM; itor++){
            snprintf(sendline + str_sz, sizeof(sendline) - str_sz, ".msg.%d", itor);
            rd_sz = sizeof(sendline);
            sctp_sendmsg(sock_fd, sendline, rd_sz, to, tolen, 0, 0, itor, 0, 0);
        }
        for(itor = 0; itor < SERV_MAX_SCTP_STRM; itor++){
            len = sizeof(peeraddr);
            rd_sz = sctp_recvmsg(sock_fd, recvline, sizeof(recvline), (struct sockaddr*) &peeraddr, &len, &sri, &msg_flags);
            printf("From str:%d seq:%d (assoc:0x%x:", sri.sinfo_stream, sri.sinfo_ssn, (u_int) sri.sinfo_assoc_id);
            printf("%.*s", rd_sz, recvline);
        }
    }
}



int main(int argc, char** argv){
    int sock_fd;
    struct sockaddr_in servaddr;
    struct sctp_event_subscribe evnts;
    int echo_to_all = 0;
    if(argc < 2){
        perror("Missing host argument\nusage >./client [host_name]"); 
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
