#include "cache.h"
#include "csapp.h"
#include "sbuf.h"
#include <stdio.h>
#include <string.h>
#include <strings.h>

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400
#define NTHREADS 8
#define SBUFSIZE 8

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";
sbuf_t sbuf; // shared buffer of connected fd

void handleHTTPreq(int fd);
void clienterror(int fd, char *cause, char *msg);
void parse_req(int clientfd, char *req_line, char *method, char *uri,
               char *host, char *port);
int parse_url(char *url, char *host, char *port, char *uri);
void forwardRequest(int clientfd, char *method, char *uri, char *host,
                    char *port);
void read_and_cache(int serverfd, int clientfd, char **resp, size_t *resp_size);
void *pthread_routine(void *vargp);

int main(int argc, char **argv) {
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;

  /* Check command line args */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  pthread_t tids[NTHREADS];
  sbuf_init(&sbuf, SBUFSIZE);
  // pre-threading
  for (int i = 0; i < NTHREADS; i++) {
    Pthread_create(&tids[i], NULL, pthread_routine, NULL);
  }

  // init cache
  cache_init();

  listenfd = Open_listenfd(argv[1]);
  while (1) {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen); // to client
    sbuf_insert(&sbuf, connfd);
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE,
                0);
    printf("Accepted connection from (%s, %s)\n", hostname, port);
  }
  return 0;
}

void *pthread_routine(void *vargp) {
  Pthread_detach(pthread_self());
  while (1) {
    int connfd = sbuf_remove(&sbuf);
    handleHTTPreq(connfd);
    Close(connfd);
  }
}

void handleHTTPreq(int fd) {
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], host[MAXLINE],
      port[MAXLINE];
  char req_line[MAXLINE];
  rio_t rio;

  /* Read request line and headers */
  Rio_readinitb(&rio, fd);
  if (!Rio_readlineb(&rio, buf, MAXLINE))
    return;
  printf("%s", buf);

  /* Save request line before reading remaining headers */
  strcpy(req_line, buf);

  /* Read remaining request headers */
  while (strcmp(buf, "\r\n")) {
    Rio_readlineb(&rio, buf, MAXLINE);
    printf("%s", buf);
  }

  parse_req(fd, req_line, method, uri, host, port);
  char key[3 * MAXLINE];
  sprintf(key, "%s:%s%s", host, port, uri);

  char *data;
  size_t size;

  int not_found = cache_lookup(key, &data, &size);
  if (not_found) {
    int clientfd = Open_clientfd(host, port);
    forwardRequest(clientfd, method, uri, host, port);

    char *resp;
    size_t resp_size;
    read_and_cache(clientfd, fd, &resp, &resp_size);

    if (resp) {
      cache_insert(key, resp, resp_size);
      free(resp);
    }
    Close(clientfd);
  } else {
    // ===== 命中 =====
    Rio_writen(fd, data, size);
    free(data);
  }

}

void clienterror(int fd, char *cause, char *msg) {
  char buf[MAXLINE];
  sprintf(buf, "%s %s\r\n", cause, msg);
  Rio_writen(fd, buf, strlen(buf));
}

void parse_req(int clientfd, char *req_line, char *method, char *uri,
               char *host, char *port) {
  char url[MAXLINE], version[MAXLINE];

  sscanf(req_line, "%s %s %s", method, url, version);

  if (strcasecmp(method, "GET")) {
    clienterror(clientfd, method, "Not implemented");
    return;
  }

  if (!parse_url(url, host, port, uri)) {
    clienterror(clientfd, method, "Can't parse URL");
    return;
  }
}

int parse_url(char *url, char *host, char *port, char *uri) {
  char *ptr, *slash, *colon;

  /* Skip "http://" */
  ptr = strstr(url, "://");
  if (ptr) {
    ptr += 3;
  } else {
    ptr = url;
  }

  slash = strchr(ptr, '/');
  colon = strchr(ptr, ':');

  if (colon && (!slash || colon < slash)) {
    /* Port is specified */
    size_t hostlen = colon - ptr;
    memcpy(host, ptr, hostlen);
    host[hostlen] = '\0';

    if (slash) {
      size_t portlen = slash - colon - 1;
      memcpy(port, colon + 1, portlen);
      port[portlen] = '\0';
    } else {
      strcpy(port, colon + 1);
    }
  } else {
    /* No port specified, use default 80 */
    strcpy(port, "80");
    if (slash) {
      size_t hostlen = slash - ptr;
      memcpy(host, ptr, hostlen);
      host[hostlen] = '\0';
    } else {
      strcpy(host, ptr);
    }
  }

  if (slash) {
    strcpy(uri, slash);
  } else {
    strcpy(uri, "/");
  }

  return 1;
}

void forwardRequest(int clientfd, char *method, char *uri, char *host,
                    char *port) {
  char buf[MAXLINE];

  /* Write request line */
  sprintf(buf, "%s %s HTTP/1.0\r\n", method, uri);
  Rio_writen(clientfd, buf, strlen(buf));

  /* Write Host header */
  sprintf(buf, "Host: %s:%s\r\n", host, port);
  Rio_writen(clientfd, buf, strlen(buf));

  Rio_writen(clientfd, (void *)user_agent_hdr, strlen(user_agent_hdr));

  sprintf(buf, "Connection: close\r\n");
  Rio_writen(clientfd, buf, strlen(buf));

  sprintf(buf, "Proxy-Connection: close\r\n\r\n");
  Rio_writen(clientfd, buf, strlen(buf));
}

void read_and_cache(int serverfd, int clientfd, char **resp,
                    size_t *resp_size) {
  char buf[MAXLINE];
  rio_t rio;
  ssize_t n;
  char *buf_data = NULL;
  size_t total = 0;
  size_t capacity = 0;

  Rio_readinitb(&rio, serverfd);

  while ((n = Rio_readnb(&rio, buf, MAXLINE)) > 0) {
    Rio_writen(clientfd, buf, n);

    if (total <= MAX_OBJECT_SIZE) {
      if (total + n > capacity) {
        capacity = (total + n) * 2;
        char *newbuf = realloc(buf_data, capacity);
        if (!newbuf) {
          // realloc 失败 → 放弃缓存
          free(buf_data);
          buf_data = NULL;
          total = MAX_OBJECT_SIZE + 1; // 标记超大，后续不再尝试
          continue;
        }
        buf_data = newbuf;
      }
      memcpy(buf_data + total, buf, n);
      total += n;
    }
  }

  if (buf_data && total <= MAX_OBJECT_SIZE) {
    *resp = buf_data;
    *resp_size = total;
  } else {
    free(buf_data);
    *resp = NULL;
    *resp_size = 0;
  }
}
