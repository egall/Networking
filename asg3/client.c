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

#define MAXLINE 128
#define SERV_PORT 1234

typedef struct server_info{
    int fd;
    char * port;
    char * host_name;
    struct server_info * next;
    struct server_info * prev;
}server_info;


void dg_cli(FILE* fp, const struct sockaddr* pservaddr, socklen_t servlen, fd_set rset, char* send_buffer,
            server_info* head, int num_servers){
    int n;
    char sendline[MAXLINE], recvline[MAXLINE+1];
    server_info* temp;
    int spin_cnt;
    temp = head;
    int maxfd = temp->fd+1;
    
    n = 1024;
    spin_cnt = 0;
    for(;;){
        select(maxfd, &rset, &rset, NULL, NULL);
        for(temp = head; temp != NULL; temp = temp->next){
            if(num_servers <= spin_cnt)break;
            if(NULL == temp->port) break;
            if(NULL == temp->host_name) break;
            setsockopt(temp->fd, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n));
            connect(temp->fd, (struct sockaddr *) pservaddr, servlen);
            strcat(send_buffer, "a");
//            printf("\nsend_buffer = %s\n", send_buffer);
            printf("temp->port = %s\n", temp->port);
            printf("temp->fd = %d\n", temp->fd);
            write(temp->fd, send_buffer, strlen(send_buffer)); //, 0, pservaddr, servlen);
            /*
            n = read(temp->fd, recvline, MAXLINE); //, 0, NULL, NULL);
            recvline[n] = '\0';
            printf("recvline = %s\n", recvline);
            */
            spin_cnt++;
        }
        if(num_servers <= spin_cnt)break;
    }
    /*
    */

}



void chomp (char* string, char delim){
    size_t len = strlen(string);
    if(len == 0) return;
    char *nlpos = string + len -1;
    if(*nlpos == delim) *nlpos = '\0';
}

int main(int argc, char** argv){
    struct sockaddr_in servaddr;
    FILE* input_file;
    server_info* head;
    server_info* tail;
    server_info* temp;
    char* send_buffer;
    fd_set rset;
    int num_servers;
    int server_count;
    int port_num;
    int itor = 0;
    


    if(argc != 3){
        perror("usage:>./client [file_name] [number of servers]\n");
        exit(1);
    }

    num_servers = atoi(argv[2]);

    send_buffer = calloc(1, (strlen(argv[1]+32)));

    strncpy(send_buffer, argv[1], (strlen(argv[1])));
    strcat(send_buffer, "\n");
    if(64 < strlen(argv[2])){
        perror("Number of servers is too high\n");
        exit(1);
    }
    strcat(send_buffer, argv[2]);
    strcat(send_buffer, "\n");
    



    input_file = fopen("server-info.txt", "r");
    if(input_file == NULL){
        perror("Could not open file\n");
        exit(1);
    }
    for(server_count = 1;server_count <= num_servers; server_count++){
        char buffer[MAXLINE];
        char* fgets_rc = fgets(buffer, sizeof(buffer), input_file);
        if(fgets_rc == NULL){
            perror("Input not read\n");
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
//        if(server_count > num_servers) break;
    }
//    if(server_count > num_servers)num_servers = server_count;

    for(temp = head; temp != NULL; temp = temp->next){
        itor++;
        temp->fd = itor;
        if(NULL == temp->port) break;
        if(NULL == temp->host_name) break;
        bzero(&servaddr, sizeof(servaddr));
        servaddr.sin_family = AF_INET;
        port_num = atoi(temp->port);
        servaddr.sin_port = htons(port_num);
        inet_pton(AF_INET, temp->host_name, &servaddr.sin_addr);
        temp->fd = socket(AF_INET, SOCK_DGRAM, 0);


        FD_SET(temp->fd, &rset);


    }

    dg_cli(input_file, (struct sockaddr *) &servaddr, sizeof(servaddr), rset, send_buffer, head, num_servers);

    

    fclose(input_file);
    for(temp = head; temp != NULL; temp = temp->next){
        free(temp->port);
        free(temp->host_name);
        free(temp);
    }

    return 0;
}
