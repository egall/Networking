/*
 * Erik Steggall
 * CMPS 156
 * 03/03/12
 * http_proxy.c
*/

/*
** http_proxy.c -- proxy http server
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <syslog.h>

#define HTTP_HEADER "GET http://www.example.com/index.html HTTP/1.1\n\n"
#define SERV_PORT 2440 
#define REMOTE_PORT 1234 
#define LISTENQ 5
#define MAXLINE 4096

static void trim( char* line ){
    int l;

    l = strlen( line );
    while ( line[l-1] == '\n' || line[l-1] == '\r' )
	line[--l] = '\0';
}

void sig_child(int signo){
    pid_t pid;
    int stat;
    while( (pid = waitpid(-1, &stat, WNOHANG)) > 0){
         printf("child %d terminated\n", pid);
    }
    return;
}

char*
proxy_http(int remotefd, char* method, char* url, char* protocol){
    char send_buff[1000];
    char buffer[200];
    char* recv_data;
    strncpy(send_buff, method, strlen(method));
    strcat(send_buff, " ");
    strcat(send_buff, url);
    strcat(send_buff, " ");
    strcat(send_buff, protocol);
    strcat(send_buff, "\n\n");
    printf("Connected to remote server\n");
    write(remotefd, send_buff, sizeof(send_buff));
    printf("Sent header:\n%s\n", send_buff);
    char *tok = NULL;
    int bufpos = 0;
    int bytesread = 0;
    bytesread = recv(remotefd, buffer, sizeof(buffer), 0);
    if(bytesread){
        printf("Data received from server: \n");
        recv_data = calloc(1, bytesread+1);
        strncpy(&recv_data[bufpos], buffer, bytesread);
    }
    else{
        perror("Couldn't read bytes\n");
        exit(0);
    }
    return recv_data;
    
}



static int open_remote_sock(char* hostname, int port, char* method, char* url, char* protocol){
    int sock;
    struct sockaddr_in remote_addr;
    char* server_ip;
    int port_num;
    char buffer[200];
    char data[2000];
    char send_buff[1000];
    uint32_t search_count = 0;
    struct hostent *hent;
    char* remote_ip;
    server_ip = "192.0.43.10";
    port_num = 80;

    if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        perror("Error: failed to create socket\n");
        exit(-2);
    }
    printf("Socket created\n");

    hent = gethostbyname(hostname);
    remote_ip = inet_ntoa(*((struct in_addr*) hent->h_addr));
    
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(remote_ip);
    remote_addr.sin_port = htons(port);

    if(connect(sock, (struct sockaddr*) &remote_addr, sizeof(remote_addr)) < 0){
        perror("Error: failed to connect to server\n");
        exit(-3);
    }
    return sock;
}

char*
get_request(int sockfd){
    ssize_t n;
    int port;
    int numbytes;
    pid_t childpid;
    char line[1000], method[1000], url[1000], protocol[1000], host[1000], path[1000], buff[1000];
    char* recved_data;
    int iport;
    while((n = read(sockfd, line, MAXLINE)) > 0){
        if(n < 0) perror("read failed\n");
        trim(line);
        if(sscanf(line, "%[^ ] %[^ ] %[^ \n]", method, url, protocol) != 3)
            perror("Bad request: Can't parse request\n");
        if( url == (char*) 0)
            perror("Bad request: NULL url\n");
        if( strncasecmp(url, "http://", 7) == 0){
	    (void) strncpy( url, "http", 4 );	/* make sure it's lower case */
	    if ( sscanf( url, "http://%[^:/]:%d%s", host, &iport, path ) == 3 ){
                printf("Got host, path and port\n");
	        port = (unsigned short) iport;
            }
	    else if ( sscanf( url, "http://%[^/]%s", host, path ) == 2 ){
                printf("Got host path\n");
	        port = 80;
            }
	    else if ( sscanf( url, "http://%[^:/]:%d", host, &iport ) == 2 ){
                printf("Got host and port\n");
	        port = (unsigned short) iport;
	        *path = '\0';
	    }
            else if ( sscanf( url, "http://%[^/]", host ) == 1 ){
                printf("Got host\n");
                port = 80;
                *path = '\0';
            }
            else
                perror("Bad Request: Can't parse URL.\n" );
        }
        else{
            perror("Bad request: Unknown URL type.\n"); 
            continue;
        }
        if( (childpid = fork()) == 0){
            int remoteservefd = 0;
            remoteservefd = open_remote_sock(host, port, method, url, protocol);
            recved_data = proxy_http(remoteservefd, method, url, protocol);
//            printf("%s", recved_data);
            close(remoteservefd);
            return recved_data;
            exit(0);
        }


    }
    return NULL;
}

void str_echo(int sockfd){
    ssize_t n;
    char buf[MAXLINE];
    while((n = read(sockfd, buf, MAXLINE)) > 0){
        if(n < 0) perror("read failed\n");
        printf("buf = %s\n", buf);
    }
}

int main(int argc, char** argv){
    int listenfd, connfd;
    pid_t childpid;
    socklen_t browser_len;
    struct sockaddr_in browseraddr, servaddr;
    void sig_child(int);
    char* ret_data;
    int port_num;
/*
    if(argv[1] == NULL){
        printf("Proxy aborted: No port number was given\n");
        exit(0);
    }
*/
    port_num = atoi(argv[1]);

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(SERV_PORT);
    bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr));
    listen(listenfd, LISTENQ);
    for(;;){
        browser_len = sizeof(browseraddr);
        if( (connfd = accept(listenfd, (struct sockaddr*) &browseraddr, &browser_len)) < 0){
            if(errno == EINTR) continue;
            else perror("accept error");
            perror("accept error");
            exit(0);
        }
        if( (childpid = fork()) == 0){
            close(listenfd);

            ret_data = get_request(connfd);
            printf("Returned data: \n");
            printf("\n%s\n", ret_data);
            write(connfd, ret_data, sizeof(ret_data));
            exit(0);
      }
        close(connfd);
     }
    return 0;
}
                
