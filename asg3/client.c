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

typedef struct server_info{
    int fd;
    char * port;
    char * host_name;
    struct server_info * next;
    struct server_info * prev;
}server_info;

void chomp (char* string, char delim){
    size_t len = strlen(string);
    if(len == 0) return;
    char *nlpos = string + len -1;
    if(*nlpos == delim) *nlpos = '\0';
}

int main(int argc, char** argv){
    int sockfd;
    int n;
    FILE* server_file;
    FILE* output_file;;
    char sendline[MAXLINE], recvline[MAXLINE+1];
    struct sockaddr_in servaddr;
    struct sockaddr* pservaddr;
    server_info* head;
    server_info* tail;
    server_info* temp;
    socklen_t servlen;
    char* send_buffer;
    fd_set rset;
    int num_servers;
    int server_counter;
    int port_num;
    int spin_cnt;
    int maxfd;


    if(argc != 3){
        perror("usage: >./client [request_file] [number of servers]\n");
        exit(1);
    }
    num_servers = atoi(argv[2]);
    if(32 < strlen(argv[1])){
        perror("request_file name too big\n");
        exit(1);
    }
    send_buffer = calloc(1, (strlen(argv[1])+32)); 

    strncpy(send_buffer, argv[1], (strlen(argv[1])));
    strcat(send_buffer, "\n");
    if(64 < strlen(argv[2])){
        perror("Number of servers is too high\n");
        exit(1);
    }
    strcat(send_buffer, argv[2]);
    strcat(send_buffer, "\n");

    server_file = fopen("server-info.txt", "r");
    if(server_file == NULL){
        perror("Could not open server info file\n");
        exit(1);
    }

    for(server_counter = 1;server_counter <= num_servers; server_counter++){
        char buffer[MAXLINE];
        char* fgets_rc = fgets(buffer, sizeof(buffer), server_file);
        if(fgets_rc == NULL){
            perror("Input not read\n");
            break;
        }
        chomp(buffer, '\n');
        char* savepos = NULL;
        char* bufptr = buffer;
        server_info * new_server;
        new_server = calloc(1, sizeof(server_info));
        if(1 == server_counter){
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
    }


    for(temp = head; temp != NULL; temp = temp->next){
        if(NULL == temp->host_name)break;
        if(NULL == temp->port)break;
        port_num = atoi(temp->port);

        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(port_num);
        inet_pton(AF_INET, temp->host_name, &servaddr.sin_addr);
        temp->fd = socket(AF_INET, SOCK_DGRAM, 0);
        pservaddr = (struct sockaddr*) &servaddr;
        servlen = sizeof(servaddr);
        connect(temp->fd, (struct sockaddr*)pservaddr, servlen);
        FD_SET(temp->fd, &rset);
    }

    output_file = fopen("output.txt", "wb");
    spin_cnt = 0;
    maxfd = tail->fd+1;
    for(temp = head; temp != NULL;){
        printf("num_servers = %d\nspin_cnt = %d\n", num_servers, spin_cnt);
        if((temp->port == NULL) || (temp->host_name == NULL)){
            printf("temp was null\n");
            temp = head;
        }
        strcat(send_buffer, "a");
        
        write(temp->fd, send_buffer, strlen(send_buffer)); //, 0, pservaddr, servlen);

        while(1){


            select(maxfd, &rset, &rset, NULL, NULL);
            if(FD_ISSET(temp->fd, &rset)){
                n = read(temp->fd, recvline, MAXLINE); //, 0, preply_addr, &len);
                recvline[n] = '\0';
                spin_cnt++;
                fwrite(recvline, 1, strlen(recvline)-1, output_file);
                break;
            }
        }
        if(temp == tail){
            temp = head;
        }
        else{
            temp = temp->next;
        }
        if(num_servers <= spin_cnt)break;
    }
    fclose(server_file);
    fclose(output_file);
    for(temp = head; temp != NULL; temp = temp->next){
        close(temp->fd);
        if(temp->port != NULL)free(temp->port);
        if(temp->host_name != NULL)free(temp->host_name);
        free(temp);
    }
    return 0;
}
