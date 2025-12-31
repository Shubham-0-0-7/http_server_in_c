#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>


// const char* CRLF = "\r\n";
// const char* SP = " ";
// typedef struct{
//     char* method;
//     char* uri;
//     char* version;
// } http_req_line;

// typedef enum{
//     PARSE_OK,
//     PARSE_ERR,
// } http_res;

// typedef struct{
//     const char* start;
//     const char* end;
// } string_view;

// typedef struct{
//     string_view splits;
//     size_t cnt;
//     size_t capac;
// } string_splits;

// static string_splits split_string(const char* str, size_t len, char spilt_by){
//     string_splits res;
//     res.splits = calloc(sizeof(string_view), res.capac);
//     char* start = str;
//     char* end = NULL;
//     size_t res_i = 0;

//     for(size_t i=0; i<len; ++i){
//         if(str[i] == split_by){
//             res_i.splits[res_i].start = start;
//             res_i.splits[res_i].end = &str[i];
//         }

//     }
// }

// static void free_splits(string_splits* spl){
//     if(spl){
//         free(spl->splits);
//         spl->splits = NULL;
//     }
// }


// http_req_line http_req_line_init(){
//     http_req_line line;
//     line.method = NULL;
//     line.uri = NULL;
//     line.version = NULL;
//     return line;
// }


// http_res parse_req_line(const char* buff, size_t len, http_req_line* req_line){
//     if(!buff || !req_line){
//         return PARSE_ERR;
//     }
//     req_line->method = "GET"; 
//     req_line->version = "HTTP/1.0"; 
//     return PARSE_OK;
// }

int handle_client(int client_socket){
    ssize_t n;
    char buff[1024];
    const char* resp = 
        "HTTP/1.0 200 OK\r\n"
        "Content-Length: 18\r\n"
        "\r\n"
        "<h1>Hello World!</h1>";

    n = read(client_socket, buff, sizeof(buff)-1);
    if(n < 0){
        perror("read(client)");
        return -1;
    }
    if(n == 0){
        printf("connection closed gracefully!\n");
        return 0;
    }
    buff[n] = '\0';
    printf("\n----\n");
    printf("REQUEST:\n%s", buff);
    printf("\n----\n");
    write(client_socket, resp, strlen(resp));
    close(client_socket);
    return 0;
}

int main(void){
    //declare 
    int rc = 0;
    struct sockaddr_in bind_addr; 
    int tcp_socket = 0;
    int ret = 0;
    int client_socket = 0;
    bool enabled = true;

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
    
    (void)setsockopt(tcp_socket, 
        SOL_SOCKET, 
        SO_REUSEADDR, 
        &enabled, 
        sizeof(enabled)
    );

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
    for( ;; ){
        printf("waiting for connections...\n");
        client_socket = accept(tcp_socket, NULL, NULL);
        printf("got a connection\n");
        rc = handle_client(client_socket);
        close(client_socket);
    }
exit:
    close(tcp_socket);
    return ret;
}
 