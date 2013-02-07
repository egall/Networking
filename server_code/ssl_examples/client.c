#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
 
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
 
 
///   CLIENT   
 
 
int main(int argc, char *argv[])
{
    printf("This is the client program\n");
 
    int sockfd;
    int len, rc ;
    struct sockaddr_in address;
    int result;
    FILE *fp;
    char buffer[64];
    char send_buffer[64];
    int done = 0;
    int ret_val = 0;
    char ch[] = "tits are tits";
 
   //Create socket for client.
    sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) { 
        perror("Socket create failed.\n") ; 
        return -1 ; 
    } 
     
    //Name the socket as agreed with server.
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(7734);
    len = sizeof(address);
 
    result = connect(sockfd, (struct sockaddr *)&address, len);
    if(result == -1)
    {
        perror("Error has occurred");
        exit(-1);
    }
 
    ret_val = sprintf(buffer, "echo \"%s\" | openssl enc -base64", ch);

    fp = popen(buffer, "r");

    if(fp == NULL){
        printf("Failed on popen()\n");
        exit(1);
    }
 
    while ( (fgets(send_buffer, sizeof(send_buffer)-1, fp)) != NULL ) {
 
        //Read and write via sockfd
        rc = write(sockfd, &send_buffer, 64);
//        printf("write rc = &#37;d\n", rc ) ; 
        if (rc == -1) break ; 
         
        read(sockfd, &send_buffer, 64);
        printf("Char from server = %s\n", ch);
        //if (ch == 'A') sleep(5) ;  // pause 5 seconds 
        done = 2;
        break;
    } 
    close(sockfd);
 
    exit(0);
}
