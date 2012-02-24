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
  int connSock, in, i, flags, ret;
  struct sockaddr_in servaddr;
  struct sctp_sndrcvinfo sndrcvinfo;
  struct sctp_event_subscribe events;
  char recv_buffer[MAX_BUFFER+1];
  size_t msg_cnt;

  /* Create an SCTP TCP-Style Socket */
  connSock = socket( AF_INET, SOCK_STREAM, IPPROTO_SCTP );

  /* Specify the peer endpoint to which we'll connect */
  bzero( (void *)&servaddr, sizeof(servaddr) );
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(MY_PORT_NUM);
  servaddr.sin_addr.s_addr = inet_addr( "127.0.0.1" );

  /* Connect to the server */
  connect( connSock, (struct sockaddr *)&servaddr, sizeof(servaddr) );
  printf("Client: SCTP association established with the server\n");

  /* Enable receipt of SCTP Snd/Rcv Data via sctp_recvmsg */
  memset( (void *)&events, 0, sizeof(events) );
  bzero(&events, sizeof(events));
  events.sctp_data_io_event = 1;
  setsockopt( connSock, IPPROTO_SCTP, SCTP_EVENTS,
               (const void *)&events, sizeof(events) );

  /* Expect two messages from the peer */
  for(msg_cnt = 0; msg_cnt < 2; ) {
    in = sctp_recvmsg( connSock, (void *)recv_buffer, sizeof(recv_buffer),
                        (struct sockaddr *)NULL, 0,
                        &sndrcvinfo, &flags );
    if (in < 0){
      if (errno != EWOULDBLOCK){
	printf("Client: Error detected while reading from socket\n");
	/* Close socket and exit */
	close(connSock);
	return -1;
      }
    }
    else if (in == 0){
      /* server closed association */
	printf("Client: Server closed association\n");
	/* Close socket and exit */
	close(connSock);
	return 0;
    }
    else {
      /* Print out received data */
    /* Null terminate the incoming string */
    msg_cnt++;
    recv_buffer[in] = 0;
    if  (sndrcvinfo.sinfo_stream == LOCALTIME_STREAM) {
      printf("Client: Received data fom Stream 0, (Local) %s\n", recv_buffer);
    } else if (sndrcvinfo.sinfo_stream == GMT_STREAM) {
      printf("Client: Received data fom Stream 1, (GMT  ) %s\n", recv_buffer);
    }
    }
  }
  printf("out of loop\n");

}

