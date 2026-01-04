#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#define CRLF "\r\n"
#define SP " "

#define HTTP_STATUS_OK            200
#define HTTP_STATUS_NOT_FOUND     404
#define HTTP_STATUS_BAD_REQUEST   400
#define HTTP_STATUS_SERVER_ERROR  500

const int PORT = 1337;
typedef struct{
    const char* data;
    size_t len;
} string;

typedef struct{
    string method;
    string uri;
    string version;
} http_req_line;

typedef enum http_status : uint16_t{
    HTTP_RES_OK = 200,
    HTTP_RES_BAD_REQ = 400,
    HTTP_RES_NOT_FOUND = 404,
    HTTP_RES_INT_SERVER_ERR = 500
} http_status;

typedef struct{
    const char* version; 
    http_status status;
} http_resp_line;

const char* http_status_to_string(http_status status){
    switch(status){
    case HTTP_RES_OK:
        return "OK";
    case HTTP_RES_BAD_REQ:
        return "Bad request";
    case HTTP_RES_INT_SERVER_ERR:
        return "Internal server error";
    default:
        return "Unknown";
    }
}

bool string_equal(string l, string r){
    if(l.len != r.len) return false;
    return memcmp(l.data, r.data, l.len) == 0;
}

string string_from_cstr(const char* str) {
    string s;
    s.len = strlen(str);
    s.data = str;
    return s;
}

static char* find_header_end(char* buff, size_t len){
    for(size_t i=0; i+3 < len; i++){
        if(buff[i] == '\r' && buff[i+1] == '\n' && buff[i+2] == '\r' && buff[i+3] == '\n'){
            return &buff[i];
        }
    }
    return NULL;
}

/* GET /index.html HTTP/1.1 
 parse this line
*/
static int parse_req_line(char* line, http_req_line* out){
    char* sp1 = strchr(line, ' ');
    if(!sp1) return -1;
    char* sp2 = strchr(sp1+1, ' ');
    if(!sp2) return -1;

    out->method.data = line;
    out->method.len = sp1-line;

    out->uri.data = sp1+1;
    out->uri.len = sp2-(sp1+1);

    out->version.data = sp2+1;
    out->version.len = strlen(sp2+1);
    return 0;
}

static const char resp_hello[] = "Hellooo";
static const char resp_bye[] = "Byeee";
static const char resp_404[] = "Not Found";

string http_resp_generate(char* buff, size_t buff_len, http_status status, size_t body_len){
    int n = 0;
    string response;
    response.len = 0;
    memset(buff, 0, buff_len);

    response.len += sprintf(buff, "%s %d %s" CRLF, "HTTP/1.0", status, http_status_to_string(status));
    response.len += sprintf(buff+response.len , "Content-Length: %zu" CRLF, body_len);
    response.len += sprintf(buff+response.len, CRLF);
    response.data = buff;
    return response;
}

bool http_send_resp(int socket, string header, string body){
    ssize_t n = send(socket, header.data, header.len, 0);
}

int handle_client(int client_socket){
    char buff[4096];
    size_t used = 0;

    for( ;; ){
        ssize_t n = read(client_socket,
            buff + used,
            sizeof(buff) - used
        );
        if(n < 0){
            perror("read");
            return -1;
        }
        if(n == 0) return 0;
        used += n;
        
        char* header_end = find_header_end(buff, used);
        if(!header_end){
            if(used == sizeof(buff)) return -1;
            continue;
        }

        char* line_end = strstr(buff, "\r\n");
        if(!line_end) return -1;

        *line_end = '\0';

        http_req_line req;
        if(parse_req_line(buff, &req) != 0) return -1;

        printf("METHOD:  %.*s\n", (int)req.method.len, req.method.data);
        printf("URI:     %.*s\n", (int)req.uri.len, req.uri.data);
        printf("VERSION: %.*s\n", (int)req.version.len, req.version.data);

        string route_hello = string_from_cstr("/hello");
        string route_bye = string_from_cstr("/bye");

        if(string_equal(req.uri, route_hello)){
            write(client_socket, resp_hello, strlen(resp_hello));
        }
        else if(string_equal(req.uri, route_bye)){
            write(client_socket, resp_bye, strlen(resp_bye));
        }
        else{
            write(client_socket, resp_404, strlen(resp_404));
        }
        close(client_socket);
        return 0;
    }
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

    bind_addr.sin_port = htons(PORT);
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
    printf("listening on http://localhost:%d/\n", PORT);
    for( ;; ){
        printf("waiting for connections...\n");
        client_socket = accept(tcp_socket, NULL, NULL);
        printf("got a connection\n");
        rc = handle_client(client_socket);
    }
exit:
    close(tcp_socket);
    return ret;
}
 