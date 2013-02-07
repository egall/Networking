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
 
///   SERVER   
 
int main(int argc, char *argv[])
{
    //Declaring process variables.
    int server_sockfd, client_sockfd;
    int server_len ; 
    int rc ; 
    int ret_val;
    FILE *fp;
    unsigned client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    char buffer[64];
    char recv_buffer[64];
 
    //Remove any old socket and create an unnamed socket for the server.
    server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htons(INADDR_ANY);
    server_address.sin_port = htons(7734) ; 
    server_len = sizeof(server_address);
 
    rc = bind(server_sockfd, (struct sockaddr *) &server_address, server_len);
    printf("RC from bind = %d\n", rc ) ; 
     
    //Create a connection queue and wait for clients
    rc = listen(server_sockfd, 5);
    printf("RC from listen = %d\n", rc ) ; 
 
    client_len = sizeof(client_address);
    client_sockfd = accept(server_sockfd, (struct sockaddr *) &client_address, &client_len);
    printf("after accept()... client_sockfd = %d\n", client_sockfd) ; 
 
    while(1)
    {
        char ch[100];
        printf("server waiting\n");
 
        //Accept a connection
        //client_len = sizeof(client_address);
        //client_sockfd = accept(server_sockfd, (struct sockaddr *) &client_address, &client_len);
        //printf("after accept()... client_sockfd = %d\n", client_sockfd) ; 
        //Read write to client on client_sockfd
 
        rc = read(client_sockfd, &ch, 64);
        printf("This is the sentence recieved:\n %s\n", ch);
        ret_val = sprintf(recv_buffer, "echo \"%s\" | openssl enc -base64 -d", ch);
        fp = popen(recv_buffer, "r");

        if(fp == NULL){
            printf("Failed on popen()\n");
            exit(1);
        }

        while(fgets(buffer, sizeof(buffer)-1, fp) != NULL){
            printf("Data decoded:\n %s", buffer);
        } 

//        printf("RC from read = %d\n", rc ) ;        
//        if (ch=='X') break ; 
//        ch++;
        write(client_sockfd, &buffer, 16);
    }
 
    printf("server exiting\n");
 
    //close(client_sockfd);
    close(client_sockfd);
    return 0;
}
