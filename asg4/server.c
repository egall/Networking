#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

#define MAX_BUFFER 200
#define MY_PORT_NUM 1234
#define LOCALTIME_STREAM 0
#define GMT_STREAM 1


int main()
{
  int listenSock, connSock, ret;
  struct sockaddr_in servaddr;
  char buffer[MAX_BUFFER+1];
  time_t currentTime;

  /* Create SCTP TCP-Style Socket */
  listenSock = socket( AF_INET, SOCK_STREAM, IPPROTO_SCTP );

  /* Accept connections from any interface */
  bzero( (void *)&servaddr, sizeof(servaddr) );
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl( INADDR_ANY );
  servaddr.sin_port = htons(MY_PORT_NUM);

  /* Bind to the wildcard address (all) and MY_PORT_NUM */
  ret = bind( listenSock,
               (struct sockaddr *)&servaddr, sizeof(servaddr) );

  /* Place the server socket into the listening state */
  listen( listenSock, 5 );

  /* Server loop... */
  while( 1 ) {
    
    printf("Server: Listening for new association...\n");

    /* Await a new client connection */
    connSock = accept( listenSock,
                        (struct sockaddr *)NULL, (int *)NULL );

    /* New client socket has connected */

    printf("Server: Client socket connected\n");

    /* Get the current time */
    currentTime = time(NULL);

    /* Send local time on stream 0 (local time stream) */
    printf("Server: Sending local time on Stream 0\n");
    snprintf( buffer, MAX_BUFFER, "%s\n", ctime(&currentTime) );
    ret = sctp_sendmsg( connSock,
                          (void *)buffer, (size_t)strlen(buffer),
                          NULL, 0, 0, 0, LOCALTIME_STREAM, 0, 0 );

    /* Send GMT on stream 1 (GMT stream) */
    printf("Server: Sending local time on Stream 1\n");
    snprintf( buffer, MAX_BUFFER, "%s\n",
               asctime( gmtime( &currentTime ) ) );

    ret = sctp_sendmsg( connSock,
                          (void *)buffer, (size_t)strlen(buffer),
                          NULL, 0, 0, 0, GMT_STREAM, 0, 0 );

    /* wait for a while */
    sleep(1); 

    /* Close the client connection */
    printf("Server: Closing association\n");
    close( connSock );

  }

  return 0;
}


