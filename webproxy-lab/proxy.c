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
  char buf[MAXLINE], _buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
  char newHeader[MAXLINE], recieved[MAXLINE];
  char filename[MAXLINE], cgiargs[MAXLINE];
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



void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
  char buf[MAXLINE], body[MAXBUF];

  /* Build the HTTP response body */
  sprintf(body, "<html><title>Tiny Error</title>");
  sprintf(body, "%s<body bgcolor=""ffffff"">\r\n", body);
  sprintf(body, "%s%s: %s\r\n", body, errnum, shortmsg);
  sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
  sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

  /* Print the HTTP response */
  sprintf(buf, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-type: text/html\r\n");
  Rio_writen(fd, buf, strlen(buf));
  sprintf(buf, "Content-length: %d\r\n\r\n", (int)strlen(body));
  Rio_writen(fd, buf, strlen(buf));
  Rio_writen(fd, body, strlen(body));
}

// 헤더값 필수값을 수동으로 삽입하라고?
// 이건 tiny에서 가져오면 되지 않나

// proxy 입장에서 요청해서 받아오는 걸 할 수 있어야함
// 아 그럼 내가 서버이자 클라가 되어야하는거구나

// 그럼 내가 서버의 입장으로써 받아온 요청값에 따라 그 값에 기반해서 클라이언트가 되어서 요청하는 사람이 되어야하는구나

/*

그렇다면 전체적 루틴이

내가 연결을 받는다
  연결 받은 곳에서 원하는 내용을 받아본다
    해당 내용을 인자로써 서버에 요청한다 : 물론 그 전에 가고자하는 서버랑 연결되어야겠지
    서버가 보내는걸 받는다
    그 내용을 그대로 클라에게 던진다

  이게 기초적인 흐름이군요
*/


// tiny 서버를 열어서 tiny와 proxy를 이어주고, 클라이언트 입장에서는 proxy:proxyport로 연결을 유도해야함
// 클라이언트의 요청을 tiny에게 요청하는 내용으로 echo-client 내용을 기반으로 구현하기
// 그리고 proxy의 serve_ 에서 tiny 내용에 있는 걸로 뱉어주는 형태로 완성