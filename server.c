#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

int main(void){
    //declare 
    int rc = 0;
    struct sockaddr_in bind_addr; 
    int tcp_socket = 0;
    int ret = 0;
    int client_socket = 0;

    tcp_socket = socket(
        AF_INET, //ipv4
        SOCK_STREAM, //tcp
        0 // dont care
    );

    //initialize
    memset(&bind_addr, 0, sizeof(bind_addr));

    if(tcp_socket == -1){
        perror("socket()");
        return 1;
    }
    printf("socket creation succeeded\n");
    
    
    bind_addr.sin_port = htons(1337);
    bind_addr.sin_family = AF_INET;
    bind_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    rc = bind(
        tcp_socket, 
        (const struct sockaddr*)&bind_addr, 
        sizeof(bind_addr)
    );

    if(rc < 0){
        perror("bind()");
        ret = 1;
        goto exit;
    }
    printf("bind succeeded\n");

    rc = listen(tcp_socket, SOMAXCONN);
    if(rc < 0){
        perror("listen()");
        ret = 1;
        goto exit;
    }
    printf("listen succeeded\n");
    printf("waiting for connections...\n");
    client_socket = accept(tcp_socket, NULL, NULL);
    printf("got a connection");
   
exit:
    close(tcp_socket);
    return ret;
}
