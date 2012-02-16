/*
 * Erik Steggall
 * asg2
 * myclient.c 
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>


#include <arpa/inet.h>

#define PORT "1234" // the port client will be connecting to 

#define MAXDATASIZE 512 // max number of bytes we can get at once 

#define LINESIZE 72

typedef struct server_info{
    int fd;
    char * port;
    char * host_name;
    struct server_info * next;
    struct server_info * prev;
}server_info;

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


void chomp (char* string, char delim){
    size_t len = strlen(string);
    if(len == 0) return;
    char *nlpos = string + len -1;
    if(*nlpos == delim) *nlpos = '\0';
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;  
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    FILE * input_file;
    FILE * output_file;
    char send_buffer[128];
    int server_count;
    int port;
    int num_servers;
    char file_name[64];
    int check;
    int itor;
    char file_offset[16];
    server_info * head;
    server_info * tail;
    server_info * temp;
    struct sockaddr_in serveaddr;
    fd_set rset;
    int maxfd;
    int spin_cnt;
    if (argc != 3) {
        fprintf(stderr,"usage: client server-info.txt #ofservers\n");
        exit(1);
    }
    num_servers = atoi(argv[2]);
    if(128 < strlen(argv[1])){
        fprintf(stderr, "file name too long\n");
        exit(1);
    }
    if(128 < strlen(argv[2])){
        fprintf(stderr, "number of servers too long\n");
        exit(1);
    }
    strcpy(file_name, argv[1]);
    strcpy(send_buffer, file_name);
    strcat(send_buffer, "\n");
    strcat(send_buffer, argv[2]);
    strcat(send_buffer, "\n");

    

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    input_file = fopen("server-info.txt", "r");
    if(input_file == NULL){
        fprintf(stderr, "Could not open %s\n", argv[1]);
        exit(0);
    }
    for(server_count = 1;server_count <= num_servers; server_count++){
        char buffer[LINESIZE];
        char* fgets_rc = fgets(buffer, sizeof(buffer), input_file);
        if(fgets_rc == NULL){
            fprintf(stderr, "Input not read\n");
            exit(0);
        }
        chomp(buffer, '\n');
        char* savepos = NULL;
        char* bufptr = buffer;
        server_info * new_server;
        new_server = calloc(1, sizeof(server_info));
        if(1 == server_count){
            tail = head = new_server;
            new_server->prev = NULL;
            new_server->next = NULL;
        }else{
            tail->next = new_server;
            new_server->prev = tail;
            new_server->next = NULL;
            tail = new_server;
        }
        char* host = strtok_r(bufptr," \t\n", &savepos);
        if(host == NULL) break;
        new_server->host_name = calloc(1, 64);
        strcpy(new_server->host_name, host);
        bufptr = NULL;

        char* port_str = strtok_r(bufptr," \t\n", &savepos);
        if(port_str == NULL) break;
        new_server->port = calloc(1, 8);
        strcpy(new_server->port, port_str);
        bufptr = NULL;
        if(server_count > num_servers) break;
    }
    itor = 0;
    for(temp = head; temp != NULL; temp = temp->next){
        itor++;
        temp->fd = itor;
        if((temp->fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
                perror("client: socket");
                continue;
        }
        bzero(&serveaddr, sizeof(serveaddr));
        serveaddr.sin_family = AF_INET;
        port = atoi(temp->port);
        serveaddr.sin_port = htons(port);
        inet_pton(AF_INET, temp->host_name, &serveaddr.sin_addr);
        if(connect(temp->fd, (struct sockaddr *) &serveaddr, sizeof(serveaddr)) == -1){
            close(temp->fd);
            perror("Client: socket");
            continue;
        }
        FD_SET(temp->fd, &rset);
    }
    output_file = fopen("output.txt", "wb");
    maxfd = tail->fd+1;
    spin_cnt = 0;
    for(;spin_cnt < num_servers;){
        select(maxfd, &rset, &rset, NULL, NULL);
        for(temp = head; temp != NULL; temp = temp->next){
            if(FD_ISSET(temp->fd, &rset)){
                strcat(send_buffer, "a");
                write(temp->fd, send_buffer, 64);
                if ((numbytes = recv(temp->fd, buf, MAXDATASIZE-1, 0)) == -1) {
                    fprintf(stderr,"nothing received\n");
                    perror("recv");
                    exit(1);
                }
                buf[numbytes] = '\0';
                spin_cnt++;
                fwrite(buf, 1, strlen(buf),  output_file);
            }
        }
    }
    fclose(output_file);
    fclose(input_file);
    for(temp = head; temp != NULL; temp = temp->next){
        free(temp->port);
        free(temp->host_name);
        free(temp);
    }


    return 0;
}
