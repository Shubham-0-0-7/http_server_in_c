#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

int main(void){
    int tcp_socket = socket(
        AF_INET, //ipv4
        SOCK_STREAM, //tcp
        0 // dc
    );
 
}