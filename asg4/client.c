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
#define MAX_BUFFER 1024
#define MY_PORT_NUM 1130 
#define CONTROL_STREAM 0
#define DATA_STREAM 1

int main(int argc, char* argv[])
{
  int connSock, in, i, flags, ret;
  struct sockaddr_in servaddr;
  struct sctp_sndrcvinfo sndrcvinfo;
  struct sctp_event_subscribe events;
  char recv_buffer[MAX_BUFFER+1];
  char send_buffer[] = "3 textfile.txt\n";
  size_t msg_cnt;
  int server_port;

  if(argc != 3){ fprintf(stderr,"Usage: > ./server [server-IP] [server-port]\n"); exit(1);}
  server_port = atoi(argv[2]);
  if(server_port < 0 || server_port > 65535){
      fprintf(stderr, "Port not in range [0-65535]\n");
      exit(0);
  }

  /* Create an SCTP TCP-Style Socket */
  connSock = socket( AF_INET, SOCK_STREAM, IPPROTO_SCTP );

  /* Specify the peer endpoint to which we'll connect */
  bzero( (void *)&servaddr, sizeof(servaddr) );
  servaddr.sin_family = AF_INET;
  servaddr.sin_port = htons(server_port);
  servaddr.sin_addr.s_addr = inet_addr( argv[1] );

  /* Connect to the server */
  connect( connSock, (struct sockaddr *)&servaddr, sizeof(servaddr) );
  printf("Client: SCTP association established with the server\n");

  /* Enable receipt of SCTP Snd/Rcv Data via sctp_recvmsg */
  memset( (void *)&events, 0, sizeof(events) );
  bzero(&events, sizeof(events));
  events.sctp_data_io_event = 1;
  setsockopt( connSock, IPPROTO_SCTP, SCTP_EVENTS,
               (const void *)&events, sizeof(events) );
  ret = sctp_sendmsg(connSock, (void *) send_buffer, (size_t) strlen(send_buffer), NULL, 0, 0, 0, CONTROL_STREAM, 0, 0);
  printf("out of loop\nret = %d\n", ret);

  /* Expect two messages from the peer */
  for(msg_cnt = 0; msg_cnt < 1; ) {
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
    if  (sndrcvinfo.sinfo_stream == CONTROL_STREAM) {
      printf("Client: Received data fom Stream 0, (Local) %s\n", recv_buffer);
    } else if (sndrcvinfo.sinfo_stream == DATA_STREAM) {
      printf("Client: Received data fom Stream 1, (GMT  ) %s\n", recv_buffer);
    }
    }
  }

}

