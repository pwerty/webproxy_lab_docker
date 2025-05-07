#include <stdio.h>
#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr =
    "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 "
    "Firefox/10.0.3\r\n";
// Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/135.0.0.0 Safari/537.36

void task(int fd);
void fowarding(int fd, rio_t *destServer);
void valueSelection(char *uri, char* link, char *cgiargs);
void read_requesthdrs(rio_t *rp);
void get_filetype(char *filename, char *filetype);
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);


int main(int argc, char **argv)
{
  int listenfd, connfd;
  char hostname[MAXLINE], port[MAXLINE];
  socklen_t clientlen;
  struct sockaddr_storage clientaddr;
  rio_t rio;

  if(argc != 2)
  {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }

  listenfd = Open_listenfd(argv[1]);

  while (1)
  {
    clientlen = sizeof(clientaddr);
    connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
    Getnameinfo((SA *)&clientaddr, clientlen, hostname, MAXLINE, port, MAXLINE, 0);
    task(connfd);
    Close(connfd);
  }
  
  exit(0);
}

void task(int fd)
{
  // fd를 위한 출력 값을 만들어야한다.
  // 즉 여기서 tiny에 값을 보내서, 다시 받아오고, 그걸 다시 뿌려주게 된다.
  struct stat sbuf;
  char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char newHeader[MAXLINE];
  int destServer_fd;
  rio_t fromClientrio, toServerrio;

  destServer_fd = Open_clientfd("localhost", "12321");
  Rio_readinitb(&toServerrio, destServer_fd);

  Rio_readinitb(&fromClientrio, fd);

  // 프록시랑 연결된 클라이언트로부터 요청 내용을 긁어온다.
  Rio_readlineb(&fromClientrio, buf, MAXLINE);
  printf("Request Headers?!\n");
  printf("%s", buf);
  sscanf(buf, "%s %s %s", method, uri, version);

  sprintf(newHeader, "%s %s HTTP/1.0\r\n", method, uri);
  sprintf(newHeader, "%sHost: localhost\r\n", newHeader);
  sprintf(newHeader, "%sServer: Tiny Web Server\r\n", newHeader);
  sprintf(newHeader, "%sUser-Agent: %s",newHeader, user_agent_hdr);
  sprintf(newHeader, "%sConnection: close\r\n", newHeader);
  sprintf(newHeader, "%sProxy-Connection: close\r\n\r\n", newHeader);
  Rio_writen(destServer_fd, newHeader, strlen(newHeader));

  // 여기서 클라이언트에 최종 전달한다.
  fowarding(fd, &toServerrio);
  Close(destServer_fd);
}

void fowarding(int fd, rio_t *destServer)
{
  char buf[MAXLINE];
  ssize_t n;

  while ((n = Rio_readlineb(destServer, buf, MAXLINE)) > 0)
  {
    Rio_writen(fd, buf, n);
    if(strcmp(buf, "\r\n") == 0)
      break;
  }

  while ((n = Rio_readnb(destServer, buf, MAXLINE)) > 0)
  {
    Rio_writen(fd, buf, n);
  }
}





// 1차 구현 완료