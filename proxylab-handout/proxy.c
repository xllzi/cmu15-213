#include <stdio.h>
#include <string.h>
#include <strings.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

void handleHTTPreq(int fd);
void clienterror(int fd, char* method, char* msg);
void read_requesthdrs(rio_t *rp);
void parse_req(int clientfd, char* buf, char* uri, char* host, char* port);
int parse_url(char* url, char* uri, char* port);
void forwardRequest(int clientfd, char* req_hdr, char* host, char* port);

int main(int argc, char** argv)
{
    int listenfd, connfd;
    char hostname[MAXLINE], port[MAXLINE];
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;

    /* Check command line args */
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    listenfd = Open_listenfd(argv[1]);
    while (1) {
        clientlen = sizeof(clientaddr);
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); //line:netp:tiny:accept
            Getnameinfo((SA *) &clientaddr, clientlen, hostname, MAXLINE, 
                        port, MAXLINE, 0);
            printf("Accepted connection from (%s, %s)\n", hostname, port);
        handleHTTPreq(connfd);
        Close(listenfd);
    }
    return 0;
}

void handleHTTPreq(int fd) {
    char buf[MAXLINE], uri[MAXLINE], host[MAXLINE], port[MAXLINE]; 
    port[0] = '8'; // default port number
    port[1] = '0';
    rio_t rio;

    /* Read request line and headers */
    Rio_readinitb(&rio, fd);
    if (!Rio_readlineb(&rio, buf, MAXLINE))  //line:netp:doit:readrequest
        return;
    while(strcmp(buf, "\r\n")) {          //line:netp:readhdrs:checkterm
        Rio_readlineb(&rio, buf, MAXLINE);
    }
    printf("%s", buf);

    parse_req(fd, buf, uri, host, port);
    int clientfd = Open_clientfd(host, port);
    forwardRequest(clientfd, buf, host, port);
}

void clienterror(int fd, char* method, char* msg) 
{
    char buf[MAXLINE];
    sprintf(buf, "%s %s\n", method, msg);
    Rio_writen(fd, buf, strlen(buf));
}

void parse_req(int clientfd, char* buf, char* uri, char* host, char* port) {
    char method[MAXLINE], url[MAXLINE], version[MAXLINE];
    char* bufp = buf;
    sscanf(buf, "%s %s %s", method, url, version);       //line:netp:doit:parserequest
    bufp = strstr(buf, "\r\n");
    // verify HTTP header
    if (strcasecmp(method, "GET")) {                     //line:netp:doit:beginrequesterr
        clienterror(clientfd, method, "Not implemented");
        return;
    }                                                    //line:netp:doit:endrequesterr
    if (!parse_url(url, uri, port)) {
        clienterror(clientfd, method, "url is not end with slash");
    }

    strcpy(version, "HTTP/1.0");
    strcpy(buf, strcat(strcat(method, uri), version));
    // parse fields
    char* end;
    bufp = strstr(bufp, "Host: ");
    end = strstr(bufp, "\r\n");
    size_t len = end - bufp;
    memcpy(host, bufp+1, len-1);

}
/* parse url, extract uri, return non-zero to indicate that
 * url is end with '/'
 */
int parse_url(char* url, char* uri, char* port) {
    char* ptr;
    ptr = index(url, '/');
    if (ptr) {
        strcpy(uri, ptr);
        char* ptr2 = index(url, ':');
        if (ptr2) {
            size_t len = ptr - ptr2;
            memcpy(port, ptr2, len);
        }
        return 0;
    } else {
        return 1;
    }

}

void forwardRequest(int clientfd, char* req_hdr, char* host, char* port) {
    char buf[MAXLINE];
    rio_t rio;
    char* cur = buf;
    char* nextl = NULL;
    while ((nextl = strstr(req_hdr, "\r\n")) != NULL) {
        size_t len = nextl - cur;
        Rio_writen(clientfd, buf, len);
        cur = nextl+2;
    }
    Rio_writen(clientfd, user_agent_hdr, strlen(user_agent_hdr));
    char* conn_hdr = "Connection: close";
    Rio_writen(clientfd, conn_hdr, strlen(conn_hdr));
    char* pconn_hdr = "Proxy-Connection: close";
    Rio_writen(clientfd, pconn_hdr, strlen(pconn_hdr));
}

void forwardResponse(int connfd) {
    char buf[MAXLINE];
    rio_t rio;
    while (!Rio_readlineb(&rio, buf, MAXLINE));
    Rio_writen(connfd, buf, strlen(buf));
}
