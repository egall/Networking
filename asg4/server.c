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
#define MY_PORT_NUM 2448
#define LOCALTIME_STREAM 0
#define GMT_STREAM 1


int main()
{
  int listenSock, connSock, ret, in, flags;
  struct sctp_sndrcvinfo sndrcvinfo;
  struct sockaddr_in servaddr;
  char send_buffer[MAX_BUFFER+1];
  char recv_buffer[MAX_BUFFER+1];
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
    snprintf( send_buffer, MAX_BUFFER, "%s\n", "hello" );
    ret = sctp_sendmsg( connSock,
                          (void *)send_buffer, (size_t)strlen(send_buffer),
                          NULL, 0, 0, 0, LOCALTIME_STREAM, 0, 0 );

    /* Send GMT on stream 1 (GMT stream) */
    printf("Server: Sending local time on Stream 1\n");
    snprintf( send_buffer, MAX_BUFFER, "%s\n",
               "world!" );

    ret = sctp_sendmsg( connSock,
                          (void *)send_buffer, (size_t)strlen(send_buffer),
                          NULL, 0, 0, 0, GMT_STREAM, 0, 0 );

    in = sctp_recvmsg(connSock, (void *) recv_buffer, sizeof(recv_buffer), (struct sockaddr *) NULL, 0, &sndrcvinfo, &flags);
    printf("recv buffer = %s\n", recv_buffer);
    /* wait for a while */
    sleep(1); 

    /* Close the client connection */
    printf("Server: Closing association\n");
    close( connSock );

  }

  return 0;
}


