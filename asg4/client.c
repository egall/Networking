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
  int connSock, in, i, ret;
  FILE* send_file;
  FILE* output_file;
  struct sockaddr_in servaddr;
  struct sctp_sndrcvinfo sndrcvinfo;
  struct sctp_event_subscribe events;
  char recv_buffer[MAX_BUFFER+1];
  char send_buffer[MAX_BUFFER+1];
  char file_name[43];
  size_t msg_cnt;
//  size_t name_len;
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

  /* Expect two messages from the peer */
  for(msg_cnt = 0; msg_cnt < 100; ) {
      bzero(send_buffer, sizeof(send_buffer));
      bzero(file_name, sizeof(file_name));
      bzero(recv_buffer, sizeof(recv_buffer));
      printf("input command:\ndir\nget [file_name]\nput [file_name]\nabort\nquit\n");
      scanf("%s", send_buffer);
      if('d' == send_buffer[0] && 'i' == send_buffer[1] && 'r' == send_buffer[2]){
          printf("send buffer = %s\n", send_buffer);
          ret = sctp_sendmsg(connSock, (void *) send_buffer, (size_t) strlen(send_buffer),
                              NULL, 0, 0, 0, CONTROL_STREAM, 0, 0);
          if(ret < 0){ perror("Didn't receive message\n"); exit(1);}
      }
      else if('g' == send_buffer[0] && 'e' == send_buffer[1] && 't' == send_buffer[2]){
          scanf("%s", file_name);
          printf("File name = %s\n", file_name);
//          name_len =  strlen(file_name);
//          file_name[name_len] = ' ';
          strcat(send_buffer, file_name);
          printf("send buffer = %s\n", send_buffer);
          ret = sctp_sendmsg(connSock, (void *) send_buffer, (size_t) strlen(send_buffer),
                               NULL, 0, 0, 0, CONTROL_STREAM, 0, 0);
          if(ret < 0){ perror("Didn't receive message\n"); exit(1);}
      }
      else if('p' == send_buffer[0] && 'u' == send_buffer[1] && 't' == send_buffer[2]){
          size_t sfile_size;
          size_t bytes_read;
          char file_buffer[1024];
          bzero(file_buffer, sizeof(file_buffer));
          scanf("%s", file_name);
          printf("File name = %s\n", file_name);
          send_file = fopen(file_name, "r");
          if(NULL == send_file){ perror("Couldn't open file\n"); exit(1);}
//          bzero(send_buffer, sizeof(send_buffer));
          fseek(send_file, 0, SEEK_END);
          sfile_size = (size_t) ftell(send_file);
          rewind(send_file);
          printf("file size = %d\n", (int) sfile_size);
          bytes_read = fread(file_buffer, 1, sfile_size, send_file);
          if(bytes_read != sfile_size){ perror("File not read correctly\n"); exit(1);}
          printf("file_buffer = %s\n", file_buffer);
          strcat(send_buffer, file_buffer);

          printf("send buffer = %s\n", send_buffer);
          ret = sctp_sendmsg(connSock, (void *) send_buffer, (size_t) strlen(send_buffer),
                               NULL, 0, 0, 0, CONTROL_STREAM, 0, 0);
          if(ret < 0){ perror("Didn't receive message\n"); exit(1);}
          exit(0);
      }
      else if('a' == send_buffer[0] && 'b' == send_buffer[1] && 'o' == send_buffer[2] && 'r' == send_buffer[3]){
          printf("abort\n");
          ret = sctp_sendmsg(connSock, (void*) send_buffer, (size_t) strlen(send_buffer), NULL, 0, 0, 0, CONTROL_STREAM, 0, 0);
          if(ret < 0){ perror("Didn't receive message\n"); exit(1);}
          exit(0);
      }
      else if('q' == send_buffer[0] && 'u' == send_buffer[1] && 'i' == send_buffer[2] && 't' == send_buffer[3]){
          printf("quit send buffer = %s\n", send_buffer);
          ret = sctp_sendmsg(connSock, (void*) send_buffer, (size_t) strlen(send_buffer), NULL, 0, 0, 0, CONTROL_STREAM, 0, 0);
	  if(ret < 0){ perror("Didn't receive message\n"); exit(1);}
          shutdown(connSock, SHUT_WR);
          close(connSock);
          exit(0);
      }
      else{
          printf("Not a valid option\n");
          exit(1);
      }
    in = sctp_recvmsg( connSock, (void *)recv_buffer, sizeof(recv_buffer),
                        (struct sockaddr *)NULL, 0,
                        &sndrcvinfo, 0);
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
    if('g' == send_buffer[0] && 'e' == send_buffer[1] && 't' == send_buffer[2]){
        output_file = fopen(file_name, "wb");
        fwrite(recv_buffer, 1, strlen(recv_buffer)-1, output_file);
    }
    if  (sndrcvinfo.sinfo_stream == CONTROL_STREAM) {
      printf("Client: Received data fom Stream 0, (Local) %s\n", recv_buffer);
    } else if (sndrcvinfo.sinfo_stream == DATA_STREAM) {
      printf("Client: Received data fom Stream 1, (GMT  ) %s\n", recv_buffer);
    }
    }
  }

}

