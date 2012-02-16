
/*
 * Erik Steggall
 * CMPS 156
 * 01/25/12
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

void dg_echo(int sockfd, struct sockaddr* pcliaddr, socklen_t cli_len){
    int n;
    socklen_t len;
    char buff[MAXSEND];
    char msg[MAXLINE];
    FILE* request_file;
    int num_servers;
    int offset;
    long actual_offset;
    long file_size;
    size_t bytes_to_read;
    size_t bytes_read;
    char* bufptr;
    char* savepos = NULL;

    n = 1024;

    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n));

    connect(sockfd, (struct sockaddr *) pcliaddr, cli_len);

    for(;;){
        
        len = cli_len;
        bzero(&msg, sizeof(msg));
        n = read(sockfd, msg, MAXLINE); //, 0, pcliaddr, &len);
        bufptr = msg;
        printf("msg = %s\n", msg);
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
        printf("offset = %d\nnum_servers = %d\nfile = %s\n", offset, num_servers, file_name);

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
        printf("Read in %s\n", buff);

        //write(sockfd, buff, bytes_read); //, 0, pcliaddr, len);
        savepos = NULL;

    }
//    sendto(sockfd, msg, n, 0, pcliaddr, len);

}
    


int main(int argc, char** argv){
    int sockfd;
    int port_num;
    struct sockaddr_in servaddr, cliaddr;

    if(NULL == argv[1]){
        perror("No port given\n");
        exit(1);
    }
    port_num = atoi(argv[1]);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port_num);

    bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    dg_echo(sockfd, (struct sockaddr *) &cliaddr, sizeof(cliaddr));

    return 0;
}

