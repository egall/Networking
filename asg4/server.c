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

#define MAX_BUFFER 400
#define CONTROL_STREAM 0
#define DATA_STREAM 1


int main(int argc, char* argv[])
{
  int listenSock, connSock, ret, in;// flags, blah;
//  struct sctp_sndrcvinfo sndrcvinfo;
  struct sockaddr_in servaddr;
  char send_buffer[MAX_BUFFER+1];
  char recv_buffer[MAX_BUFFER+1];
  time_t currentTime;
  int server_port;

  if(argc != 2){ fprintf(stderr,"Usage: > ./server [server-port]\n"); exit(1);}
  server_port = atoi(argv[1]);
  if(server_port < 0 || server_port > 65535){
      fprintf(stderr, "Port not in range [0-65535]\n");
      exit(0);
  }
  
  /* Create SCTP TCP-Style Socket */
  listenSock = socket( AF_INET, SOCK_STREAM, IPPROTO_SCTP );

  /* Accept connections from any interface */
  bzero( (void *)&servaddr, sizeof(servaddr) );
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl( INADDR_ANY );
  servaddr.sin_port = htons(server_port);

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

    in = sctp_recvmsg(connSock, (void *) recv_buffer, sizeof(recv_buffer), (struct sockaddr *) NULL, 0, NULL, 0);
    if(in < 0)perror("Receive failed");
    printf("in = %d\n", (int) in);
    printf("recv buffer = %s\n", recv_buffer);
    if('d' == recv_buffer[0] && 'i' == recv_buffer[1] && 'r' == recv_buffer[2]){
        printf("Do ls -l\n");
        FILE* fpipe;
        char* command = "ls -l";
        char ls_line[256];
        char ls_buff[1024];
        bzero(ls_buff, sizeof(ls_buff));
        if( !(fpipe = (FILE*) popen(command, "r"))){ perror("Problem with pipe"); exit(1);}
        while(fgets(ls_line, sizeof(ls_line), fpipe)){
            strcat(ls_buff, ls_line);
        }
        printf("\n> ls -l:\n%s\n", ls_buff);
        pclose(fpipe);
        ret = sctp_sendmsg(connSock, (void*) ls_buff, (size_t) strlen(ls_buff), NULL,
                           0, 0, 0, DATA_STREAM, 0, 0);
    }
    else if('g' == recv_buffer[0] && 'e' == recv_buffer[1] && 't' == recv_buffer[2]){
        printf("Get filename\n");
        char send_file[256];
        char* bufptr;
        FILE* request_file;
        char* savepos = NULL;
        size_t cfile_size;
        size_t bytes_read;
        bufptr = recv_buffer;
        char* getfile = strtok_r(bufptr, " \n\t\0", &savepos);
        if(NULL == getfile){
            perror("No input\n");
            exit(1);
        }
        bufptr = NULL;
/*        if('2' != getfile[0]) continue;

        char* file_name = strtok_r(bufptr," \n\t\0", &savepos);
        if(NULL == file_name){
            perror("No file name input\n");
            exit(1);
        }
        bufptr = NULL;
        printf("file name = %s\n", file_name);
*/
        request_file = fopen("textfile.txt", "r");
        if(NULL == request_file){ perror("File not opened\n"); exit(1);}
        fseek(request_file, 0, SEEK_END);
        cfile_size = (size_t) ftell(request_file);
        rewind(request_file);
        bytes_read = fread(send_file, 1, cfile_size, request_file);
        printf("buff = %s\n", send_file);
        ret = sctp_sendmsg(connSock, (void*) send_file, (size_t) strlen(send_file), NULL,
                           0, 0, 0, DATA_STREAM, 0, 0);
        fclose(request_file);
        
        

    }
    else if('p' == recv_buffer[0]){
        printf("Put filename\n");
        char recv_file[2048];
        char* bufptr;
        FILE* output_file;
        char* savepos = NULL;
        output_file = fopen("output.txt", "wb");
        if(NULL == output_file){ perror("Couldn't open output.txt\n"); exit(1);}
        fwrite(recv_buffer, 1, strlen(recv_buffer)-1, output_file);
        fclose(output_file);
        
    }
    else if('4' == recv_buffer[0]){
        printf("Abort \n");
    }
    else if('5' == recv_buffer[0]){
        printf("quit\n");
    }else{
        printf("Not a valid option\n");
    }
    printf("recv buffer = %s\n", recv_buffer);

    /* Get the current time */
    currentTime = time(NULL);

    /* Send local time on stream 0 (local time stream) */
    printf("Server: Sending local time on Stream 0\n");
    snprintf( send_buffer, MAX_BUFFER, "%s\n", "hello" );
    ret = sctp_sendmsg( connSock,
                          (void *)send_buffer, (size_t)strlen(send_buffer),
                          NULL, 0, 0, 0, CONTROL_STREAM, 0, 0 );

    /* Send GMT on stream 1 (GMT stream) 
    printf("Server: Sending local time on Stream 1\n");
    snprintf( send_buffer, MAX_BUFFER, "%s\n",
               "world!" );

    ret = sctp_sendmsg( connSock,
                          (void *)send_buffer, (size_t)strlen(send_buffer),
                          NULL, 0, 0, 0, DATA_STREAM, 0, 0 );
    */

    /* wait for a while */
    sleep(1); 

    /* Close the client connection */
    printf("Server: Closing association\n");
    close( connSock );

  }

  return 0;
}


