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


int main(int argc, char** argv){
    int sockfd;
    int n;
    socklen_t len, clilen;
    char mesg[MAXLINE];
    char buff[MAXSEND];
    struct sockaddr_in servaddr, cliaddr;
    struct sockaddr * pcliaddr;
    int port_num;
    FILE* request_file;
    int num_servers;
    int offset;
    long actual_offset;
    long file_size;
    size_t bytes_to_read;
    size_t bytes_read;
    char* bufptr;
    char* savepos = NULL;

    if(argc != 2){
        perror("usage: >./server [port_num] \n");
        exit(1);
    }

    port_num = atoi(argv[1]);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port_num);

    bind(sockfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
    pcliaddr = (struct sockaddr*) &cliaddr;
    clilen = sizeof(cliaddr);
    for(;;){
        bzero(&mesg, sizeof(mesg));
        len = clilen;
        n = recvfrom(sockfd, mesg, MAXLINE, 0, pcliaddr, &len);
        bufptr = mesg;
        char* file_name = strtok_r(bufptr," \n\t\0", &savepos);
        if(NULL == file_name){
            perror("No file name input\n");
            exit(1);
        }
        bufptr = NULL;

        char* num_servers_str = strtok_r(bufptr," \n\t\0", &savepos);
        if(NULL == num_servers_str){
            perror("No file name input\n");
            exit(1);
        }
        bufptr = NULL;

        char* offset_str = strtok_r(bufptr," \n\t\0", &savepos);
        if(NULL == offset_str){
            perror("No file name input\n");
            exit(1);
        }
        bufptr = NULL;



        offset = (strlen(offset_str)-1);
        num_servers = atoi(num_servers_str);
        offset = offset%num_servers;
        request_file = fopen(file_name, "r");


        fseek(request_file, 0, SEEK_END);
        file_size = ftell(request_file);
        bytes_to_read = (size_t) file_size/num_servers;
        actual_offset = (long) offset*bytes_to_read;

        fseek(request_file, actual_offset, SEEK_SET);

        bytes_read = fread(buff,1,bytes_to_read,request_file);
        if(bytes_read > bytes_to_read || bytes_read < 0){
            perror("Error reading file\n");
            close(sockfd);
            exit(1);
        }
        buff[bytes_read] = '\n';
        printf("buf = %s\n", buff);


        sendto(sockfd, buff, bytes_read, 0, pcliaddr, len);
    }
    fclose(request_file);

}
    
