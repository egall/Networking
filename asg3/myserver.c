/*
 * Erik Steggall
 * CMPS 156
 * 01/25/12
 * Assignement #2
*/

/*
** server.c -- a stream socket server demo
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

#define PORT "3490"  // the port users will be connecting to

#define MAXLINE 512 

#define BACKLOG 10     // how many pending connections queue will hold

void sigchld_handler(int s)
{
    while(waitpid(-1, NULL, WNOHANG) > 0);
}

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

int main(int argc, char **argv){
    int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // connector's address information
    struct sockaddr_in servaddr, cliaddr;
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    time_t ticks;
    char buff[MAXLINE];
    int port_num;
    FILE* request_file;
    char* file_name;
    char input_buffer[128];
    char file_offset[16];
    int file_off;
    int check;
    char* serve_num_str;
    char* offset_str;
    int num_servers;
    long actual_offset;
    size_t bytes_to_read;
    int dis_offset;
    long file_size;
    int buff_size;
    socklen_t len, clilen;

    buff_size = 220*1024;
    

    if(argv[1] == NULL){
        printf("Server aborted: No port number was given\n");
        exit(0);
    }
    port_num = atoi(argv[1]);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port_num);
    bind(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));



    /*
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, argv[1], &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, SOCK_DGRAM,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &buff_size,
                sizeof(buff_size)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        return 2;
    }

    freeaddrinfo(servinfo); // all done with this structure

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    sa.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }
    */

    printf("server: waiting for connections...\n");
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &buff_size, sizeof(buff_size));

    for(;;){
        len = sizeof(cliaddr);
        recvfrom(sockfd, input_buffer, 64, cliaddr, &len);
    }

//    while(1) {  // main accept() loop
        sin_size = sizeof their_addr;
        /*
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }

        inet_ntop(their_addr.ss_family,
            get_in_addr((struct sockaddr *)&their_addr),
            s, sizeof s);
        printf("server: got connection from %s\n", s);
        */

//        if(check < 0) fprintf(stderr, "Read failed on file name\n");
        char* savepos = NULL;
        char* bufptr = input_buffer;

        file_name = calloc(1, 32);
        char* file_name_buf = strtok_r(bufptr," \t\n\0", &savepos);
        strcpy(file_name, file_name_buf);
        if(NULL == file_name){
            perror("No file name input\n");
            exit(1);
        }
        bufptr = NULL;


        serve_num_str = calloc(1, 4);
        char* serve_num_buf = strtok_r(bufptr," \t\n\0", &savepos);
        strcpy(serve_num_str, serve_num_buf);
        if(NULL == serve_num_str){
            perror("No file name input\n");
            exit(1);
        }
        bufptr = NULL;


        offset_str = calloc(1, 32);
        char* offset_buf = strtok_r(bufptr," \t\n\0", &savepos);
        strcpy(offset_str, offset_buf);
        if(NULL == offset_str){
            perror("No file name input\n");
            exit(1);
        }
        bufptr = NULL;

        num_servers = atoi(serve_num_str);
        dis_offset = strlen(offset_str);
        dis_offset--;



        /*
         * Open the file for reading, error and exit if it doesn't open
         */
        request_file = fopen(file_name, "r");
        
        if(request_file == NULL){
            perror ("Error opening file\n");
            printf ("Error opening file\n");
            close(new_fd);
            exit(0);
        }
        fseek(request_file, 0, SEEK_END);
        file_size = ftell(request_file);
        bytes_to_read = (size_t) file_size/num_servers;
        actual_offset = (long) dis_offset*bytes_to_read;
        fseek(request_file, actual_offset, SEEK_SET);
        /*
         * Get data from file, put data in buff
         */
        if ( fread(buff, 1, bytes_to_read, request_file) < 0){
            perror ("Error reading file\n");
            printf ("Error reading file\n");
            close(new_fd);
            exit(0);
        }

        if (!fork()) { // this is the child process
            close(sockfd); // child doesn't need the listener
            if (send(new_fd, buff, 512, 0) == -1)
                perror("send");
            close(new_fd);
            exit(0);
        }
        close(new_fd);  // parent doesn't need this
//    }

    free(file_name);
    free(serve_num_str);
    free(offset_str);
    fclose(request_file);


    return 0;
}
