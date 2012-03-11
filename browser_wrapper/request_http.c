
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#define HTTP_HEADER "GET http://www.example.com/index.html HTTP/1.1\n\n"
#define SS_LEN        7
#define PACKET_LINES  100
#define BUFFER_SIZE 200

int main(int argc, char **argv)
{
  int sock;
  struct sockaddr_in server_addr;
  char *server_ip;
  int port_num;
  char buffer[BUFFER_SIZE];
  char data[PACKET_LINES * BUFFER_SIZE];
  uint32_t search_count = 0;

  if(argc != 3) {
    fprintf(stderr,"usage: client <server ip address> <port>\n");
    exit(-1);
  }

  server_ip = argv[1];
  port_num  = atoi(argv[2]);

  if((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
    perror("Error: failed to create socket\n");
    exit(-2);
  }

  printf("Socket created\n");

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = inet_addr(server_ip);
  server_addr.sin_port = htons(port_num);

  if(connect(sock,(struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
    perror("Error: failed to connect to server");
    exit(-3);
  }

  printf("Connected to remote server\n");

  printf("Sending HTTP GET request\n");

  write(sock,HTTP_HEADER, sizeof(HTTP_HEADER));

  char *tok = NULL;
  int bufpos = 0;
  int bytesread = 0;

  bytesread = recv(sock,buffer,sizeof(buffer),0);
  if(bytesread) {
    printf("Data received from server\n");
    strncpy(&data[bufpos],buffer,bytesread);
    printf("%s\n", data);
  }

  close(sock);
  return 0;
}

