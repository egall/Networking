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

static void
proxy_http( char* method, char* path, char* protocol, FILE* sockrfp, FILE* sockwfp )
    {
    printf("##### proxy_http ####\n");
    printf("method = %s\n", method);
    printf("path = %s\n", path);
    printf("protocol = %s\n", protocol);
    char line[10000], protocol2[10000], comment[10000];
    int first_line, status, ich;
    long content_length, i;

    /* Send request. */
    (void) fprintf( sockwfp, "%s %s %s\r\n", method, path, protocol );
    content_length = -1;
    first_line = 1;
    status = -1;
    while ( fgets( line, sizeof(line), sockrfp ) != (char*) 0 ){
	if ( strcmp( line, "\n" ) == 0 || strcmp( line, "\r\n" ) == 0 )
	    break;
	(void) fputs( line, stdout );
//	(void) alarm( TIMEOUT );
	trim( line );
	if ( first_line )
	    {
	    (void) sscanf( line, "%[^ ] %d %s", protocol2, &status, comment );
	    first_line = 0;
	    }
	if ( strncasecmp( line, "Content-Length:", 15 ) == 0 )
	    content_length = atol( &(line[15]) );
    }
    /* Add a response header. */
    (void) fputs( "Connection: close\r\n", stdout );
    (void) fputs( line, stdout );
    (void) fflush( stdout );
    /* Under certain circumstances we don't look for the contents, even
    ** if there was a Content-Length.
    */
    if ( strcasecmp( method, "HEAD" ) != 0 && status != 304 )
	{
	/* Forward the content too, either counted or until EOF. */
	for ( i = 0;
	      ( content_length == -1 || i < content_length ) && ( ich = getc( sockrfp ) ) != EOF;
	      ++i )
	    {
	    putchar( ich );
	    if ( i % 10000 == 0 )
                printf("IDK\n");
//		(void) alarm( TIMEOUT );
	    }
	}
    (void) fflush( stdout );
    }




/*
static int open_remote_sock(char* hostname, int port, char* forward){
    socklen_t remote_len;
    struct sockaddr_in remoteaddr;
    char buff[1000];
    int numbytes;
    int ptonret;
    remote_len = sizeof(remoteaddr);
    struct hostent *he;
    int sock_family, sock_type, sock_protocol;
    int sockfd;
    printf("opening remote server socket...\n");

    he = gethostbyname("localhost");
//    he = gethostbyname("www.example.com");

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) perror("Internal Error: Couldn't create socket.\n");

    printf("port = %d\nhostname = %s\n", port, hostname);
    memset(&remoteaddr, 0, sizeof(remoteaddr));
    remoteaddr.sin_family = AF_INET;
    remoteaddr.sin_port = htons(REMOTE_PORT);
    remoteaddr.sin_addr.s_addr = inet_addr("192.0.43.10");
//    remoteaddr.sin_port = htons(port);
//    ptonret = inet_pton(AF_INET, he->h_addr, &remoteaddr.sin_addr);
    if(ptonret < 0)
      perror("Can't set inet_pton\n");
    
    if( connect(sockfd, (struct sockaddr*) &remoteaddr, remote_len) < 0)
        perror("Service unavailable: connection refused\n");
     
 
    return sockfd;
} 
*/

static int open_remote_sock(char* hostname, int port, char* forward){
    int sock;
    struct sockaddr_in remote_addr;
    char* server_ip;
    int port_num;
    char buffer[200];
    char data[2000];
    uint32_t search_count = 0;
    server_ip = "192.0.43.10";
    port_num = 80;

    if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0){
        perror("Error: failed to create socket\n");
        exit(-2);
    }
    printf("Socket created\n");
    
    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(server_ip);
    remote_addr.sin_port = htons(port);

    if(connect(sock, (struct sockaddr*) &remote_addr, sizeof(remote_addr)) < 0){
        perror("Error: failed to connect to server\n");
        exit(-3);
    }
    printf("Connected to remote server\n");
    write(sock, HTTP_HEADER, sizeof(HTTP_HEADER));
    printf("Sent header\n");
    char *tok = NULL;
    int bufpos = 0;
    int bytesread = 0;
    bytesread = recv(sock, buffer, sizeof(buffer), 0);
    if(bytesread){
        printf("Data received from server\n");
        strncpy(&data[bufpos], buffer, bytesread);
        printf("%s\n", data);
    }
    else{
        printf("couldn't read bytes\n");
    }
    return 0;
}

void get_request(int sockfd){
    ssize_t n;
    int port;
    int numbytes;
    pid_t childpid;
    char line[1000], method[1000], url[1000], protocol[1000], host[1000], path[1000], buff[1000], forward[1000];
    int iport;
    while((n = read(sockfd, line, MAXLINE)) > 0){
        if(n < 0) perror("read failed\n");
        strcpy(forward, line);
        trim(line);
        if(sscanf(line, "%[^ ] %[^ ] %[^ ]", method, url, protocol) != 3)
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
//            FILE* remoterfp;
//            FILE* remotewfp;

            remoteservefd = open_remote_sock(host, port, forward);
//            remoterfp = fdopen(remoteservefd, "r");
//            remotewfp = fdopen(remoteservefd, "w");
//            numbytes = send(remoteservefd, forward, sizeof(forward), 0); 
/*
            numbytes = send(remoteservefd, forward, strlen(forward), 0);
            printf("numbytes = %d\n", numbytes);
            if(numbytes < 0){
               perror("Didn't send bytes\n");
               exit(0);
            }
            numbytes = recv(remoteservefd, buff, sizeof(buff), 0);
            printf("numbytes = %d\n", numbytes);
            printf("Data = %s\n", buff);
            if(numbytes < 0){
            printf("##############################################\n");
               perror("Didn't receive bytes\n");
               exit(0);
            }
*/
//            proxy_http(method, path, protocol, remoterfp, remotewfp);
            printf("remote server fd = %d\n", remoteservefd);
            close(remoteservefd);
            exit(0);
        }


    }
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

            get_request(connfd);
            exit(0);
      }
        close(connfd);
     }
    return 0;
}
                
